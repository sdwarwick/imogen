#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(makeBusProperties()),
		tree(*this, nullptr, "PARAMETERS", createParameters()),
		minimumSubBlockSize(16),
		subBlockSubdivisionIsStrict(false),
		currentInputPitch(0.0f),
		lastSampleRate(44100), lastBlockSize(512),
		adsrIsOn(true),
		previousStereoWidth(100.0f),
		prevQuickKillMs(0),
		prevConcertPitch(440),
		previousmidipan(64),
		prevideb(0.0f), prevodeb(0.0f),
		limiterIsOn(true),
		wetBuffer(2, MAX_BUFFERSIZE),
		dryBuffer(2, MAX_BUFFERSIZE),
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

#endif
{
	for (int i = 0; i < 13; ++i) { harmonizer.addVoice(new HarmonizerVoice); }
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
	
	epochIndices.ensureStorageAllocated(MAX_BUFFERSIZE);
	epochIndices.clearQuick();
};

ImogenAudioProcessor::~ImogenAudioProcessor()
{
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock) {
	
	// sample rate
	if(lastSampleRate != sampleRate)
	{
		harmonizer.setCurrentPlaybackSampleRate(sampleRate);
		lastSampleRate = sampleRate;
		// update latency for dry/wet mixer !
	}
	
	 // block size
	{
		const int newblocksize = samplesPerBlock > MAX_BUFFERSIZE ? MAX_BUFFERSIZE : samplesPerBlock;

		if(lastBlockSize != newblocksize)
		{
			lastBlockSize = newblocksize;
			if(wetBuffer.getNumSamples() != newblocksize)
				wetBuffer.setSize(2, newblocksize, true, true, true);
			if(dryBuffer.getNumSamples() != newblocksize)
				dryBuffer.setSize(2, newblocksize, true, true, true);
		}
	}
	
	// concert pitch
	if(int(concertPitchListener) != prevConcertPitch)
		harmonizer.setConcertPitchHz(concertPitchListener);
	
	wetBuffer.clear();
	
	updateIOgains();
	updateAdsr();
	updateStereoWidth();
	updateDryVoxPan();
	updateQuickKillMs();
	harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
	harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
	harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
	
	dspSpec.sampleRate = sampleRate;
	dspSpec.maximumBlockSize = MAX_BUFFERSIZE * 2;
	dspSpec.numChannels = 2;
	
	// limiter
	limiter.prepare(dspSpec);
	updateLimiter();
	
	// dry wet mixer
	dryWet.prepare(dspSpec);
	dryWet.setMixingRule(dsp::DryWetMixingRule::linear);
	dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
	dryWet.setWetLatency(2); // latency in samples of the ESOLA algorithm
	
	savePrevParamValues();
};



void ImogenAudioProcessor::releaseResources() {
	
	wetBuffer.clear();
	dryBuffer.clear();
	harmonizer.resetNoteOnCounter();
};



void ImogenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// update settings & parameters
	{
		updateIOgains();
		updateLimiter();
		updateAdsr();
		updateStereoWidth();
		updateDryVoxPan();
		updateQuickKillMs();
		harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
		harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
		harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
		
		dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
		
		// concert pitch
		if(int(concertPitchListener) != prevConcertPitch)
			harmonizer.setConcertPitchHz(concertPitchListener);
	}
	
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
		
		if (samplesToNextMidiMessage < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
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
	
	savePrevParamValues();
};




void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& buffer, const int inputChannel, const int startSample, const int numSamples)
{
	// update settings & parameters
	{
		updateIOgains();
		updateLimiter();
		updateAdsr();
		updateStereoWidth();
		updateDryVoxPan();
		updateQuickKillMs();
		harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
		harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
		harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
		
		dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
		
		// concert pitch
		if(int(concertPitchListener) != prevConcertPitch)
			harmonizer.setConcertPitchHz(concertPitchListener);
	}
	
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
	
	savePrevParamValues();
};



void ImogenAudioProcessor::renderChunk(AudioBuffer<float>& buffer, const int inputChannel)
{
	// update settings & parameters
	{
		updateIOgains();
		updateLimiter();
		updateAdsr();
		updateStereoWidth();
		updateDryVoxPan();
		updateQuickKillMs();
		harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
		harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
		harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
		
		dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
		
		// concert pitch
		if(int(concertPitchListener) != prevConcertPitch)
			harmonizer.setConcertPitchHz(concertPitchListener);
	}
	
	const int numSamples = buffer.getNumSamples();
	
	// buffer sizes
	if(wetBuffer.getNumSamples() != numSamples)
		wetBuffer.setSize(2, numSamples, true, true, true);
	if(dryBuffer.getNumSamples() != numSamples)
		dryBuffer.setSize(2, numSamples, true, true, true);
	
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	wetBuffer.clear();
	
	buffer.applyGain(inputChannel, 0, numSamples, inputGainMultiplier); // apply input gain
	
	// copy input signal to dryBuffer & apply panning
	dryBuffer.copyFrom(0, 0, buffer, inputChannel, 0, numSamples);
	dryBuffer.applyGain(0, 0, numSamples, dryvoxpanningmults[0]);
	dryBuffer.copyFrom(1, 0, buffer, inputChannel, 0, numSamples);
	dryBuffer.applyGain(1, 0, numSamples, dryvoxpanningmults[1]);
	
	dsp::AudioBlock<float> dwinblock(dryBuffer);
	dryWet.pushDrySamples(dwinblock);
	
	currentInputPitch = pitch.getPitch(buffer, inputChannel, lastSampleRate);
	
	harmonizer.setCurrentInputFreq(currentInputPitch); // do this here if possible? input pitch should be calculated/updated as frequently as possible
	
	harmonizer.renderVoices(buffer, inputChannel, numSamples, wetBuffer, epochIndices); // puts the harmonizer's rendered stereo output samples into "buffer"
	
	// clear any extra channels present in I/O buffer
	{
		if (buffer.getNumChannels() > 2)
			for (int i = 3; i <= buffer.getNumChannels(); ++i)
				buffer.clear(i - 1, 0, numSamples);
	}
	
	dsp::AudioBlock<float> dwoutblock (buffer);
	dryWet.mixWetSamples(dwoutblock);
	
	buffer.applyGain(0, numSamples, outputGainMultiplier); // apply master output gain
	
	// output limiter
	if(limiterIsOn)
	{
		dsp::AudioBlock<float> limiterBlock (buffer);
		limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
	}
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
	savePrevParamValues();
};



