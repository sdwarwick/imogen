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
		midiLatch(false),
		lastSampleRate(44100), lastBlockSize(512), prevLastSampleRate(44100), prevLastBlockSize(512),
		frameIsPitched(false),
		adsrIsOn(true),
		prevAttack(0.0f), prevDecay(0.0f), prevSustain(0.0f), prevRelease(0.0f),
		previousStereoWidth(100.0f),
		lowestPannedNote(0),
		prevVelocitySens(100.0f),
		prevPitchBendUp(2.0f), prevPitchBendDown(2.0f),
		pedalPitchToggle(false),
		pedalPitchThresh(127),
		latchIsOn(false), previousLatch(false),
		stealingIsOn(true),
		analysisShift(100), analysisShiftHalved(50), analysisLimit(461),
		previousmidipan(64),
		previousMasterDryWet(100),
		dryMultiplier(0.0f), wetMultiplier(1.0f),
		prevideb(0.0f), prevodeb(0.0f),
		limiterIsOn(true),
		wetBuffer(NUMBER_OF_CHANNELS, MAX_BUFFERSIZE),
		dryBuffer(NUMBER_OF_CHANNELS, MAX_BUFFERSIZE),
		dryBufferWritePosition(0), dryBufferReadPosition(0),
		adsrAttackListener(*tree.getRawParameterValue("adsrAttack")),
		adsrDecayListener(*tree.getRawParameterValue("adsrDecay")),
		adsrSustainListener(*tree.getRawParameterValue("adsrSustain")),
		adsrReleaseListener(*tree.getRawParameterValue("adsrRelease")),
		adsrOnOffListener(*tree.getRawParameterValue("adsrOnOff")),
		stereoWidthListener(*tree.getRawParameterValue("stereoWidth")),
		lowestPanListener(*tree.getRawParameterValue("lowestPan")),
		midiVelocitySensListener(*tree.getRawParameterValue("midiVelocitySensitivity")),
		pitchBendUpListener(*tree.getRawParameterValue("PitchBendUpRange")),
		pitchBendDownListener(*tree.getRawParameterValue("PitchBendDownRange")),
		pedalPitchToggleListener(*tree.getRawParameterValue("pedalPitchToggle")),
		pedalPitchThreshListener(*tree.getRawParameterValue("pedalPitchThresh")),
		inputGainListener(*tree.getRawParameterValue("inputGain")),
		outputGainListener(*tree.getRawParameterValue("outputGain")),
		midiLatchListener(*tree.getRawParameterValue("midiLatch")),
		voiceStealingListener(*tree.getRawParameterValue("voiceStealing")),
		inputChannelListener(*tree.getRawParameterValue("inputChan")),
		dryVoxPanListener(*tree.getRawParameterValue("dryPan")),
		masterDryWetListener(*tree.getRawParameterValue("masterDryWet")),
		limiterThreshListener(*tree.getRawParameterValue("limiterThresh")),
		limiterReleaseListener(*tree.getRawParameterValue("limiterRelease")),
		limiterToggleListener(*tree.getRawParameterValue("limiterIsOn"))

#endif
{
	
	for (int i = 0; i < NUMBER_OF_VOICES; ++i) { harmonizer.addVoice(new HarmonizerVoice); }
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
	
	epochLocations.ensureStorageAllocated(MAX_BUFFERSIZE);
	epochLocations.clearQuick();
	epochLocations.fill(0);
};

ImogenAudioProcessor::~ImogenAudioProcessor()
{
};


AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
	std::vector<std::unique_ptr<RangedAudioParameter>> params;
	
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrAttack", "ADSR Attack", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.035f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrDecay", "ADSR Decay", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.06f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrSustain", "ADSR Sustain", NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.8f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrRelease", "ADSR Release", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.1f));
	params.push_back(std::make_unique<AudioParameterBool>("adsrOnOff", "ADSR on/off", true));
	params.push_back(std::make_unique<AudioParameterInt> ("stereoWidth", "Stereo Width", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterInt>("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
	params.push_back(std::make_unique<AudioParameterInt>("dryPan", "Dry vox pan", 0, 127, 64));
	params.push_back(std::make_unique<AudioParameterInt>("masterDryWet", "% wet", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterInt> ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("PitchBendUpRange", "Pitch bend range (up)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("PitchBendDownRange", "Pitch bend range (down)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterBool>("pedalPitchToggle", "Pedal pitch on/off", false));
	params.push_back(std::make_unique<AudioParameterInt>("pedalPitchThresh", "Pedal pitch threshold", 0, 127, 127));
	params.push_back(std::make_unique<AudioParameterFloat>("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), 0.0f));
	params.push_back(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -4.0f));
	params.push_back(std::make_unique<AudioParameterBool>("midiLatch", "MIDI Latch on/off", false));
	params.push_back(std::make_unique<AudioParameterBool>("voiceStealing", "Voice stealing", false));
	params.push_back(std::make_unique<AudioParameterInt>("inputChan", "Input channel", 0, 16, 0));
	params.push_back(std::make_unique<AudioParameterFloat>("limiterThresh", "Limiter threshold (dBFS)", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -2.0f));
	params.push_back(std::make_unique<AudioParameterInt>("limiterRelease", "limiter release (ms)", 1, 250, 10));
	params.push_back(std::make_unique<AudioParameterBool>("limiterIsOn", "Limiter on/off", true));
	
	return { params.begin(), params.end() };
};


//==============================================================================
const juce::String ImogenAudioProcessor::getName() const {
    return JucePlugin_Name;
};

bool ImogenAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
};

bool ImogenAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
};

bool ImogenAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
};

double ImogenAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
};

int ImogenAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
};

int ImogenAudioProcessor::getCurrentProgram() {
    return 0;
};

void ImogenAudioProcessor::setCurrentProgram (int index) {
};

const juce::String ImogenAudioProcessor::getProgramName (int index) {
    return {};
};

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName) {
};

