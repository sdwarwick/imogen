#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessor::ImogenAudioProcessor():
    AudioProcessor(makeBusProperties()),
    tree(*this, nullptr, "PARAMETERS", createParameters()),
    floatEngine(*this), doubleEngine(*this),
    inputGainMultiplier(1.0f), outputGainMultiplier(1.0f),
    modulatorInput(ModulatorInputSource::left),
    limiterIsOn(true),
    prevDryPan(64), prevideb(0.0f), prevodeb(0.0f),
    choppingInput(true),
    wasBypassedLastCallback(true)
{
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    dryPan             = dynamic_cast<AudioParameterInt*>  (tree.getParameter("dryPan"));                   jassert(dryPan);
    dryWet             = dynamic_cast<AudioParameterInt*>  (tree.getParameter("masterDryWet"));             jassert(dryWet);
    inputChan          = dynamic_cast<AudioParameterInt*>  (tree.getParameter("inputChan"));                jassert(inputChan);
    adsrAttack         = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrAttack"));               jassert(adsrAttack);
    adsrDecay          = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrDecay"));                jassert(adsrDecay);
    adsrSustain        = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrSustain"));              jassert(adsrSustain);
    adsrRelease        = dynamic_cast<AudioParameterFloat*>(tree.getParameter("adsrRelease"));              jassert(adsrRelease);
    adsrToggle         = dynamic_cast<AudioParameterBool*> (tree.getParameter("adsrOnOff"));                jassert(adsrToggle);
    quickKillMs        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("quickKillMs"));              jassert(quickKillMs);
    quickAttackMs      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("quickAttackMs"));            jassert(quickAttackMs);
    stereoWidth        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("stereoWidth"));              jassert(stereoWidth);
    lowestPanned       = dynamic_cast<AudioParameterInt*>  (tree.getParameter("lowestPan"));                jassert(lowestPanned);
    velocitySens       = dynamic_cast<AudioParameterInt*>  (tree.getParameter("midiVelocitySensitivity"));  jassert(velocitySens);
    pitchBendUp        = dynamic_cast<AudioParameterInt*>  (tree.getParameter("PitchBendUpRange"));         jassert(pitchBendUp);
    pitchBendDown      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("PitchBendDownRange"));       jassert(pitchBendDown);
    pedalPitchIsOn     = dynamic_cast<AudioParameterBool*> (tree.getParameter("pedalPitchToggle"));         jassert(pedalPitchIsOn);
    pedalPitchThresh   = dynamic_cast<AudioParameterInt*>  (tree.getParameter("pedalPitchThresh"));         jassert(pedalPitchThresh);
    pedalPitchInterval = dynamic_cast<AudioParameterInt*>  (tree.getParameter("pedalPitchInterval"));       jassert(pedalPitchInterval);
    descantIsOn        = dynamic_cast<AudioParameterBool*> (tree.getParameter("descantToggle"));            jassert(descantIsOn);
    descantThresh      = dynamic_cast<AudioParameterInt*>  (tree.getParameter("descantThresh"));            jassert(descantThresh);
    descantInterval    = dynamic_cast<AudioParameterInt*>  (tree.getParameter("descantInterval"));          jassert(descantInterval);
    concertPitchHz     = dynamic_cast<AudioParameterInt*>  (tree.getParameter("concertPitch"));             jassert(concertPitchHz);
    voiceStealing      = dynamic_cast<AudioParameterBool*> (tree.getParameter("voiceStealing"));            jassert(voiceStealing);
    latchIsOn          = dynamic_cast<AudioParameterBool*> (tree.getParameter("latchIsOn"));                jassert(latchIsOn);
    inputGain          = dynamic_cast<AudioParameterFloat*>(tree.getParameter("inputGain"));                jassert(inputGain);
    outputGain         = dynamic_cast<AudioParameterFloat*>(tree.getParameter("outputGain"));               jassert(outputGain);
    limiterToggle      = dynamic_cast<AudioParameterBool*> (tree.getParameter("limiterIsOn"));              jassert(limiterToggle);
    limiterThresh      = dynamic_cast<AudioParameterFloat*>(tree.getParameter("limiterThresh"));            jassert(limiterThresh);
    limiterRelease     = dynamic_cast<AudioParameterInt*>  (tree.getParameter("limiterRelease"));           jassert(limiterRelease);
    
    const double initSamplerate   = std::max<double>(44100.0, getSampleRate());
    const int initSamplesPerBlock = std::max(MAX_BUFFERSIZE, getBlockSize());
    
    if (isUsingDoublePrecision())
    {
        doubleEngine.initialize(initSamplerate, initSamplesPerBlock, 12);
        updateAllParameters(doubleEngine);
    }
    else
    {
        floatEngine.initialize(initSamplerate, initSamplesPerBlock, 12);
        updateAllParameters(floatEngine);
    }
};

