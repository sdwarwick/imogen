
#include "PluginProcessor.h"


juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters() const
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    
    // general
    params.push_back(std::make_unique<juce::AudioParameterBool> ("mainBypass", "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("dryPan", "Dry vox pan", 0, 127, 64));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("masterDryWet", "% wet", 0, 100, 100));
    
    // ADSR
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrAttack", "ADSR Attack", msRange, 0.035f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrDecay", "ADSR Decay", msRange, 0.06f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrSustain", "ADSR Sustain",
                                                                 juce::NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrRelease", "ADSR Release", msRange, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("adsrOnOff", "ADSR on/off", true));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("quickKillMs", "Quick kill ms", 1, 250, 15));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("quickAttackMs", "Quick attack ms", 1, 250, 15));
    
    // stereo width
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    
    // midi settings
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("PitchBendUpRange", "Pitch bend range (up)", 0, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("PitchBendDownRange", "Pitch bend range (down)", 0, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("voiceStealing", "Voice stealing", false));
    // pedal pitch
    params.push_back(std::make_unique<juce::AudioParameterBool> ("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    // descant
    params.push_back(std::make_unique<juce::AudioParameterBool> ("descantToggle", "Descant on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("descantInterval", "Descant interval", 1, 12, 12));
    
    // input & output gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>("inputGain", "Input gain",   gainRange, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("outputGain", "Output gain", gainRange, -4.0f));
    
    // output limiter
    params.push_back(std::make_unique<juce::AudioParameterBool> ("limiterIsOn", "Limiter on/off", true));
    
    // NEED GUI FOR THIS -- soft pedal gain multiplier
    params.push_back(std::make_unique<juce::AudioParameterFloat>("softPedalGain", "Soft pedal gain", gainRange, 0.0f));
    
    // aftertouch gain toggle on/off
    params.push_back(std::make_unique<juce::AudioParameterBool> ("aftertouchGainToggle", "Aftertouch gain on/off", true));
    // use channel pressure on/off
    params.push_back(std::make_unique<juce::AudioParameterBool> ("channelPressureToggle", "Use channel pressure", false));
    
    // playing but released gain multiplier
    params.push_back(std::make_unique<juce::AudioParameterFloat>("playingButReleasedGain", "Released & ringing gain", gainRange, 0.5f));
    
    // NEED GUI -- PITCH DETECTION SETTINGS
    // Note that the minimum possible Hz value will impact the plugin's latency.
    juce::NormalisableRange<float> confidenceRange (0.0f, 1.0f, 0.01f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitchDetectionConfidenceUpperThresh", "Confidence upper thresh",
                                                                 confidenceRange, 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitchDetectionConfidenceLowerThresh", "Confidence lower thresh",
                                                                 confidenceRange, 0.05f));
    
    return { params.begin(), params.end() };
}


void ImogenAudioProcessor::initializeParameterPointers()
{
    isBypassed         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("mainBypass"));                 jassert (isBypassed);
    dryPan             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("dryPan"));                     jassert(dryPan);
    dryWet             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("masterDryWet"));               jassert(dryWet);
    adsrAttack         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrAttack"));                 jassert(adsrAttack);
    adsrDecay          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrDecay"));                  jassert(adsrDecay);
    adsrSustain        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrSustain"));                jassert(adsrSustain);
    adsrRelease        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrRelease"));                jassert(adsrRelease);
    adsrToggle         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("adsrOnOff"));                  jassert(adsrToggle);
    quickKillMs        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("quickKillMs"));                jassert(quickKillMs);
    quickAttackMs      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("quickAttackMs"));              jassert(quickAttackMs);
    stereoWidth        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("stereoWidth"));                jassert(stereoWidth);
    lowestPanned       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("lowestPan"));                  jassert(lowestPanned);
    velocitySens       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("midiVelocitySensitivity"));    jassert(velocitySens);
    pitchBendUp        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("PitchBendUpRange"));           jassert(pitchBendUp);
    pitchBendDown      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("PitchBendDownRange"));         jassert(pitchBendDown);
    pedalPitchIsOn     = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("pedalPitchToggle"));           jassert(pedalPitchIsOn);
    pedalPitchThresh   = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchThresh"));           jassert(pedalPitchThresh);
    pedalPitchInterval = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchInterval"));         jassert(pedalPitchInterval);
    descantIsOn        = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("descantToggle"));              jassert(descantIsOn);
    descantThresh      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantThresh"));              jassert(descantThresh);
    descantInterval    = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantInterval"));            jassert(descantInterval);
    concertPitchHz     = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("concertPitch"));               jassert(concertPitchHz);
    voiceStealing      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("voiceStealing"));              jassert(voiceStealing);
    inputGain          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("inputGain"));                  jassert(inputGain);
    outputGain         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("outputGain"));                 jassert(outputGain);
    limiterToggle      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("limiterIsOn"));                jassert(limiterToggle);
    softPedalGain      = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("softPedalGain"));              jassert(softPedalGain);
    pitchDetectionConfidenceUpperThresh = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("pitchDetectionConfidenceUpperThresh")); jassert(pitchDetectionConfidenceUpperThresh);
    pitchDetectionConfidenceLowerThresh = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("pitchDetectionConfidenceLowerThresh")); jassert(pitchDetectionConfidenceLowerThresh);
    aftertouchGainToggle   = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("aftertouchGainToggle"));   jassert(aftertouchGainToggle);
    channelPressureToggle  = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("channelPressureToggle"));  jassert(channelPressureToggle);
    playingButReleasedGain = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("playingButReleasedGain")); jassert(playingButReleasedGain);
}