//==============================================================================
void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock) {
	
	lastSampleRate = sampleRate;
	
	if(samplesPerBlock >= MAX_BUFFERSIZE) {
		lastBlockSize = MAX_BUFFERSIZE;
	} else {
		lastBlockSize = samplesPerBlock;
	}
	
	dsp::ProcessSpec oscSpec;
	oscSpec.sampleRate = sampleRate;
	oscSpec.maximumBlockSize = MAX_BUFFERSIZE;
	oscSpec.numChannels = 1;
	for(int i = 0; i < NUMBER_OF_VOICES; ++i)
	{
	//	harmEngine[i]->prepareOsc(oscSpec);
	}
	
	// DSP settings
	{
		wetBuffer.clear();
		if (prevLastSampleRate != lastSampleRate || prevLastBlockSize != lastBlockSize)
		{
			harmonizer.setCurrentPlaybackSampleRate(lastSampleRate);
			pitchTracker.checkBufferSize(lastBlockSize);
			if(wetBuffer.getNumSamples() != lastBlockSize) {
				wetBuffer.setSize(NUMBER_OF_CHANNELS, lastBlockSize, true, true, true);
			}
			
			//const int drybuffersize = sampleRate * lastBlockSize; // size of circular dryBuffer. won't update dynamically within processBlock, size is set only here!
			if(dryBuffer.getNumSamples() != lastBlockSize) {
				dryBuffer.setSize(NUMBER_OF_CHANNELS, lastBlockSize, true, true, true);
			}
			prevLastSampleRate = lastSampleRate;
			prevLastBlockSize = lastBlockSize;
		}
	}
	
	// ADSR settings
	{
		adsrIsOn = adsrOnOffListener > 0.5f;
		
		if (prevAttack != adsrAttackListener || prevDecay != adsrDecayListener || prevSustain != adsrSustainListener || prevRelease != adsrReleaseListener)
		{
			harmonizer.updateADSRsettings(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener);
			prevAttack = adsrAttackListener;
			prevDecay = adsrDecayListener;
			prevSustain = adsrSustainListener;
			prevRelease = adsrReleaseListener;
		}
	}
	
	// MIDI velocity sensitivity
	{
		if(prevVelocitySens != midiVelocitySensListener) {
			harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
			prevVelocitySens = midiVelocitySensListener;
		}
	}
	
	// stereo width
	{
		if (previousStereoWidth != stereoWidthListener) {
		//	midiProcessor.updateStereoWidth(stereoWidthListener);
			previousStereoWidth = stereoWidthListener;
		}
		lowestPannedNote = round(lowestPanListener);
	}
	
	// dry vox pan
	{
		if(dryVoxPanListener != previousmidipan) {
			dryvoxpanningmults[1] = dryVoxPanListener / 127.0f;
			dryvoxpanningmults[0] = 1.0f - dryvoxpanningmults[1];
			previousmidipan = dryVoxPanListener;
		}
	}
	
	// pitch bend settings
	{
		if (prevPitchBendUp != pitchBendUpListener || prevPitchBendDown != pitchBendDownListener) {
			harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
			prevPitchBendUp = pitchBendUpListener;
			prevPitchBendDown = pitchBendDownListener;
		}
	}
	
	// MIDI pedal pitch
	{
		pedalPitchToggle = pedalPitchToggleListener > 0.5f;
		pedalPitchThresh = round(pedalPitchThreshListener);
	}
	
	// master dry/wet
	{
		if(masterDryWetListener != previousMasterDryWet) {
			wetMultiplier = masterDryWetListener / 100.0f;
			dryMultiplier = 1.0f - wetMultiplier;
			previousMasterDryWet = masterDryWetListener;
		}
	}
	
	// input & output gain
	{
		if(inputGainListener != prevideb) {
			const float igv = inputGainListener;
			inputGainMultiplier = Decibels::decibelsToGain(igv);
			prevideb = inputGainListener;
		}
		if(outputGainListener != prevodeb) {
			const float ogv = outputGainListener;
			outputGainMultiplier = Decibels::decibelsToGain(ogv);
			prevodeb = outputGainListener;
		}
	}
	
	// MIDI latch
	{
		latchIsOn = midiLatchListener > 0.5f;
	//	if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(); }
		previousLatch = latchIsOn;
	}
	
	stealingIsOn = voiceStealingListener > 0.5f; // voice stealing on/off
	harmonizer.setNoteStealingEnabled(stealingIsOn);
	
	// limiter setup
	{
		dsp::ProcessSpec spec;
		spec.sampleRate = sampleRate;
		spec.maximumBlockSize = MAX_BUFFERSIZE;
		spec.numChannels = 2;
		limiter.prepare(spec);
		limiter.setThreshold(limiterThreshListener);
		limiter.setRelease(limiterReleaseListener);
		limiterIsOn = limiterToggleListener > 0.5f;
	}
	
};

void ImogenAudioProcessor::releaseResources() {
	
	wetBuffer.clear();
	dryBuffer.clear();
	
	pitchTracker.clearBuffer();
};

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
	// MIDI
	{
	//	if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(); }
	//	if (previousStereoWidth != stereoWidthListener) { midiProcessor.updateStereoWidth(stereoWidthListener); }
		lowestPannedNote = round(lowestPanListener);
		pedalPitchToggle = pedalPitchToggleListener > 0.5f;
		pedalPitchThresh = round(pedalPitchThreshListener);
		latchIsOn = midiLatchListener > 0.5f;
	}
	
	int inptchn = inputChannelListener;
	if (inputChannelListener >= buffer.getNumChannels()) { inptchn = buffer.getNumChannels() - 1; }
	const int inputChannel = inptchn;
	
	int samplesLeft = buffer.getNumSamples();
	// int numElapsedLoops = 0;
	while (samplesLeft > 0)
	{
		//const int numSamples = std::max(samplesLeft, MAX_BUFFERSIZE);
		
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
		processBlockPrivate(proxy, numSamples, inputChannel, midiMessages);
		samplesLeft -= numSamples;
		// ++numElapsedLoops
	}
	// numElapsedLoops represents the number of times processBlockPrivate() was run, and can be used for latency calculations
};