/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::updateAdsr()
{
	adsrIsOn = adsrOnOffListener > 0.5f;
	harmonizer.setADSRonOff(adsrIsOn);
	
	if (adsrIsOn)
		harmonizer.updateADSRsettings(adsrAttackListener, adsrDecayListener, adsrSustainListener, adsrReleaseListener);
};


void ImogenAudioProcessor::updateIOgains()
{
	if(inputGainListener != prevideb) {
		inputGainMultiplier = Decibels::decibelsToGain(float(inputGainListener));
		prevideb = inputGainListener;
	}
	if(outputGainListener != prevodeb) {
		outputGainMultiplier = Decibels::decibelsToGain(float(outputGainListener));
		prevodeb = outputGainListener;
	}
};


void ImogenAudioProcessor::updateLimiter()
{
	limiter.setThreshold(limiterThreshListener);
	limiter.setRelease(limiterReleaseListener);
	limiterIsOn = limiterToggleListener > 0.5f;
};


void ImogenAudioProcessor::updateStereoWidth()
{
	harmonizer.updateLowestPannedNote(round(lowestPanListener));
	
	if (previousStereoWidth != int(stereoWidthListener))
	{
		harmonizer.updateStereoWidth(stereoWidthListener);
		previousStereoWidth = int(stereoWidthListener);
	}
};


void ImogenAudioProcessor::updateQuickKillMs()
{
	if(prevQuickKillMs != int(quickKillMsListener))
	{
		harmonizer.updateQuickReleaseMs(quickKillMsListener);
		prevQuickKillMs = int(quickKillMsListener);
	}
};


void ImogenAudioProcessor::updateDryVoxPan()
{
	if(int(dryVoxPanListener) != previousmidipan)
	{
		const float Rpan = dryVoxPanListener / 127.0f;
		dryvoxpanningmults[1] = Rpan;
		dryvoxpanningmults[0] = 1.0f - Rpan;
		previousmidipan = dryVoxPanListener;
	}
};


void ImogenAudioProcessor::updateNumVoices(const int newNumVoices)
{
	if(const int currentVoices = harmonizer.getNumVoices(); currentVoices != newNumVoices)
	{
		if(newNumVoices > currentVoices) 
		{
			const int voicesToAdd = newNumVoices - currentVoices;
			for(int i = 0; i < voicesToAdd; ++i)
				harmonizer.addVoice(new HarmonizerVoice);
		}
		else
		{
			harmonizer.removeNumVoices(currentVoices - newNumVoices);
		}
	}
};

void ImogenAudioProcessor::savePrevParamValues()
{
	prevideb = inputGainListener;
	prevodeb = outputGainListener;
	previousStereoWidth = int(stereoWidthListener);
	prevQuickKillMs = int(quickKillMsListener);
	previousmidipan = int(dryVoxPanListener);
};

void ImogenAudioProcessor::setMinimumRenderingSubdivisionSize (const int numSamples, const bool shouldBeStrict) noexcept
{
	// Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering. When rendering, the audio blocks that are passed into processBlock() will be split up into smaller blocks that lie between all the incoming midi messages, and it is these smaller sub-blocks that are rendered with multiple calls to harmonizer.renderVoices() via processBlockPrivate(). Obviously in a pathological case where there are midi messages on every sample, then renderVoices() could be called once per sample and lead to poor performance, so this setting allows you to set a lower limit on the block size.
	// If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples. If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate (this can sometimes help to avoid quantisation or phasing issues).
	jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
	minimumSubBlockSize = numSamples;
	subBlockSubdivisionIsStrict = shouldBeStrict;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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



void ImogenAudioProcessor::saveNewPreset()
{
	
};


void ImogenAudioProcessor::updatePreset()
{
	
};


void ImogenAudioProcessor::loadPreset()
{
	
};



AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties()
{
	PluginHostType host;
	if(host.isLogic() || host.isGarageBand())
		return BusesProperties().withInput("Input", AudioChannelSet::mono(), false)
		.withInput("Sidechain", AudioChannelSet::mono(), true)
		.withOutput("Output", AudioChannelSet::stereo(), true);
	else
		return BusesProperties().withInput("Input", AudioChannelSet::mono(), true)
		.withOutput("Output", AudioChannelSet::stereo(), true);
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
			tree.replaceState(juce::ValueTree::fromXml (*xmlState));
};


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

bool ImogenAudioProcessor::hasEditor() const {
	return true; // (change this to false if you choose to not supply an editor)
};

juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor() {
	return new ImogenAudioProcessorEditor (*this);
};

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new ImogenAudioProcessor();
};
