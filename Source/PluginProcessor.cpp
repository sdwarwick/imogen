#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      #endif
                       ),
		voxCurrentPitch(0.0f),
		tree(*this, nullptr, "PARAMETERS", createParameters()),
		midiProcessor(harmEngine),
		midiLatch(false),
		lastSampleRate(44100), lastBlockSize(512), prevLastSampleRate(44100), prevLastBlockSize(512),
		frameIsPitched(false),
		adsrIsOn(true),
		prevAttack(0.0f), prevDecay(0.0f), prevSustain(0.0f), prevRelease(0.0f),
		previousStereoWidth(100.0f),
		prevVelocitySens(100.0f),
		prevPitchBendUp(2.0f), prevPitchBendDown(2.0f),
		latchIsOn(false), previousLatch(false),
		stealingIsOn(true),
		analysisShift(100), analysisShiftHalved(50), analysisLimit(461),
		previousmidipan(64),
		previousMasterDryWet(100),
		dryMultiplier(0.0f), wetMultiplier(1.0f),
		prevideb(0.0f), prevodeb(0.0f),
		dryBufferWritePosition(0), dryBufferReadPosition(0)

#endif
{
	
	// initializes each instance of the HarmonyVoice class inside the harmEngine array:
	for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
		harmEngine.add(new HarmonyVoice(i));
	}
	
	wetBuffer.setSize(NUMBER_OF_CHANNELS, MAX_BUFFERSIZE);
	wetBuffer.clear();
	dryBuffer.setSize(NUMBER_OF_CHANNELS, MAX_BUFFERSIZE);
	dryBuffer.clear();
	
	dryvoxpanningmults[0] = 64;
	dryvoxpanningmults[1] = 64;
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
	
	Timer::startTimer(TIMER_RATE_MS);
}

ImogenAudioProcessor::~ImogenAudioProcessor() {
	for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
		delete harmEngine[i];
	}
	
	Timer::stopTimer();
}


AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
	std::vector<std::unique_ptr<RangedAudioParameter>> params;
	
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrAttack", "ADSR Attack", NormalisableRange<float> (0.01f, 1.0f), 0.035f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrDecay", "ADSR Decay", NormalisableRange<float> (0.01f, 1.0f), 0.06f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrSustain", "ADSR Sustain", NormalisableRange<float> (0.01f, 1.0f), 0.8f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrRelease", "ADSR Release", NormalisableRange<float> (0.01f, 1.0f), 0.1f));
	params.push_back(std::make_unique<AudioParameterBool>("adsrOnOff", "ADSR on/off", true));
	params.push_back(std::make_unique<AudioParameterFloat> ("stereoWidth", "Stereo Width", NormalisableRange<float> (0.0, 100.0), 100));
	params.push_back(std::make_unique<AudioParameterInt>("dryPan", "Dry vox pan", 0, 127, 64));
	params.push_back(std::make_unique<AudioParameterInt>("masterDryWet", "% wet", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", NormalisableRange<float> (0.0, 100.0), 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("PitchBendUpRange", "Pitch bend range (up)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("PitchBendDownRange", "Pitch bend range (down)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f), 0.0f));
	params.push_back(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f), -4.0f));
	params.push_back(std::make_unique<AudioParameterBool>("midiLatch", "MIDI Latch on/off", false));
	params.push_back(std::make_unique<AudioParameterBool>("voiceStealing", "Voice stealing", false));
	params.push_back(std::make_unique<AudioParameterInt>("inputChan", "Input channel", 0, 99, 0));
	params.push_back(std::make_unique<AudioParameterFloat>("limiterThresh", "Limiter threshold (dBFS)", NormalisableRange<float>(-60.0f, 0.0f), -2.0f));
	params.push_back(std::make_unique<AudioParameterInt>("limiterRelease", "limiter release (ms)", 1, 250, 10));
	
	return { params.begin(), params.end() };
}


//==============================================================================
const juce::String ImogenAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ImogenAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ImogenAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ImogenAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ImogenAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int ImogenAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ImogenAudioProcessor::getCurrentProgram() {
    return 0;
}