ImogenAudioProcessor::~ImogenAudioProcessor()
{ };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, samplesPerBlock, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, samplesPerBlock, floatEngine,  doubleEngine);
};


template <typename SampleType1, typename SampleType2>
void ImogenAudioProcessor::prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                                                 ImogenEngine<SampleType1>& activeEngine,
                                                 ImogenEngine<SampleType2>& idleEngine)
{
    if (! activeEngine.hasBeenInitialized())
        activeEngine.initialize (sampleRate, samplesPerBlock, 12);
    else
        activeEngine.prepare (sampleRate, samplesPerBlock);
    
    if (! idleEngine.hasBeenReleased())
        idleEngine.releaseResources();
    
    updateAllParameters(activeEngine);
};


void ImogenAudioProcessor::releaseResources()
{
    doubleEngine.releaseResources();
    floatEngine .releaseResources();
};


void ImogenAudioProcessor::reset()
{
    floatEngine .reset();
    doubleEngine.reset();
};


void ImogenAudioProcessor::killAllMidi()
{
    if (isUsingDoublePrecision())
        doubleEngine.reset();
    else
        floatEngine.reset();
};


// audio rendering ----------------------------------------------------------------------------------------------------------------------------------

template <typename SampleType>
void ImogenAudioProcessor::processBlockWrapped (AudioBuffer<SampleType>& buffer,
                                                MidiBuffer& midiMessages,
                                                ImogenEngine<SampleType>& engine)
{
    if ( (buffer.getNumSamples() == 0) || (buffer.getNumChannels() == 0) ) // some hosts are crazy
        return;
    
    if (! engine.hasBeenInitialized())
        return;
    
    if ( ( host.isLogic() || host.isGarageBand() ) && ( getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled() ) )
        return; // our audio input is disabled! can't do processing
    
    updateAllParameters(engine);
    
    AudioBuffer<SampleType> inBus  = AudioProcessor::getBusBuffer(buffer, true, (host.isLogic() || host.isGarageBand()));
    AudioBuffer<SampleType> outBus = AudioProcessor::getBusBuffer(buffer, false, 0); // out bus must be configured to stereo
    
    engine.process (inBus, outBus, midiMessages, wasBypassedLastCallback, false);
    
    wasBypassedLastCallback = false;
};


template <typename SampleType>
void ImogenAudioProcessor::processBlockBypassedWrapped (AudioBuffer<SampleType>& buffer,
                                                        MidiBuffer& midiMessages,
                                                        ImogenEngine<SampleType>& engine)
{
    if ( (buffer.getNumSamples() == 0) || (buffer.getNumChannels() == 0) ) // some hosts are crazy
        return;
    
    if (! engine.hasBeenInitialized())
        return;
    
    if ( ( host.isLogic() || host.isGarageBand() ) && ( getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled() ) )
        return; // our audio input is disabled! can't do processing
    
    updateAllParameters(engine);
    
    AudioBuffer<SampleType> inBus  = AudioProcessor::getBusBuffer(buffer, true, (host.isLogic() || host.isGarageBand()));
    AudioBuffer<SampleType> outBus = AudioProcessor::getBusBuffer(buffer, false, 0); // out bus must be configured to stereo
    
    if (! wasBypassedLastCallback)
        engine.process (inBus, outBus, midiMessages, false, true); // render 1 more output frame & ramp gain to 0
    else
        engine.processBypassed (inBus, outBus); // N.B. midi passes through unaffected when plugin is bypassed
    
    wasBypassedLastCallback = true;
};


