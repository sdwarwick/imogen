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
		tree (*this, nullptr, "PARAMETERS", createParameters()),
		midiLatch(false),
		historyLength(400),
		prevAttack(0.0f), prevDecay(0.0f), prevSustain(0.0f), prevRelease(0.0f),
		previousStereoWidth(0.0f),
		prevVelocitySens(0.0f),
		prevPitchBendUp(0.0f), prevPitchBendDown(0.0f),
		latchIsOn(false), previousLatch(false),
		mixBufferPos(0),
		stealingIsOn(true)

#endif
{
	
	// initializes each instance of the HarmonyVoice class inside the harmEngine array:
	for (int i = 0; i < numVoices; ++i) {
		harmEngine.add(new HarmonyVoice(i));
	}
	
	for(int i = 0; i < historyLength; ++i) {
		history.add(0);
	}
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
	params.push_back(std::make_unique<AudioParameterFloat> ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", NormalisableRange<float> (0.0, 100.0), 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("PitchBendUpRange", "Pitch bend range (up)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("PitchBendDownRange", "Pitch bend range (down)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f), 0.0));
	params.push_back(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f), -4.0));
	params.push_back(std::make_unique<AudioParameterBool>("midiLatch", "MIDI Latch", false));
	params.push_back(std::make_unique<AudioParameterBool>("voiceStealing", "Voice stealing", false));
	
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
void ImogenAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
	
	lastSampleRate = sampleRate;
	lastBlockSize = samplesPerBlock;
	
	// DSP settings
	if (prevLastSampleRate != lastSampleRate || prevLastBlockSize != lastBlockSize)
	{
		for (int i = 0; i < numVoices; ++i) {
			harmEngine[i]->updateDSPsettings(lastSampleRate, lastBlockSize);
		}
		prevLastSampleRate = lastSampleRate;
		prevLastBlockSize = lastBlockSize;
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
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
	
	const auto numSamples = buffer.getNumSamples();
	
	// update buffer sizes
	{
		// check buffer sizes
		if (processingBuffer.getNumSamples() != numSamples) {
			processingBuffer.setSize(2, numSamples, true, true, true);
			wetBuffer.setSize(2, numSamples, true, true, true);
			outputBuffer.setSize(2, numSamples, true, true, true);
			
			if ((2 * numSamples) + (2 * getLatencySamples()) > mixBuffer.getNumSamples()) {
				mixBuffer.setSize(2, (2 * numSamples) + (2 * getLatencySamples()), true, true, true);
			}
		}
	}
	
	// update settings/parameters
	{
		if(previousStereoWidth != *stereoWidthListener) {  // update stereo width, if the value has changed
			midiProcessor.updateStereoWidth(stereoWidthListener); // update array of possible panning values
			// update active voices' assigned panning values
			int activeVoiceNumber = 0;
			for (int i = 0; i < numVoices; ++i) {
				if(harmEngine[i]->voiceIsOn) {
					midiProcessor.refreshMidiPanVal(harmEngine, i, activeVoiceNumber);
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
		if(latchIsOn == false && previousLatch == true) { midiProcessor.turnOffLatch(harmEngine); }
		
		stealingIsOn = *voiceStealingListener > 0.5f;
	}
	
	midiProcessor.processIncomingMidi(midiMessages, harmEngine, latchIsOn, stealingIsOn);
	
	// need to update the voxCurrentPitch variable!!
	// identify grain lengths & peak locations ONCE based on input signal, then pass info to individual instances of shifter ?
	
	inputGainMultiplier = Decibels::decibelsToGain(*inputGainListener);
	outputGainMultiplier = Decibels::decibelsToGain(*outputGainListener);
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	buffer.applyGain(0, 0, numSamples, inputGainMultiplier); // apply input gain to input buffer
	
	// this for loop steps through each of the 12 instances of HarmonyVoice to render their audio:
	for (int i = 0; i < numVoices; i++) {  // i = the harmony voice # currently being processed
		if (harmEngine[i]->voiceIsOn) {  // only do audio processing on active voices:
			
//	 1	// update ADSR parameters & other settings, if they have changed
			{
				// adsr settings & midi velocity sensitivity
				if(prevAttack != *adsrAttackListener || prevDecay != *adsrDecayListener || prevSustain != *adsrSustainListener || prevRelease != *adsrReleaseListener || prevVelocitySens != *midiVelocitySensListener) {
				harmEngine[i]->adsrSettingsListener(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener, midiVelocitySensListener);
				}
			}
			
	// 2	// render next audio vector
			harmEngine[i]->renderNextBlock(buffer, 0, buffer.getNumSamples(), voxCurrentPitch);
		}
	}
	// goal is to add all 12 voices together into a master audio signal for harmEngine, which can then be mixed with the original input signal (dry/wet)
	// make sure to be ADDING to same audio vector, and not OVERWRITING that sample.
	
	
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
	
	// HISTORY array, for GUI visualization:
	{
		for(int i = 0; i < numSamples; ++i) {
			// just adding a sample every 20 or so...
			if(i%20 == 0) {
				const float sample = (buffer.getSample(0, i)/2) + (buffer.getSample(1, i) / 2);
				history.add(sample);
				
				if (history.size() > historyLength) {
							history.remove(0);
				}
			}
		}
	}
	
	// update storage of previous frame's parameters, for comparison when the NEXT frame comes in...
	{
		prevAttack = *adsrAttackListener;
		prevDecay = *adsrDecayListener;
		prevSustain = *adsrSustainListener;
		prevRelease = *adsrReleaseListener;
		prevVelocitySens = *midiVelocitySensListener;
		previousLatch = latchIsOn;
		previousStereoWidth = *stereoWidthListener;
		prevPitchBendUp = *pitchBendUpListener;
		prevPitchBendDown = *pitchBendDownListener;
	}
}


/*===========================================================================================================================
 ============================================================================================================================*/


void ImogenAudioProcessor::killAllMidi() {
	midiProcessor.killAll(harmEngine);
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