void ImogenAudioProcessor::setCurrentProgram (int index) {
}

const juce::String ImogenAudioProcessor::getProgramName (int index) {
    return {};
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName) {
}

//==============================================================================
void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock) {
	
	lastSampleRate = sampleRate;
	
	if(samplesPerBlock >= MAX_BUFFERSIZE) {
		lastBlockSize = MAX_BUFFERSIZE;
	} else {
		lastBlockSize = samplesPerBlock;
	}
	
	// DSP settings
	{
		if (prevLastSampleRate != lastSampleRate || prevLastBlockSize != lastBlockSize)
		{
			for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
				harmEngine[i]->updateDSPsettings(lastSampleRate, lastBlockSize);
			}
			
			pitchTracker.checkBufferSize(lastBlockSize);
			if(wetBuffer.getNumSamples() != lastBlockSize || wetBuffer.hasBeenCleared()) {
				wetBuffer.setSize(NUMBER_OF_CHANNELS, lastBlockSize, true, false, true);
			}
			
			const int numSeconds = 1;
			const int drybuffersize = sampleRate * samplesPerBlock * numSeconds; // size of circular dryBuffer. won't update dynamically within processBlock, size is set only here!
			if(dryBuffer.getNumSamples() != drybuffersize || dryBuffer.hasBeenCleared()) {
				dryBuffer.setSize(NUMBER_OF_CHANNELS, drybuffersize, true, true, true);
			}
			
		}
		prevLastSampleRate = lastSampleRate;
		prevLastBlockSize = lastBlockSize;
	}
	
	// ADSR settings
	{
		adsrIsOn = *adsrOnOffListener > 0.5f;
		
		if (prevAttack != *adsrAttackListener || prevDecay != *adsrDecayListener || prevSustain != *adsrSustainListener || prevRelease != *adsrReleaseListener)
		{
			for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
				harmEngine[i]->adsrSettingsListener(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener);
			}
			prevAttack = *adsrAttackListener;
			prevDecay = *adsrDecayListener;
			prevSustain = *adsrSustainListener;
			prevRelease = *adsrReleaseListener;
			prevVelocitySens = *midiVelocitySensListener;
	}
	}
	
	// MIDI velocity sensitivity
	{
		if(prevVelocitySens != *midiVelocitySensListener) {
			for(int i = 0; i < NUMBER_OF_VOICES; ++i)
			{
				harmEngine[i]->midiVelocitySensitivityListener(midiVelocitySensListener);
			}
		}
		prevVelocitySens = *midiVelocitySensListener;
	}
	
	// stereo width
	{
		if (previousStereoWidth != *stereoWidthListener) {
			midiProcessor.updateStereoWidth(stereoWidthListener);
			previousStereoWidth = *stereoWidthListener;
		}
	}
	
	// dry vox pan
	{
		if(*dryVoxPanListener != previousmidipan) {
			const float panR = (*dryVoxPanListener)/127.0f;
			const float panL = 1.0f - panR;
			dryvoxpanningmults[0] = panL;
			dryvoxpanningmults[1] = panR;
			
			previousmidipan = *dryVoxPanListener;
		}
	}
	
	// pitch bend settings
	{
		if (prevPitchBendUp != *pitchBendUpListener || prevPitchBendDown != * pitchBendDownListener) {
			for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
				harmEngine[i]->pitchBendSettingsListener(pitchBendUpListener, pitchBendDownListener);
			}
			prevPitchBendUp = *pitchBendUpListener;
			prevPitchBendDown = *pitchBendDownListener;
		}
	}
	
	// master dry/wet
	{
		if(*masterDryWetListener != previousMasterDryWet) {
			wetMultiplier = (*masterDryWetListener)/100.0f;
			dryMultiplier = 1.0f - wetMultiplier;
			previousMasterDryWet = *masterDryWetListener;
		}
	}
	
	// input & output gain
	{
		if(*inputGainListener != prevideb || *outputGainListener != prevodeb) {
			inputGainMultiplier = Decibels::decibelsToGain(*inputGainListener);
			outputGainMultiplier = Decibels::decibelsToGain(*outputGainListener);
		}
		prevideb = *inputGainListener;
		prevodeb = *outputGainListener;
	}
	
	// MIDI latch
	{
		latchIsOn = *midiLatchListener > 0.5f;
		if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(); }
		previousLatch = latchIsOn;
	}
	
	stealingIsOn = *voiceStealingListener > 0.5f; // voice stealing on/off
	
	dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = MAX_BUFFERSIZE;
	spec.numChannels = 2;
	limiter.prepare(spec);
	limiter.setThreshold(*limiterThreshListener);
	limiter.setRelease(*limiterReleaseListener);
	
	if(Timer::isTimerRunning() == false)
	{
		Timer::startTimer(TIMER_RATE_MS);
	}
}

