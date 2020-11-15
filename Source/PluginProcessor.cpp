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
		tree(*this, nullptr, "PARAMETERS", createParameters()),
		midiProcessor(harmEngine),
		midiLatch(false),
		prevAttack(0.0f), prevDecay(0.0f), prevSustain(0.0f), prevRelease(0.0f),
		previousStereoWidth(0.0f),
		prevVelocitySens(0.0f),
		prevPitchBendUp(0.0f), prevPitchBendDown(0.0f),
		latchIsOn(false), previousLatch(false),
		stealingIsOn(true),
		previousmidipan(64),
		previousMasterDryWet(100),
		dryMultiplier(0.0f), wetMultiplier(1.0f)

#endif
{
	
	// initializes each instance of the HarmonyVoice class inside the harmEngine array:
	for (int i = 0; i < numVoices; ++i) {
		harmEngine.add(new HarmonyVoice(i));
	}
	
	wetBuffer.setSize(2, 512);
	dryBuffer.setSize(2, 512);
	
	dryvoxpanningmults[0] = 64;
	dryvoxpanningmults[1] = 64;
}

ImogenAudioProcessor::~ImogenAudioProcessor() {
	for (int i = 0; i < numVoices; ++i) {
		delete harmEngine[i];
	}
	
}


AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
	std::vector<std::unique_ptr<RangedAudioParameter>> params;
	
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrAttack", "ADSR Attack", NormalisableRange<float> (0.01f, 1.0f), 0.035f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrDecay", "ADSR Decay", NormalisableRange<float> (0.01f, 1.0f), 0.06f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrSustain", "ADSR Sustain", NormalisableRange<float> (0.01f, 1.0f), 0.8f));
	params.push_back(std::make_unique<AudioParameterFloat> ("adsrRelease", "ADSR Release", NormalisableRange<float> (0.01f, 1.0f), 0.1f));
	params.push_back(std::make_unique<AudioParameterFloat> ("stereoWidth", "Stereo Width", NormalisableRange<float> (0.0, 100.0), 100));
	params.push_back(std::make_unique<AudioParameterInt>("dryPan", "Dry vox pan", 0, 127, 64));
	params.push_back(std::make_unique<AudioParameterInt>("masterDryWet", "% wet", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", NormalisableRange<float> (0.0, 100.0), 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("PitchBendUpRange", "Pitch bend range (up)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("PitchBendDownRange", "Pitch bend range (down)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f), 0.0));
	params.push_back(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f), -4.0));
	params.push_back(std::make_unique<AudioParameterBool>("midiLatch", "MIDI Latch", false));
	params.push_back(std::make_unique<AudioParameterBool>("voiceStealing", "Voice stealing", false));
	params.push_back(std::make_unique<AudioParameterInt>("inputChan", "Input channel", 0, 99, 0));
	
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
	lastBlockSize = samplesPerBlock;
	
	// DSP settings
	{
		if (prevLastSampleRate != lastSampleRate || prevLastBlockSize != lastBlockSize)
		{
			for (int i = 0; i < numVoices; ++i) {
				harmEngine[i]->updateDSPsettings(lastSampleRate, lastBlockSize);
			}
			prevLastSampleRate = lastSampleRate;
			prevLastBlockSize = lastBlockSize;
		}
		if (wetBuffer.getNumSamples() != samplesPerBlock) {
			wetBuffer.setSize(2, samplesPerBlock, true, true, true);
		}
		if(dryBuffer.getNumSamples() != samplesPerBlock) {
			dryBuffer.setSize(2, samplesPerBlock, true, true, true);
		}
		for(int i = 0; i < numVoices; ++i) {
			harmEngine[i]->checkBufferSizes(samplesPerBlock);
		}
	}
	
	// ADSR settings (...and also midi velocity sensitivity)
	if (prevAttack != *adsrAttackListener || prevDecay != *adsrDecayListener || prevSustain != *adsrSustainListener || prevRelease != *adsrReleaseListener || prevVelocitySens != *midiVelocitySensListener)
	{
		for (int i = 0; i < numVoices; ++i) {
			harmEngine[i]->adsrSettingsListener(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener, midiVelocitySensListener);
		}
		prevAttack = *adsrAttackListener;
		prevDecay = *adsrDecayListener;
		prevSustain = *adsrSustainListener;
		prevRelease = *adsrReleaseListener;
		prevVelocitySens = *midiVelocitySensListener;
	}
	
	// stereo width
	if (previousStereoWidth != *stereoWidthListener) {
		midiProcessor.updateStereoWidth(stereoWidthListener);
		previousStereoWidth = *stereoWidthListener;
	}
	
	// dry vox pan
	if(*dryVoxPanListener != previousmidipan) {
		const float panR = (*dryVoxPanListener)/127.0;
		const float panL = 1.0 - panR;
		dryvoxpanningmults[0] = panL;
		dryvoxpanningmults[1] = panR;
		
		previousmidipan = *dryVoxPanListener;
	}
	
	// pitch bend settings
	if (prevPitchBendUp != *pitchBendUpListener || prevPitchBendDown != * pitchBendDownListener) {
		for (int i = 0; i < numVoices; ++i) {
			harmEngine[i]->pitchBendSettingsListener(pitchBendUpListener, pitchBendDownListener);
		}
		prevPitchBendUp = *pitchBendUpListener;
		prevPitchBendDown = *pitchBendDownListener;
	}
}

