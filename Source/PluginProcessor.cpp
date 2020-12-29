#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor():
#ifndef JucePlugin_PreferredChannelConfigurations
	AudioProcessor(makeBusProperties()),
#endif
	tree(*this, nullptr, "PARAMETERS", createParameters()),
	wetBuffer(2, MAX_BUFFERSIZE), dryBuffer(2, MAX_BUFFERSIZE),
	lastSampleRate(44100), limiterIsOn(true), inputGainMultiplier(1.0f), outputGainMultiplier(1.0f), currentInputPitch(0.0f),
	prevDryPan(64), prevideb(0.0f), prevodeb(0.0f),
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
	inputGainListener(*tree.getRawParameterValue("inputGain")),
	outputGainListener(*tree.getRawParameterValue("outputGain")),
	voiceStealingListener(*tree.getRawParameterValue("voiceStealing")),
	inputChannelListener(*tree.getRawParameterValue("inputChan")),
	dryVoxPanListener(*tree.getRawParameterValue("dryPan")),
	masterDryWetListener(*tree.getRawParameterValue("masterDryWet")),
	limiterThreshListener(*tree.getRawParameterValue("limiterThresh")),
	limiterReleaseListener(*tree.getRawParameterValue("limiterRelease")),
	limiterToggleListener(*tree.getRawParameterValue("limiterIsOn")),
	quickKillMsListener(*tree.getRawParameterValue("quickKillMs")),
	concertPitchListener(*tree.getRawParameterValue("concertPitch"))
{
	for (int i = 0; i < 12; ++i) { harmonizer.addVoice(new HarmonizerVoice(&harmonizer)); }
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
	
	epochIndices.ensureStorageAllocated(MAX_BUFFERSIZE);
	epochIndices.clearQuick();
};

ImogenAudioProcessor::~ImogenAudioProcessor()
{ };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock) {
	
	// sample rate
	if(lastSampleRate != sampleRate)
	{
		harmonizer.setCurrentPlaybackSampleRate(sampleRate);
		lastSampleRate = sampleRate;
		// update latency for dry/wet mixer !
	}
	
	wetBuffer.clear();
	dryBuffer.clear();
	
	dspSpec.sampleRate = sampleRate;
	dspSpec.maximumBlockSize = MAX_BUFFERSIZE * 2;
	dspSpec.numChannels = 2;
	
	// limiter
	limiter.prepare(dspSpec);
	
	// dry wet mixer
	dryWet.prepare(dspSpec);
	dryWet.setMixingRule(dsp::DryWetMixingRule::linear);
	dryWet.setWetLatency(2); // latency in samples of the ESOLA algorithm
	
	updateAllParameters();
	harmonizer.resetNoteOnCounter(); // ??
};


void ImogenAudioProcessor::releaseResources()
{
	wetBuffer.clear();
	dryBuffer.clear();
	harmonizer.resetNoteOnCounter();
};


// audio rendering ----------------------------------------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	const int inputChannel = inputChannelListener >= buffer.getNumChannels() ? buffer.getNumChannels() - 1 : int(inputChannelListener);
	
	epochIndices = epochs.extractEpochSampleIndices(buffer, inputChannel, lastSampleRate); // this only needs to be done once per top-level processBlock call, because epoch locations are not dependant on the size of the rendered chunks...
	
	auto midiIterator = midiMessages.findNextSamplePosition(0);
	
	int numSamples = buffer.getNumSamples();
	int startSample = 0;
	bool firstEvent = true;
	
	for (; numSamples > 0; ++midiIterator)
	{
		if (midiIterator == midiMessages.cend())
		{
			processBlockPrivate(buffer, inputChannel, startSample, numSamples);
			return;
		}
		
		const auto metadata = *midiIterator;
		const int samplesToNextMidiMessage = metadata.samplePosition - startSample;
		
		if (samplesToNextMidiMessage >= numSamples)
		{
			processBlockPrivate(buffer, inputChannel, startSample, numSamples);
			harmonizer.handleMidiEvent(metadata.getMessage());
			break;
		}
		
		if (samplesToNextMidiMessage < 1)
		{
			harmonizer.handleMidiEvent(metadata.getMessage());
			continue;
		}
		
		firstEvent = false;
	
		processBlockPrivate(buffer, inputChannel, startSample, samplesToNextMidiMessage);
		
		harmonizer.handleMidiEvent(metadata.getMessage());
		startSample += samplesToNextMidiMessage;
		numSamples  -= samplesToNextMidiMessage;
	}
	
	std::for_each (midiIterator,
				   midiMessages.cend(),
				   [&] (const MidiMessageMetadata& meta) { harmonizer.handleMidiEvent (meta.getMessage()); });
	
};