void ImogenAudioProcessor::releaseResources() {
	
	wetBuffer.clear();
	dryBuffer.clear();
	for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
		harmEngine[i]->clearBuffers();
	}
	pitchTracker.clearBuffer();
	limiter.reset(); // ??
	Timer::stopTimer();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


/*===========================================================================================================================
============================================================================================================================*/


void ImogenAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	
	midiProcessor.processIncomingMidi(midiMessages, latchIsOn, stealingIsOn);
	
	int inpt = *inputChannelListener;
	if (inpt > buffer.getNumChannels()) {
		inpt = buffer.getNumChannels();
	}
	const int inputChannel = inpt - 1; // if the user enters "channel 1", that's actually channel 0 in an audioBuffer
	
	int samplesLeft = buffer.getNumSamples();
	// int numElapsedLoops = 0;
	while (samplesLeft > 0)
	{
		// const int numSamples = std::max(samplesLeft, MAX_BUFFERSIZE);
		
		// slice input buffer into frames of length MAX_BUFFERSIZE or smaller
		int samps;
		if(samplesLeft >= MAX_BUFFERSIZE) {
			samps = MAX_BUFFERSIZE;
		}
		else {
			samps = samplesLeft;
		}
		const int numSamples = samps; // number of samples in this frame / slice
		
		AudioBuffer<float> proxy (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples() - samplesLeft, numSamples);
		processBlockPrivate(proxy, numSamples, inputChannel);
		samplesLeft -= numSamples;
		// ++numElapsedLoops
	}
	// numElapsedLoops represents the number of times processBlockPrivate() was run, and can be used for latency calculations
}