void ImogenAudioProcessor::releaseResources() {
	
	wetBuffer.clear();
	dryBuffer.clear();
	for(int i = 0; i < numVoices; ++i) {
		harmEngine[i]->clearBuffers();
	}
	
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
	
	if(*masterDryWetListener != previousMasterDryWet) {
		wetMultiplier = (*masterDryWetListener)/100.0f;
		dryMultiplier = 1.0 - wetMultiplier;
	}
	
	const auto numSamples = buffer.getNumSamples();
	
	int inputChannel = *inputChannelListener;
	if (inputChannel > buffer.getNumChannels()) {
		inputChannel = buffer.getNumChannels() - 1;
	}

	// check buffer sizes
	if (wetBuffer.getNumSamples() != numSamples) {
		wetBuffer.setSize(2, numSamples, true, true, true);
	}
	if(dryBuffer.getNumSamples() != numSamples) {
		dryBuffer.setSize(2, numSamples, true, true, true);
	}
	
	// update settings/parameters
	{
		if(previousStereoWidth != *stereoWidthListener) {  // update stereo width, if the value has changed
			midiProcessor.updateStereoWidth(stereoWidthListener); // update array of possible panning values
			// update active voices' assigned panning values
			int activeVoiceNumber = 0;
			for (int i = 0; i < numVoices; ++i) {
				if(harmEngine[i]->voiceIsOn) {
					midiProcessor.refreshMidiPanVal(i, activeVoiceNumber);
					++activeVoiceNumber;
				}
			}
		}
		// pitch bend settings -- need to update all voices' pitch bend settings, even if voice is currently off !
		if (prevPitchBendUp != *pitchBendUpListener || prevPitchBendDown != * pitchBendDownListener) {
			for(int i = 0; i < numVoices; ++i) {
				harmEngine[i]->pitchBendSettingsListener(pitchBendUpListener, pitchBendDownListener);
			}
		}
		
		latchIsOn = *midiLatchListener > 0.5f;
		if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(); }
		
		stealingIsOn = *voiceStealingListener > 0.5f;
	}
	
	midiProcessor.processIncomingMidi(midiMessages, latchIsOn, stealingIsOn);
	
	// need to update the voxCurrentPitch variable!!
	// identify grain lengths & peak locations ONCE based on input signal, then pass info to individual instances of shifter ?
	
	inputGainMultiplier = Decibels::decibelsToGain(*inputGainListener);
	outputGainMultiplier = Decibels::decibelsToGain(*outputGainListener);
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	buffer.applyGain(0, 0, numSamples, inputGainMultiplier); // apply input gain to input buffer
	
	wetBuffer.clear();
	
	const float* readPointer = buffer.getReadPointer(inputChannel);
	
	analyzeInput(buffer, inputChannel);
	
	writeToDryBuffer(readPointer, dryBuffer, numSamples);
	
	// this for loop steps through each of the 12 instances of HarmonyVoice to render their audio:
	for (int i = 0; i < numVoices; i++) {  // i = the harmony voice # currently being processed
		if (harmEngine[i]->voiceIsOn) {  // only do audio processing on active voices:
			
			// update ADSR parameters & other settings, if they have changed
			{
				if(prevAttack != *adsrAttackListener || prevDecay != *adsrDecayListener || prevSustain != *adsrSustainListener || prevRelease != *adsrReleaseListener || prevVelocitySens != *midiVelocitySensListener) {
				harmEngine[i]->adsrSettingsListener(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener, midiVelocitySensListener);
				}
			}
			
			// render next audio vector (writes pitch shifted samples to HarmonyVoice's stereo harmonyBuffer)
			harmEngine[i]->renderNextBlock(buffer, readPointer, numSamples, inputChannel, voxCurrentPitch);
			
			// writes shifted sample values to wetBuffer
			for (int channel = 0; channel < numChannels; ++channel) {
				const float* reading = harmEngine[i]->harmonyBuffer.getReadPointer(channel);
				float* output = wetBuffer.getWritePointer(channel);
				
				for(int sample = 0; sample < numSamples; ++sample) {
					output[sample] = (output[sample] + reading[sample])/2.0f;  // add value TO the wetBuffer instead of overwriting
				}
			}
		}
	}
	// goal is to add all active voices' audio together into wetBuffer !!

	buffer.clear();
	{
	// plan B if that doesnt work:
//		if (buffer.getNumChannels() > 2) {
//			for (int i = 3; i <= buffer.getNumChannels(); ++i) {
//				buffer.clear(i, 0, numSamples);
//			}
//		}
	}
	
	// place shifted audio samples from wetBuffer into buffer
	for (int channel = 0; channel < numChannels; ++channel)
	{
		const float* wetreadPointer = wetBuffer.getReadPointer(channel);
		const float* dryreadPointer = dryBuffer.getReadPointer(channel);
		float* outputSample = buffer.getWritePointer(channel);
		
		for (int sample = 0; sample < numSamples; ++sample)
		{
			outputSample[sample] = ((wetreadPointer[sample] * wetMultiplier) + (dryreadPointer[sample] * dryMultiplier))/2.0f; // mix dry & wet signals together & output to speakers
		}
	}
	
	buffer.applyGain(0, 0, numSamples, outputGainMultiplier);
	buffer.applyGain(1, 0, numSamples, outputGainMultiplier);
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
	
	// update storage of previous frame's parameters, for comparison when the NEXT frame comes in...
	{
		prevAttack = *adsrAttackListener;
		prevDecay = *adsrDecayListener;
		prevSustain = *adsrSustainListener;
		prevRelease = *adsrReleaseListener;
		prevVelocitySens = *midiVelocitySensListener;
		previousLatch = latchIsOn;
		previousStereoWidth = *stereoWidthListener;
		previousMasterDryWet = *masterDryWetListener;
		prevPitchBendUp = *pitchBendUpListener;
		prevPitchBendDown = *pitchBendDownListener;
		previousmidipan = *dryVoxPanListener; // this is the panning for the dry vox signal
	}
}


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

void ImogenAudioProcessor::analyzeInput (AudioBuffer<float> input, const int inputChan) {
	
};


void ImogenAudioProcessor::writeToDryBuffer (const float* readingPointer, AudioBuffer<float>& dryBuffer, const int numSamples) {
	
	// move samples from input buffer (stereo channel) to dryBuffer (stereo buffer, panned according to midiPan)
	
	// NEED TO ACCOUNT FOR LATENCY OF THE HARMONY ALGORITHM !!!
	
	if(*dryVoxPanListener != previousmidipan) {
		const float panR = (*dryVoxPanListener)/127.0;
		const float panL = 1.0 - panR;
		dryvoxpanningmults[0] = panL;
		dryvoxpanningmults[1] = panR;
	}
	
	for(int channel = 0; channel < 2; ++channel) {
		float* drywriting = dryBuffer.getWritePointer(channel);
		for (int sample = 0; sample < numSamples; ++sample) {
			drywriting[sample] = readingPointer[sample] * dryvoxpanningmults[channel];
		}
	}
	
};