void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& buffer, const int numSamples, const int inputChannel, MidiBuffer& inputMidi)
{

	if(wetBuffer.getNumSamples() != numSamples) {
		wetBuffer.setSize(NUMBER_OF_CHANNELS, numSamples, true, false, true);
	}
	wetBuffer.clear();
	
	// update settings/parameters
	{
		// ADSR settings
		{
			adsrIsOn = adsrOnOffListener > 0.5f;
			if (adsrIsOn)
				harmonizer.updateADSRsettings(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener);
		}
		
		// MIDI velocity sensitivity
			if(prevVelocitySens != midiVelocitySensListener)
				harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
		
		// dry vox pan
		{
			if(dryVoxPanListener != previousmidipan)
			{
				dryvoxpanningmults[1] = dryVoxPanListener / 127.0f;
				dryvoxpanningmults[0] = 1.0f - dryvoxpanningmults[1];
			}
		}
		
		// pitch bend settings
		{
			if (prevPitchBendUp != pitchBendUpListener || prevPitchBendDown != pitchBendDownListener) {
				harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
			}
		}
		
		// master dry/wet
		{
			if(masterDryWetListener != previousMasterDryWet) {
				wetMultiplier = (masterDryWetListener)/100.0f;
				dryMultiplier = 1.0f - wetMultiplier;
			}
		}
		
		// input & output gain
		{
			if(inputGainListener != prevideb) {
				const float igv = inputGainListener;
				inputGainMultiplier = Decibels::decibelsToGain(igv);
			}
			if(outputGainListener != prevodeb) {
				const float ogv = outputGainListener;
				outputGainMultiplier = Decibels::decibelsToGain(ogv);
			}
		}
		
		// limiter settings
		{
			limiter.setThreshold(limiterThreshListener);
			limiter.setRelease(limiterReleaseListener);
			limiterIsOn = limiterToggleListener > 0.5f;
		}
	}
	
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	//buffer.applyGain(inputChannel, 0, numSamples, inputGainMultiplier); // apply input gain
	
	//analyzeInput(buffer, inputChannel, numSamples); // extract epoch indices, etc
	
	stealingIsOn = voiceStealingListener > 0.5f;
	harmonizer.setNoteStealingEnabled(stealingIsOn);
	
	harmonizer.renderNextBlock(buffer, inputMidi, 0, numSamples);
	
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
	
	
	buffer.applyGain(0, numSamples, outputGainMultiplier); // apply master output gain
	
	// output limiter
	if(limiterIsOn) {
		dsp::AudioBlock<float> limiterBlock (buffer);
		limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
	}
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
	
	// update storage of previous frame's parameters, for comparison when the next frame comes in...
	{
		prevAttack = adsrAttackListener;
		prevDecay = adsrDecayListener;
		prevSustain = adsrSustainListener;
		prevRelease = adsrReleaseListener;
		prevVelocitySens = midiVelocitySensListener;
		prevVelocitySens = midiVelocitySensListener;
		previousStereoWidth = stereoWidthListener;
		previousmidipan = dryVoxPanListener;
		prevPitchBendUp = pitchBendUpListener;
		prevPitchBendDown = pitchBendDownListener;
		previousMasterDryWet = masterDryWetListener;
		prevideb = inputGainListener;
		prevodeb = outputGainListener;
		previousLatch = latchIsOn;
	}
};


/*===========================================================================================================================
 ============================================================================================================================*/


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





//==============================================================================
bool ImogenAudioProcessor::hasEditor() const {
	return true; // (change this to false if you choose to not supply an editor)
};

juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor() {
	return new ImogenAudioProcessorEditor (*this);
};

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
	auto state = tree.copyState();
	std::unique_ptr<juce::XmlElement> xml (state.createXml());
	copyXmlToBinary (*xml, destData);
};

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
	
	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName (tree.state.getType()))
			tree.replaceState (juce::ValueTree::fromXml (*xmlState));
};

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new ImogenAudioProcessor();
};
//==============================================================================