void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& buffer, const int numSamples, const int inputChannel)
{
	if(wetBuffer.getNumSamples() != numSamples) {
		wetBuffer.setSize(NUMBER_OF_CHANNELS, numSamples, true, false, true);
	}
	
	// update settings/parameters
	{
		// ADSR settings
		{
			adsrIsOn = *adsrOnOffListener > 0.5f;
			
			if (prevAttack != *adsrAttackListener || prevDecay != *adsrDecayListener || prevSustain != *adsrSustainListener || prevRelease != *adsrReleaseListener)
			{
				for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
					harmEngine[i]->adsrSettingsListener(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener);
				}
			}
		}
		
		// MIDI velocity sensitivity
		{
			if(prevVelocitySens != *midiVelocitySensListener) {
				for(int i = 0; i < NUMBER_OF_VOICES; ++i)
				{
					harmEngine[i]->midiVelocitySensitivityListener(midiVelocitySensListener);
				}
			}
		}
		
		// dry vox pan
		{
			if(*dryVoxPanListener != previousmidipan) {
				const float panR = (*dryVoxPanListener)/127.0f;
				const float panL = 1.0f - panR;
				dryvoxpanningmults[0] = panL;
				dryvoxpanningmults[1] = panR;
			}
		}
		
		// pitch bend settings
		{
			if (prevPitchBendUp != *pitchBendUpListener || prevPitchBendDown != * pitchBendDownListener) {
				for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
					harmEngine[i]->pitchBendSettingsListener(pitchBendUpListener, pitchBendDownListener);
				}
			}
		}
		
		// master dry/wet
		{
			if(*masterDryWetListener != previousMasterDryWet) {
				wetMultiplier = (*masterDryWetListener)/100.0f;
				dryMultiplier = 1.0f - wetMultiplier;
			}
		}
		
		// input & output gain
		{
			if(*inputGainListener != prevideb || *outputGainListener != prevodeb) {
				inputGainMultiplier = Decibels::decibelsToGain(*inputGainListener);
				outputGainMultiplier = Decibels::decibelsToGain(*outputGainListener);
			}
		}
		
		// MIDI latch
		{
			latchIsOn = *midiLatchListener > 0.5f;
			if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(); }
		}
		
		// voice stealing
		stealingIsOn = *voiceStealingListener > 0.5f;
		
		// limiter settings
		limiter.setThreshold(*limiterThreshListener);
		limiter.setRelease(*limiterReleaseListener);
	}
	
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	buffer.applyGain(inputChannel, 0, numSamples, inputGainMultiplier); // apply input gain
	
	writeToDryBuffer(buffer, inputChannel, numSamples); // write this frame's input to the stereo, circular dryBuffer
	
	analyzeInput(buffer, inputChannel, numSamples); // extract epoch indices, etc
	
	for (int i = 0; i < NUMBER_OF_VOICES; ++i) {  // i = the harmony voice # currently being processed
		if (harmEngine[i]->voiceIsOn) {  // only do audio processing on active voices:
			
			// writes this HarmonyVoice's shifted samples to its harmonyBuffer
			harmEngine[i]->renderNextBlock(buffer, numSamples, inputChannel, voxCurrentPitch, epochLocations, 3, adsrIsOn); // how to calculate numOfEpochsPerFrame parameter?
			
			// writes shifted sample values to wetBuffer
			for(int channel = 0; channel < NUMBER_OF_CHANNELS; ++channel)
			{
				wetBuffer.addFrom(channel, 0, harmEngine[i]->harmonyBuffer, channel, 0, numSamples);
			}
		}
	}
	
	// clear any extra channels present in I/O buffer
	{
		if (buffer.getNumChannels() > NUMBER_OF_CHANNELS) {
			for (int i = NUMBER_OF_CHANNELS + 1; i <= buffer.getNumChannels(); ++i) {
				buffer.clear(i - 1, 0, numSamples);
			}
		}
	}

	// write from wetBuffer to I/O buffer
	wetBuffer.applyGain(0, numSamples, wetMultiplier);
	for(int channel = 0; channel < NUMBER_OF_CHANNELS; ++channel)
	{
		buffer.copyFrom(channel, 0, wetBuffer, channel, 0, numSamples);
	}
	
	// write from dryBuffer to I/O buffer, accounting for latency of harmony algorithm
	{
		const int harmonyLatency = 0; // latency in samples of the harmony algorithm, used to realign the dry signal w the wet
		
		dryBufferReadPosition = dryBufferWritePosition - numSamples - harmonyLatency;
		
		if (dryBufferReadPosition < 0) {
			dryBufferReadPosition += dryBuffer.getNumSamples();
		}
		
		if(dryBufferReadPosition + numSamples <= dryBuffer.getNumSamples())
		{
			dryBuffer.applyGain(dryBufferReadPosition, numSamples, dryMultiplier);
			for(int channel = 0; channel < NUMBER_OF_CHANNELS; ++channel)
			{
				buffer.addFrom(channel, 0, dryBuffer, channel, dryBufferReadPosition, numSamples);
			}
		}
		else
		{
			const int midPos = dryBuffer.getNumSamples() - dryBufferReadPosition;
			for(int channel = 0; channel < NUMBER_OF_CHANNELS; ++channel)
			{
				dryBuffer.applyGain(channel, dryBufferReadPosition, midPos, dryMultiplier);
				buffer.addFrom(channel, 0, dryBuffer, channel, dryBufferReadPosition, midPos);
				dryBuffer.applyGain(channel, 0, numSamples - midPos, dryMultiplier);
				buffer.addFrom(channel, midPos, dryBuffer, channel, 0, numSamples - midPos);
			}
		}
		dryBufferReadPosition += numSamples; // these two lines may be redundant, since dryBufferReadPosition is calculated each frame based on dryBufferWritePosition & the latency offset...
		dryBufferReadPosition %= dryBuffer.getNumSamples();
	}
	
	buffer.applyGain(0, numSamples, outputGainMultiplier); // apply master output gain
	
	// output limiter
	dsp::AudioBlock<float> limiterBlock (buffer);
	limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
	
	// update storage of previous frame's parameters, for comparison when the next frame comes in...
	{
		prevAttack = *adsrAttackListener;
		prevDecay = *adsrDecayListener;
		prevSustain = *adsrSustainListener;
		prevRelease = *adsrReleaseListener;
		prevVelocitySens = *midiVelocitySensListener;
		prevVelocitySens = *midiVelocitySensListener;
		previousStereoWidth = *stereoWidthListener;
		previousmidipan = *dryVoxPanListener;
		prevPitchBendUp = *pitchBendUpListener;
		prevPitchBendDown = *pitchBendDownListener;
		previousMasterDryWet = *masterDryWetListener;
		prevideb = *inputGainListener;
		prevodeb = *outputGainListener;
		previousLatch = latchIsOn;
	}
};