void ImogenAudioProcessor::updateParameterDefaults()
{
    defaultDryPan.store (dryPan->get());
    defaultDryWet.store (dryWet->get());
    defaultQuickKillMs.store (quickKillMs->get());
    defaultQuickAttackMs.store (quickAttackMs->get());
    defaultStereoWidth.store (stereoWidth->get());
    defaultLowestPannedNote.store (lowestPanned->get());
    defaultVelocitySensitivity.store (velocitySens->get());
    defaultPitchbendUp.store (pitchBendUp->get());
    defaultPitchbendDown.store (pitchBendDown->get());
    defaultPedalPitchThresh.store (pedalPitchThresh->get());
    defaultPedalPitchInterval.store (pedalPitchInterval->get());
    defaultDescantThresh.store (descantThresh->get());
    defaultDescantInterval.store (descantInterval->get());
    defaultConcertPitchHz.store (concertPitchHz->get());
    defaultAdsrAttack.store (adsrAttack->get());
    defaultAdsrDecay.store (adsrDecay->get());
    defaultAdsrSustain.store (adsrSustain->get());
    defaultAdsrRelease.store (adsrRelease->get());
    defaultInputGain.store (inputGain->get());
    defaultOutputGain.store (outputGain->get());
    defaultSoftPedalGain.store (softPedalGain->get());
    defaultPitchUpperConfidenceThresh.store (pitchDetectionConfidenceUpperThresh->get());
    defaultPitchLowerConfidenceThresh.store (pitchDetectionConfidenceLowerThresh->get());
    defaultPlayingButReleasedGain.store (playingButReleasedGain->get());
    
    parameterDefaultsAreDirty.store (true);
}


juce::AudioProcessorParameter* ImogenAudioProcessor::getBypassParameter() const
{
    return tree.getParameter ("mainBypass");
}



// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine)
{
    activeEngine.updatePitchDetectionConfidenceThresh (pitchDetectionConfidenceUpperThresh->get(),
                                                       pitchDetectionConfidenceLowerThresh->get());
    
    activeEngine.updateInputGain    (juce::Decibels::decibelsToGain (inputGain->get()));
    activeEngine.updateOutputGain   (juce::Decibels::decibelsToGain (outputGain->get()));
    activeEngine.updateSoftPedalGain(juce::Decibels::decibelsToGain (softPedalGain->get()));
    activeEngine.updateDryVoxPan (dryPan->get());
    activeEngine.updateDryWet (dryWet->get());
    activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    activeEngine.updateQuickKill (quickKillMs->get());
    activeEngine.updateQuickAttack (quickAttackMs->get());
    activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
    activeEngine.updateMidiVelocitySensitivity (velocitySens->get());
    activeEngine.updatePitchbendSettings (pitchBendUp->get(), pitchBendDown->get());
    activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), descantInterval->get());
    activeEngine.updateConcertPitch (concertPitchHz->get());
    activeEngine.updateNoteStealing (voiceStealing->get());
    activeEngine.updateLimiter (limiterToggle->get());
    activeEngine.updateAftertouchGainOnOff (aftertouchGainToggle->get());
    activeEngine.updateUsingChannelPressure (channelPressureToggle->get());
    activeEngine.updatePlayingButReleasedGain (playingButReleasedGain->get());
}


void ImogenAudioProcessor::updatePitchDetectionHzRange (int minHz, int maxHz)
{
    suspendProcessing (true);
    
    if (isUsingDoublePrecision())
    {
        doubleEngine.updatePitchDetectionHzRange (minHz, maxHz);
        
        if (getLatencySamples() != doubleEngine.reportLatency())
            setLatencySamples (doubleEngine.reportLatency());
    }
    else
    {
        floatEngine.updatePitchDetectionHzRange (minHz, maxHz);
        
        if (getLatencySamples() != floatEngine.reportLatency())
            setLatencySamples (floatEngine.reportLatency());
    }
    
    suspendProcessing (false);
}