void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& buffer, const int inputChannel, const int startSample, const int numSamples)
{
	int samplesLeft = numSamples;
	int chunkStartSample = startSample;
	
	while(samplesLeft > 0)
	{
		const int chunkNumSamples = samplesLeft > MAX_BUFFERSIZE ? MAX_BUFFERSIZE : samplesLeft;
		
		AudioBuffer<float> proxy(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), chunkStartSample, chunkNumSamples);
		
		renderChunk(proxy, inputChannel);
		
		samplesLeft -= chunkNumSamples;
		chunkStartSample += chunkNumSamples;
	}
};


void ImogenAudioProcessor::renderChunk(AudioBuffer<float>& buffer, const int inputChannel)
{
	updateAllParameters();
	
	const int numSamples = buffer.getNumSamples();
	
	// buffer sizes
	{
	if(wetBuffer.getNumSamples() != numSamples)
		wetBuffer.setSize(2, numSamples, true, true, true);
	if(dryBuffer.getNumSamples() != numSamples)
		dryBuffer.setSize(2, numSamples, true, true, true);
	}
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	wetBuffer.clear();
	dryBuffer.clear();
	
	buffer.applyGain(inputChannel, 0, numSamples, inputGainMultiplier); // apply input gain
	
	// copy input signal to dryBuffer & apply panning
	{
		dryBuffer.copyFrom(0, 0, buffer, inputChannel, 0, numSamples);
		dryBuffer.applyGain(0, 0, numSamples, dryvoxpanningmults[0]);
		dryBuffer.copyFrom(1, 0, buffer, inputChannel, 0, numSamples);
		dryBuffer.applyGain(1, 0, numSamples, dryvoxpanningmults[1]);
	}
	
	dsp::AudioBlock<float> dwinblock(dryBuffer);
	dryWet.pushDrySamples(dwinblock);
	
	currentInputPitch = pitch.getPitch(buffer, inputChannel, lastSampleRate);
	
	harmonizer.setCurrentInputFreq(currentInputPitch); // do this here if possible? input pitch should be calculated/updated as frequently as possible
	
	harmonizer.renderVoices(buffer, inputChannel, numSamples, wetBuffer, epochIndices); // puts the harmonizer's rendered stereo output into "wetBuffer"
	
	// clear any extra channels present in I/O buffer
	{
		if (buffer.getNumChannels() > 2)
			for (int i = 3; i <= buffer.getNumChannels(); ++i)
				buffer.clear(i - 1, 0, numSamples);
	}
	
	dsp::AudioBlock<float> dwoutblock (wetBuffer);
	dryWet.mixWetSamples(dwoutblock); // puts the mixed dry & wet samples into wetBuffer
	
	buffer.makeCopyOf(wetBuffer, true); // transfer from wetBuffer to I/O buffer
	
	buffer.applyGain(0, numSamples, outputGainMultiplier); // apply master output gain
	
	// output limiter
	if(limiterIsOn)
	{
		dsp::AudioBlock<float> limiterBlock (buffer);
		limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
	}
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
};


