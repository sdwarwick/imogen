#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(makeBusProperties()),
		voxCurrentPitch(0.0f),
		tree(*this, nullptr, "PARAMETERS", createParameters()),
		lastSampleRate(44100), lastBlockSize(512),
		frameIsPitched(false),
		adsrIsOn(true),
		previousStereoWidth(100.0f),
		pedalPitchToggle(false),
		pedalPitchThresh(127),
		previousmidipan(64),
		prevideb(0.0f), prevodeb(0.0f),
		limiterIsOn(true),
		wetBuffer(NUMBER_OF_CHANNELS, MAX_BUFFERSIZE),
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
	for (int i = 0; i < 13; ++i) { harmonizer.addVoice(new HarmonizerVoice); }
	
	harmonizer.setMinimumRenderingSubdivisionSize(64);
	
	dryvoxpanningmults[0] = 0.5f;
	dryvoxpanningmults[1] = 0.5f;
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
		const int newblocksize = samplesPerBlock >= MAX_BUFFERSIZE ? MAX_BUFFERSIZE : samplesPerBlock;
		
		if(lastBlockSize != newblocksize)
		{
			lastBlockSize = newblocksize;
			if(wetBuffer.getNumSamples() != newblocksize) { wetBuffer.setSize(NUMBER_OF_CHANNELS, newblocksize, true, true, true); }
		}
	}
	
	wetBuffer.clear();
	
	updateIOgains();
	updateAdsr();
	updateStereoWidth();
	harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
	harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
	harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);

	
	// dry vox pan
	{
		if(dryVoxPanListener != previousmidipan) {
			dryvoxpanningmults[1] = dryVoxPanListener / 127.0f;
			dryvoxpanningmults[0] = 1.0f - dryvoxpanningmults[1];
			previousmidipan = dryVoxPanListener;
		}
	}
	
	
	// MIDI pedal pitch
	{
		pedalPitchToggle = pedalPitchToggleListener > 0.5f;
		pedalPitchThresh = round(pedalPitchThreshListener);
	}
	
	// master dry/wet
	{
		//masterDryWetListener
	}
	
	dspSpec.sampleRate = sampleRate;
	dspSpec.maximumBlockSize = MAX_BUFFERSIZE;
	dspSpec.numChannels = 2;
	
	// limiter
	limiter.prepare(dspSpec);
	updateLimiter();
	
	// dry wet mixer
	dryWet.prepare(dspSpec);
	dryWet.setMixingRule(dsp::DryWetMixingRule::linear);
	dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
	dryWet.setWetLatency(2); // latency in samples of the ESOLA algorithm
};



void ImogenAudioProcessor::releaseResources() {
	
	wetBuffer.clear();
	
	harmonizer.resetNoteOnCounter();
};



void ImogenAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	
	const int inputChannel = inputChannelListener >= buffer.getNumChannels() ? buffer.getNumChannels() - 1 : int(inputChannelListener);
	
	int samplesLeft = buffer.getNumSamples();
	int startSample = buffer.getNumSamples() - samplesLeft;
	
	while (samplesLeft > 0)
	{
		const int numSamples = samplesLeft >= MAX_BUFFERSIZE ? MAX_BUFFERSIZE : samplesLeft;
		
		AudioBuffer<float> proxy (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), startSample, numSamples);
		
		processBlockPrivate(proxy, numSamples, inputChannel, midiMessages);
		
		// update midi buffer timestamps:
		// for all midiMessages from startSample to the end of the midiBuffer, subtract startSample from their timestamps
		{
			auto midiIterator = midiMessages.findNextSamplePosition(startSample);
			
			std::for_each (midiIterator,
						   midiMessages.cend(),
						   [&] (const MidiMessageMetadata& meta)
						   {
							   MidiMessage current = meta.getMessage();
							   current.setTimeStamp(current.getTimeStamp() - startSample);
						   });
		}
		
		samplesLeft -= numSamples;
		startSample += numSamples;
	}
};


void ImogenAudioProcessor::processBlockPrivate(AudioBuffer<float>& buffer, const int numSamples, const int inputChannel, MidiBuffer& inputMidi)
{
	// update settings & parameters
	{
		if(wetBuffer.getNumSamples() != numSamples) { wetBuffer.setSize(NUMBER_OF_CHANNELS, numSamples, true, true, true); }
		
		wetBuffer.clear();
		
		updateIOgains();
		updateLimiter();
		updateAdsr();
		updateStereoWidth();
		harmonizer.updateMidiVelocitySensitivity(midiVelocitySensListener);
		harmonizer.setNoteStealingEnabled(voiceStealingListener > 0.5f);
		harmonizer.updatePitchbendSettings(pitchBendUpListener, pitchBendDownListener);
		
		// dry vox pan
		{
			if(dryVoxPanListener != previousmidipan) {
				dryvoxpanningmults[1] = dryVoxPanListener / 127.0f;
				dryvoxpanningmults[0] = 1.0f - dryvoxpanningmults[1];
				previousmidipan = dryVoxPanListener;
			}
		}
		
		// MIDI pedal pitch
		{
			pedalPitchToggle = pedalPitchToggleListener > 0.5f;
			pedalPitchThresh = round(pedalPitchThreshListener);
		}
		
		// master dry/wet
		{
			dryWet.setWetMixProportion(masterDryWetListener / 100.0f);
		}
		
	}
	
	
	//==========================  AUDIO DSP SIGNAL CHAIN STARTS HERE ==========================//
	
	
	buffer.applyGain(inputChannel, 0, numSamples, inputGainMultiplier); // apply input gain
	
	dsp::AudioBlock<float> dwinblock (buffer);
	dryWet.pushDrySamples(dwinblock); // first, pan the dry signal!
	
	harmonizer.renderNextBlock(buffer, inputChannel, 0, numSamples, wetBuffer, inputMidi);
	
	// clear any extra channels present in I/O buffer
	{
		if (buffer.getNumChannels() > NUMBER_OF_CHANNELS)
			for (int i = NUMBER_OF_CHANNELS + 1; i <= buffer.getNumChannels(); ++i)
				buffer.clear(i - 1, 0, numSamples);
	}

	dsp::AudioBlock<float> dwoutblock (buffer);
	dryWet.mixWetSamples(dwoutblock);
	
	buffer.applyGain(0, numSamples, outputGainMultiplier); // apply master output gain
	
	// output limiter
	if(limiterIsOn) {
		dsp::AudioBlock<float> limiterBlock (buffer);
		limiter.process(dsp::ProcessContextReplacing<float>(limiterBlock));
	}
	
	//==========================  AUDIO DSP SIGNAL CHAIN ENDS HERE ==========================//
	
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
	
	if (previousStereoWidth != stereoWidthListener)
	{
		harmonizer.updateStereoWidth(stereoWidthListener);
		previousStereoWidth = stereoWidthListener;
	}
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
			tree.replaceState (juce::ValueTree::fromXml (*xmlState));
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