void ImogenAudioProcessor::updateNumVoices (const int newNumVoices)
{
    if (isUsingDoublePrecision())
    {
        if (doubleEngine.getCurrentNumVoices() == newNumVoices)
            return;
        
        suspendProcessing (true);
        doubleEngine.updateNumVoices (newNumVoices);
    }
    else
    {
        if (floatEngine.getCurrentNumVoices() == newNumVoices)
            return;
        
        suspendProcessing (true);
        floatEngine.updateNumVoices (newNumVoices);
    }
    
    suspendProcessing (false);
}


void ImogenAudioProcessor::updateModulatorInputSource (const int newSource)
{
    if (isUsingDoublePrecision())
        doubleEngine.setModulatorSource (newSource);
    else
        floatEngine.setModulatorSource (newSource);
}


// functions for preset & state management system ---------------------------------------------------------------------------------------------------

inline juce::File ImogenAudioProcessor::getPresetsFolder() const
{
    juce::File rootFolder;
    
#if JUCE_MAC
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory)
                    .getChildFile ("Audio")
                    .getChildFile ("Presets")
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");
#elif JUCE_LINUX
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory)
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");
#elif JUCE_WINDOWS
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDocumentsDirectory)
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");
#else
  #error Unsupported operating system!
#endif
    
    if (! rootFolder.isDirectory())
        rootFolder.createDirectory(); // creates the presets folder if it doesn't already exist
    
    return rootFolder;
}


void ImogenAudioProcessor::deletePreset (juce::String presetName)
{
    juce::File presetToDelete = getPresetsFolder().getChildFile(presetName);
    
    if (presetToDelete.existsAsFile())
        if (! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    
    updateHostDisplay();
}


// functions for saving state info.....................................

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml;
    
    if (isUsingDoublePrecision())
        xml = pluginStateToXml(doubleEngine);
    else
        xml = pluginStateToXml(floatEngine);
    
    copyXmlToBinary (*xml, destData);
}


void ImogenAudioProcessor::savePreset (juce::String presetName)
{
    // this function can be used both to save new preset files or to update existing ones
    
    std::unique_ptr<juce::XmlElement> xml;
    
    if (isUsingDoublePrecision())
        xml = pluginStateToXml(doubleEngine);
    else
        xml = pluginStateToXml(floatEngine);
    
    xml->setAttribute ("presetName", presetName);
    
    presetName += ".xml";
    
    xml->writeTo (getPresetsFolder().getChildFile(presetName));
    updateHostDisplay();
}


template<typename SampleType>
inline std::unique_ptr<juce::XmlElement> ImogenAudioProcessor::pluginStateToXml (bav::ImogenEngine<SampleType>& activeEngine)
{
    std::unique_ptr<juce::XmlElement> xml = tree.copyState().createXml();
    
    const int numVoices = activeEngine.getCurrentNumVoices();
    const int inputSource = activeEngine.getModulatorSource();
    
    xml->setAttribute ("numberOfVoices", numVoices);
    xml->setAttribute ("modulatorInputSource", inputSource);
    
    return xml;
}


// functions for loading state info.................................

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlElement (getXmlFromBinary (data, sizeInBytes));
    
    if (isUsingDoublePrecision())
        updatePluginInternalState (*xmlElement, doubleEngine);
    else
        updatePluginInternalState (*xmlElement, floatEngine);
}


bool ImogenAudioProcessor::loadPreset (juce::String presetName)
{
    presetName += ".xml";
    
    juce::File presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if (! presetToLoad.existsAsFile())
        return false;
    
    auto xmlElement = juce::parseXML (presetToLoad);
    
    if (isUsingDoublePrecision())
        return updatePluginInternalState (*xmlElement, doubleEngine);
    
    return updatePluginInternalState (*xmlElement, floatEngine);
}


template<typename SampleType>
inline bool ImogenAudioProcessor::updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine)
{
    if (! newState.hasTagName (tree.state.getType()))
        return false;
    
    suspendProcessing (true);
    
    tree.replaceState (juce::ValueTree::fromXml (newState));
    
    updateNumVoices (newState.getIntAttribute ("numberOfVoices", 4));
    activeEngine.setModulatorSource (newState.getIntAttribute ("modulatorInputSource", 0));
    
    updateAllParameters (activeEngine);
    
    updateParameterDefaults();
    
    suspendProcessing (false);
    
    updateHostDisplay();
    return true;  // TODO: how to check if replacing tree state was successful...?
}