/*===========================================================================================================================
 ============================================================================================================================*/


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::updateAllParameters()
{
	updateIOgains();
	updateLimiter();
	updateAdsr();
	updateStereoWidth();
	updateDryVoxPan();
	updateQuickKillMs();
	updateMidiVelocitySensitivity();
	updateNoteStealing();
	updatePitchbendSettings();
	updateDryWet();
	updateConcertPitch();
};


void ImogenAudioProcessor::updateAdsr()
{
	harmonizer.setADSRonOff(float(adsrOnOffListener) > 0.5f);
	harmonizer.updateADSRsettings(float(adsrAttackListener), float(adsrDecayListener), float(adsrSustainListener), float(adsrReleaseListener));
};


void ImogenAudioProcessor::updateIOgains()
{
	if(const float newIn = float(inputGainListener); newIn != prevideb)
	{
		inputGainMultiplier = Decibels::decibelsToGain(newIn);
		prevideb = newIn;
	}
	
	if(const float newOut = float(outputGainListener); newOut != prevodeb)
	{
		outputGainMultiplier = Decibels::decibelsToGain(newOut);
		prevodeb = newOut;
	}
};


void ImogenAudioProcessor::updateLimiter()
{
	limiter.setThreshold(float(limiterThreshListener));
	limiter.setRelease(float(limiterReleaseListener));
	limiterIsOn = float(limiterToggleListener) > 0.5f;
};


void ImogenAudioProcessor::updateStereoWidth()
{
	harmonizer.updateLowestPannedNote(int(round(float(lowestPanListener))));
	harmonizer.updateStereoWidth(int(round(float(stereoWidthListener))));
};


void ImogenAudioProcessor::updateQuickKillMs()
{
	harmonizer.updateQuickReleaseMs(int(round(float(quickKillMsListener))));
};


void ImogenAudioProcessor::updateDryVoxPan()
{
	if(const int newDryPan = int(round(float(dryVoxPanListener))); newDryPan != prevDryPan)
	{
		const float Rpan = newDryPan / 127.0f;
		dryvoxpanningmults[1] = Rpan;
		dryvoxpanningmults[0] = 1.0f - Rpan;
		prevDryPan = newDryPan;
	}
};


void ImogenAudioProcessor::updateMidiVelocitySensitivity()
{
	harmonizer.updateMidiVelocitySensitivity(int(round(float(midiVelocitySensListener))));
};


void ImogenAudioProcessor::updateNoteStealing()
{
	harmonizer.setNoteStealingEnabled(float(voiceStealingListener) > 0.5f);
};


void ImogenAudioProcessor::updatePitchbendSettings()
{
	harmonizer.updatePitchbendSettings(int(round(float(pitchBendUpListener))), int(round(float(pitchBendDownListener))));
};


void ImogenAudioProcessor::updateDryWet()
{
	dryWet.setWetMixProportion(float(masterDryWetListener) / 100.0f);
	
	// need to set latency!!!
};


void ImogenAudioProcessor::updateConcertPitch()
{
	harmonizer.setConcertPitchHz(int(round(float(concertPitchListener))));
};