/*===========================================================================================================================
 ============================================================================================================================*/


void ImogenAudioProcessor::killAllMidi() {
	midiProcessor.killAll();
};


bool ImogenAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor() {
    return new ImogenAudioProcessorEditor (*this);
}

//==============================================================================
void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}





/////// ANALYZE INPUT

void ImogenAudioProcessor::analyzeInput (AudioBuffer<float>& input, const int inputChan, const int numSamples)
{
	const float newPitch = pitchTracker.pitchDetectionResult(input, inputChan, numSamples, lastSampleRate);
	frameIsPitched = pitchTracker.isitPitched();
	if (newPitch > 0.0f)
	{
		voxCurrentPitch = newPitch;
	} else {
		voxCurrentPitch = 80.0f; // need to set to arbitrary value ??
		frameIsPitched = false;
	}
	
	epochLocations = epochs.extractEpochIndices(input, inputChan, numSamples, lastSampleRate);
	
};



//// write dry input to dryBuffer so it can be mixed w wet signal for output
void ImogenAudioProcessor::writeToDryBuffer (AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples) {
	// move samples from input buffer (stereo channel) to dryBuffer (stereo buffer, panned according to midiPan)
	// NEED TO ACCOUNT FOR LATENCY OF THE HARMONY ALGORITHM !!!

	for(int channel = 0; channel < NUMBER_OF_CHANNELS; ++channel)
	{
		if(dryBufferWritePosition + numSamples <= dryBuffer.getNumSamples())
		{
			dryBuffer.copyFrom(channel, dryBufferWritePosition, inputBuffer, inputChan, 0, numSamples);
			dryBuffer.applyGain(channel, dryBufferWritePosition, numSamples, dryvoxpanningmults[channel]);
		}
		else
		{
			const int midPos = dryBuffer.getNumSamples() - dryBufferWritePosition;
			dryBuffer.copyFrom(channel, dryBufferWritePosition, inputBuffer, inputChan, 0, midPos);
			dryBuffer.applyGain(channel, dryBufferWritePosition, midPos, dryvoxpanningmults[channel]);
			dryBuffer.copyFrom(channel, 0, inputBuffer, inputChan, midPos, numSamples - midPos);
			dryBuffer.applyGain(channel, 0, numSamples - midPos, dryvoxpanningmults[channel]);
		}
	}
	dryBufferWritePosition += numSamples;
	dryBufferWritePosition %= dryBuffer.getNumSamples();
	
};



void ImogenAudioProcessor::timerCallback() {
	
	// update stereo width
	{
		if (previousStereoWidth != *stereoWidthListener) {
			midiProcessor.updateStereoWidth(stereoWidthListener);
		}
	}
	
};



Array<int> ImogenAudioProcessor::returnActivePitches() {
	return midiProcessor.getActivePitches();
};