/*===========================================================================================================================
 ============================================================================================================================*/

bool ImogenAudioProcessor::shouldWarnUserToEnableSidechain() const
{
    return getBusesLayout().getChannelSet(true, 1) == AudioChannelSet::disabled(); // only for Logic & Garageband
};


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void ImogenAudioProcessor::updateAllParameters (ImogenEngine<SampleType>& activeEngine)
{
    updateIOgains();
    updateDryVoxPan();
    
    limiterIsOn.store(limiterToggle->get());

    activeEngine.updateLimiter(limiterThresh->get(), limiterRelease->get());
    activeEngine.updateDryWet(dryWet->get());
    activeEngine.updateQuickKill(quickKillMs->get());
    activeEngine.updateQuickAttack(quickAttackMs->get());
    activeEngine.updateNoteStealing(voiceStealing->get());
    
    // these parameter functions have the potential to alter the pitch & other properties of currently playing harmonizer voices:
    activeEngine.updateConcertPitch(concertPitchHz->get());
    activeEngine.updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
    activeEngine.updateStereoWidth(stereoWidth->get(), lowestPanned->get());
    activeEngine.updateMidiVelocitySensitivity(velocitySens->get());
    activeEngine.updateAdsr(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    
    // these parameter functions have the potential to trigger or turn off midi notes / harmonizer voices:
    activeEngine.updateMidiLatch(latchIsOn->get());
    activeEngine.updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    activeEngine.updateDescant(descantIsOn->get(), descantThresh->get(), descantInterval->get());
    
    // update num voices
    
    // update chopping mode
};


void ImogenAudioProcessor::updateDryVoxPan()
{
    const int newDryPan = dryPan->get();
    
    if(newDryPan != prevDryPan)
    {
        const float Rpan = newDryPan / 127.0f;
        dryvoxpanningmults[1] = Rpan;
        dryvoxpanningmults[0] = 1.0f - Rpan;
        prevDryPan = newDryPan;
    }
};

void ImogenAudioProcessor::updateDryWet()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateDryWet(dryWet->get());
    else
        floatEngine.updateDryWet(dryWet->get());
};

void ImogenAudioProcessor::updateAdsr()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateAdsr(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    else
        floatEngine .updateAdsr(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
};

void ImogenAudioProcessor::updateQuickKillMs()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateQuickKill(quickKillMs->get());
    else
        floatEngine .updateQuickKill(quickKillMs->get());
};

void ImogenAudioProcessor::updateQuickAttackMs()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateQuickAttack(quickAttackMs->get());
    else
        floatEngine .updateQuickAttack(quickAttackMs->get());
};

void ImogenAudioProcessor::updateStereoWidth()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateStereoWidth(stereoWidth->get(), lowestPanned->get());
    else
        floatEngine .updateStereoWidth(stereoWidth->get(), lowestPanned->get());
};

void ImogenAudioProcessor::updateMidiVelocitySensitivity()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiVelocitySensitivity(velocitySens->get());
    else
        floatEngine .updateMidiVelocitySensitivity(velocitySens->get());
};

void ImogenAudioProcessor::updatePitchbendSettings()
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
    else
        floatEngine .updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
};

void ImogenAudioProcessor::updatePedalPitch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    else
        floatEngine .updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
};

void ImogenAudioProcessor::updateDescant()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateDescant(descantIsOn->get(), descantThresh->get(), descantInterval->get());
    else
        floatEngine .updateDescant(descantIsOn->get(), descantThresh->get(), descantInterval->get());
};

void ImogenAudioProcessor::updateConcertPitch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateConcertPitch(concertPitchHz->get());
    else
        floatEngine .updateConcertPitch(concertPitchHz->get());
};

void ImogenAudioProcessor::updateNoteStealing()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateNoteStealing(voiceStealing->get());
    else
        floatEngine .updateNoteStealing(voiceStealing->get());
};

void ImogenAudioProcessor::updateMidiLatch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiLatch(latchIsOn->get());
    else
        floatEngine .updateMidiLatch(latchIsOn->get());
};