void ImogenAudioProcessor::updateNumVoices(const int newNumVoices)
{
	if(const int currentVoices = harmonizer.getNumVoices(); currentVoices != newNumVoices)
	{
		if(newNumVoices > currentVoices) 
		{
			for(int i = 0; i < newNumVoices - currentVoices; ++i)
				harmonizer.addVoice(new HarmonizerVoice(&harmonizer));
		}
		else
			harmonizer.removeNumVoices(currentVoices - newNumVoices);
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// functions for custom preset management system ----------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::savePreset(juce::String presetName) 
{
	// this function can be used both to save new preset files or to update existing ones
	
	File writingTo = getPresetsFolder().getChildFile(presetName);
	
	auto xml(tree.copyState().createXml());
	
	xml->writeTo(writingTo);
};


void ImogenAudioProcessor::loadPreset(juce::String presetName)
{
	File presetToLoad = getPresetsFolder().getChildFile(presetName);
	
	if(presetToLoad.existsAsFile())
	{
		auto xmlElement = juce::parseXML(presetToLoad);
		
		if(xmlElement.get() != nullptr && xmlElement->hasTagName (tree.state.getType()))
			tree.replaceState(juce::ValueTree::fromXml (*xmlElement));
	}
};


void ImogenAudioProcessor::deletePreset(juce::String presetName) const
{
	File presetToDelete = getPresetsFolder().getChildFile(presetName);
	
	if(presetToDelete.existsAsFile())
		if(! presetToDelete.moveToTrash())
			presetToDelete.deleteFile();
};


juce::File ImogenAudioProcessor::getPresetsFolder() const
{
	File rootFolder;
	
#ifdef JUCE_MAC
	rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
	rootFolder = rootFolder.getChildFile("Audio").getChildFile("Presets").getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
	
#ifdef JUCE_WINDOWS
	rootFolder = File::getSpecialLocation(File::SpecialLocationType::UserDocumentsDirectory);
	rootFolder = rootFolder.getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
	
#ifdef JUCE_LINUX
	rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
	rootFolder = rootFolder.getChildFile("Ben Vining Music Software").getChildFile("Imogen");
#endif
	
	if(! rootFolder.isDirectory() && ! rootFolder.existsAsFile())
		rootFolder.createDirectory(); // creates the presets folder if it doesn't already exist
	
	return rootFolder;
};


void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
	auto xml(tree.copyState().createXml());
	copyXmlToBinary (*xml, destData);
};


void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	auto xmlState(getXmlFromBinary(data, sizeInBytes));
	
	if (xmlState.get() != nullptr && xmlState->hasTagName (tree.state.getType()))
		tree.replaceState(juce::ValueTree::fromXml (*xmlState));
};


// standard and general-purpose functions -----------------------------------------------------------------------------------------------------------

AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties()
{
	PluginHostType host;
	if(host.isLogic() || host.isGarageBand())
		return BusesProperties().withInput("Input", AudioChannelSet::mono(), false).withInput("Sidechain", AudioChannelSet::mono(), true)
		.withOutput("Output", AudioChannelSet::stereo(), true);
	else
		return BusesProperties().withInput("Input", AudioChannelSet::mono(), true).withOutput("Output", AudioChannelSet::stereo(), true);
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
	params.push_back(std::make_unique<AudioParameterInt>("quickKillMs", "Quick kill ms", 1, 250, 15));
	params.push_back(std::make_unique<AudioParameterInt> ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
	params.push_back(std::make_unique<AudioParameterFloat> ("PitchBendUpRange", "Pitch bend range (up)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("PitchBendDownRange", "Pitch bend range (down)", NormalisableRange<float> (1.0f, 12.0f), 2));
	params.push_back(std::make_unique<AudioParameterFloat>("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), 0.0f));
	params.push_back(std::make_unique<AudioParameterFloat>("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -4.0f));
	params.push_back(std::make_unique<AudioParameterBool>("voiceStealing", "Voice stealing", false));
	params.push_back(std::make_unique<AudioParameterInt>("inputChan", "Input channel", 0, 16, 0));
	params.push_back(std::make_unique<AudioParameterInt>("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
	params.push_back(std::make_unique<AudioParameterFloat>("limiterThresh", "Limiter threshold (dBFS)", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -2.0f));
	params.push_back(std::make_unique<AudioParameterInt>("limiterRelease", "limiter release (ms)", 1, 250, 10));
	params.push_back(std::make_unique<AudioParameterBool>("limiterIsOn", "Limiter on/off", true));
	
	return { params.begin(), params.end() };
};


double ImogenAudioProcessor::getTailLengthSeconds() const
{
	if(harmonizer.isADSRon())
		return double(adsrReleaseListener); // ADSR release time in seconds
	else
		return double(quickKillMsListener * 1000.0f); // "quick kill" time in seconds
};

int ImogenAudioProcessor::getNumPrograms() {
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
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


juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor() {
	return new ImogenAudioProcessorEditor (*this);
};

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new ImogenAudioProcessor();
};