void ImogenAudioProcessor::updateIOgains()
{
    const float newIn = inputGain->get();
    if(newIn != prevideb)
        inputGainMultiplier.store(newIn);
    prevideb = newIn;
    
    const float newOut = outputGain->get();
    if(newOut != prevodeb)
        outputGainMultiplier.store(newOut);
    prevodeb = newOut;
};

void ImogenAudioProcessor::updateLimiter()
{
    limiterIsOn.store(limiterToggle->get());
    
    if (isUsingDoublePrecision())
        doubleEngine.updateLimiter(limiterThresh->get(), limiterRelease->get());
    else
        floatEngine.updateLimiter(limiterThresh->get(), limiterRelease->get());
};

void ImogenAudioProcessor::updatePitchDetectionSettings(const float newMinHz, const float newMaxHz, const float newTolerance)
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePitchDetectionSettings(newMinHz, newMaxHz, newTolerance);
    else
        floatEngine .updatePitchDetectionSettings(newMinHz, newMaxHz, newTolerance);
};




void ImogenAudioProcessor::returnActivePitches(Array<int>& outputArray) const
{
    if (isUsingDoublePrecision())
        doubleEngine.returnActivePitches(outputArray);
    else
        floatEngine.returnActivePitches(outputArray);
};


void ImogenAudioProcessor::updateNumVoices(const int newNumVoices)
{
    if (isUsingDoublePrecision())
        doubleEngine.updateNumVoices(newNumVoices);
    else
        floatEngine.updateNumVoices(newNumVoices);
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// functions for custom preset management system ----------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::savePreset(juce::String presetName) 
{
    // this function can be used both to save new preset files or to update existing ones
    
    File writingTo = getPresetsFolder().getChildFile(presetName);
    
    auto xml(tree.copyState().createXml());
   // xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    xml->writeTo(writingTo);
    updateHostDisplay();
};


void ImogenAudioProcessor::loadPreset(juce::String presetName)
{
    File presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if(presetToLoad.existsAsFile())
    {
        auto xmlElement = juce::parseXML(presetToLoad);
        
        if(xmlElement.get() != nullptr && xmlElement->hasTagName (tree.state.getType()))
        {
            tree.replaceState(juce::ValueTree::fromXml (*xmlElement));
            updateNumVoices( xmlElement->getIntAttribute("numberOfVoices", 4) ); // TO DO : send notif to GUI to update numVoices comboBox
            updateHostDisplay();
        }
    }
};


void ImogenAudioProcessor::deletePreset(juce::String presetName) 
{
    File presetToDelete = getPresetsFolder().getChildFile(presetName);
    
    if(presetToDelete.existsAsFile())
        if(! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    updateHostDisplay();
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
 //   xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    copyXmlToBinary (*xml, destData);
};


void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr && xmlState->hasTagName (tree.state.getType()))
    {
        tree.replaceState(juce::ValueTree::fromXml (*xmlState));
        const int newNumOfVoices = xmlState->getIntAttribute("numberOfVoices", 4);
        updateNumVoices(newNumOfVoices); // TO DO : send notif to GUI to update numVoices comboBox
    }
};


// standard and general-purpose functions -----------------------------------------------------------------------------------------------------------

AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters() const
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    // general
    params.push_back(std::make_unique<AudioParameterInt>	("dryPan", "Dry vox pan", 0, 127, 64));
    
    params.push_back(std::make_unique<AudioParameterInt>	("masterDryWet", "% wet", 0, 100, 100));
    
    params.push_back(std::make_unique<AudioParameterInt>	("inputChan", "Input channel", 0, 16, 0));
    
    // ADSR
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrAttack", "ADSR Attack", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.035f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrDecay", "ADSR Decay", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.06f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrSustain", "ADSR Sustain", NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.8f));
    params.push_back(std::make_unique<AudioParameterFloat> 	("adsrRelease", "ADSR Release", NormalisableRange<float> (0.001f, 1.0f, 0.001f), 0.1f));
    params.push_back(std::make_unique<AudioParameterBool>	("adsrOnOff", "ADSR on/off", true));
    params.push_back(std::make_unique<AudioParameterInt>	("quickKillMs", "Quick kill ms", 1, 250, 15));
    params.push_back(std::make_unique<AudioParameterInt>	("quickAttackMs", "Quick attack ms", 1, 250, 15));
    
    // stereo width
    params.push_back(std::make_unique<AudioParameterInt> 	("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back(std::make_unique<AudioParameterInt>	("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    
    // midi settings
    params.push_back(std::make_unique<AudioParameterInt> 	("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back(std::make_unique<AudioParameterInt> 	("PitchBendUpRange", "Pitch bend range (up)", 0, 12, 2));
    params.push_back(std::make_unique<AudioParameterInt>	("PitchBendDownRange", "Pitch bend range (down)", 0, 12, 2));
    // pedal pitch
    params.push_back(std::make_unique<AudioParameterBool>	("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back(std::make_unique<AudioParameterInt>	("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back(std::make_unique<AudioParameterInt>	("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    // descant
    params.push_back(std::make_unique<AudioParameterBool>	("descantToggle", "Descant on/off", false));
    params.push_back(std::make_unique<AudioParameterInt>	("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back(std::make_unique<AudioParameterInt>	("descantInterval", "Descant interval", 1, 12, 12));
    // concert pitch Hz
    params.push_back(std::make_unique<AudioParameterInt>	("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    // voice stealing
    params.push_back(std::make_unique<AudioParameterBool>	("voiceStealing", "Voice stealing", false));
    // midi latch
    params.push_back(std::make_unique<AudioParameterBool>	("latchIsOn", "MIDI latch on/off", false));
    
    // input & output gain
    params.push_back(std::make_unique<AudioParameterFloat>	("inputGain", "Input Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<AudioParameterFloat>	("outputGain", "Output Gain", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -4.0f));
    
    // output limiter
    params.push_back(std::make_unique<AudioParameterBool>	("limiterIsOn", "Limiter on/off", true));
    params.push_back(std::make_unique<AudioParameterFloat>	("limiterThresh", "Limiter threshold", NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -2.0f));
    params.push_back(std::make_unique<AudioParameterInt>	("limiterRelease", "limiter release (ms)", 1, 250, 10));
    
    return { params.begin(), params.end() };
};


double ImogenAudioProcessor::getTailLengthSeconds() const
{
    if (adsrToggle->get())
        return double (adsrRelease->get()); // ADSR release time in seconds
    
    return double (quickKillMs->get() * 1000.0f); // "quick kill" time in seconds
};

int ImogenAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
};

int ImogenAudioProcessor::getCurrentProgram() {
    return 0;
};

void ImogenAudioProcessor::setCurrentProgram (int index)
{ };

const juce::String ImogenAudioProcessor::getProgramName (int index) {
    return {};
};

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{ };


AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
{
    if (host.isLogic() || host.isGarageBand())
        return BusesProperties().withInput ("Input",     AudioChannelSet::stereo(), true)
                                .withInput ("Sidechain", AudioChannelSet::mono(),   true)
                                .withOutput("Output",    AudioChannelSet::stereo(), true);
    
    return     BusesProperties().withInput ("Input",     AudioChannelSet::stereo(), true)
                                .withOutput("Output",    AudioChannelSet::stereo(), true);
};


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if ( (layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled()) && (! (host.isLogic() || host.isGarageBand())) )
        return false;
    
    if ( layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled() )
        return false;
    
    if ( layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
};


bool ImogenAudioProcessor::canAddBus(bool isInput) const
{
    if (! host.isLogic() || host.isGarageBand())
        return false;
    
    return isInput;
};


void ImogenAudioProcessor::updateTrackProperties (const TrackProperties& properties)
{
    String trackName   = properties.name;   // The name   of the track - this will be empty if the track name is not known
    Colour trackColour = properties.colour; // The colour of the track - this will be transparentBlack if the colour is not known
    
    if (trackName != "")
    {
        // do something cool with the name of the mixer track the plugin is loaded on
    }
    
    if (trackColour != Colours::transparentBlack)
    {
        // do something cool with the colour of the mixer track the plugin is loaded on
    }
};


juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
    return new ImogenAudioProcessorEditor(*this);
};


// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
};
