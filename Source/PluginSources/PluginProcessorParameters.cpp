
#include "PluginProcessor.h"


juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
    
    // main bypass
    params.push_back(std::make_unique<juce::AudioParameterBool> ("mainBypass", "Bypass", false));
    // lead bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>("leadBypass", "Lead bypass", false));
    // harmony bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>("harmonyBypass", "Harmony bypass", false));
    
    // lead vox pan
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("dryPan", "Dry vox pan", 0, 127, 64));
    
    // ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrAttack", "ADSR Attack", msRange, 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrDecay", "ADSR Decay", msRange, 0.06f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrSustain", "ADSR Sustain", zeroToOneRange, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrRelease", "ADSR Release", msRange, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("adsrOnOff", "ADSR on/off", true));
    
    // stereo width
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    
    // midi settings
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("PitchBendRange", "Pitch bend range (st)", 0, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("voiceStealing", "Voice stealing", false));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("aftertouchGainToggle", "Aftertouch gain on/off", true));
    // midi pedal pitch
    params.push_back(std::make_unique<juce::AudioParameterBool> ("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    // midi descant
    params.push_back(std::make_unique<juce::AudioParameterBool> ("descantToggle", "Descant on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("descantInterval", "Descant interval", 1, 12, 12));
    
    // mixer settings
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("masterDryWet", "% wet", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("inputGain", "Input gain",   gainRange, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("outputGain", "Output gain", gainRange, -4.0f));
    
    // limiter toggle
    params.push_back(std::make_unique<juce::AudioParameterBool> ("limiterIsOn", "Limiter on/off", true));
    // noise gate
    params.push_back(std::make_unique<juce::AudioParameterBool> ("noiseGateIsOn", "Noise gate toggle", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("noiseGateThresh", "Noise gate threshold", gainRange, -20.0f));
    // de-esser
    params.push_back(std::make_unique<juce::AudioParameterBool> ("deEsserIsOn", "De-esser toggle", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("deEsserThresh", "De-esser thresh", gainRange, -6.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("deEsserAmount", "De-esser amount", zeroToOneRange, 0.5f));
    // compressor
    params.push_back(std::make_unique<juce::AudioParameterBool> ("compressorToggle", "Compressor on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("compressorAmount", "Compressor amount", zeroToOneRange, 0.35f));
    // reverb
    params.push_back(std::make_unique<juce::AudioParameterBool> ("reverbIsOn", "Reverb toggle", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("reverbDryWet", "Reverb dry/wet", 0, 100, 35));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbDecay", "Reverb decay", zeroToOneRange, 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbDuck", "Duck amount", zeroToOneRange, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbLoCut", "Reverb low cut", hzRange, 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverbHiCut", "Reverb high cut", hzRange, 5500.0f));
    
    // pitch detection vocal range
#define imogen_DEFAULT_VOCAL_RANGE_TYPE 0
    params.push_back(std::make_unique<juce::AudioParameterChoice>("vocalRangeType", "Input vocal range",
                                                                  imgn_VOCAL_RANGE_TYPES,
                                                                  imogen_DEFAULT_VOCAL_RANGE_TYPE, "Input vocal range",
                                                                  [this] (int index, int maximumStringLength)
                                                                         {
                                                                             juce::String name = vocalRangeTypes[index];
                                                                             if (name.length() > maximumStringLength)
                                                                                 name = name.substring (0, maximumStringLength);
                                                                             return name;
                                                                         },
                                                                  [this] (const juce::String& text)
                                                                         {
                                                                             for (int i = 0; i < 4; ++i)
                                                                                 if (text.equalsIgnoreCase(vocalRangeTypes[i]))
                                                                                     return i;
                                                                             return imogen_DEFAULT_VOCAL_RANGE_TYPE;
                                                                         } ));
    return { params.begin(), params.end() };
}

#undef imogen_DEFAULT_VOCAL_RANGE_TYPE
#undef imgn_VOCAL_RANGE_TYPES


void ImogenAudioProcessor::initializeParameterPointers()
{
    mainBypass         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("mainBypass"));                 jassert (mainBypass);
    leadBypass         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("leadBypass"));                 jassert (leadBypass);
    harmonyBypass      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("harmonyBypass"));              jassert (harmonyBypass);
    dryPan             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("dryPan"));                     jassert (dryPan);
    dryWet             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("masterDryWet"));               jassert (dryWet);
    adsrAttack         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrAttack"));                 jassert (adsrAttack);
    adsrDecay          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrDecay"));                  jassert (adsrDecay);
    adsrSustain        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrSustain"));                jassert (adsrSustain);
    adsrRelease        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrRelease"));                jassert (adsrRelease);
    adsrToggle         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("adsrOnOff"));                  jassert (adsrToggle);
    stereoWidth        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("stereoWidth"));                jassert (stereoWidth);
    lowestPanned       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("lowestPan"));                  jassert (lowestPanned);
    velocitySens       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("midiVelocitySensitivity"));    jassert (velocitySens);
    pitchBendRange     = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("PitchBendRange"));             jassert (pitchBendRange);
    pedalPitchIsOn     = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("pedalPitchToggle"));           jassert (pedalPitchIsOn);
    pedalPitchThresh   = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchThresh"));           jassert (pedalPitchThresh);
    pedalPitchInterval = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchInterval"));         jassert (pedalPitchInterval);
    descantIsOn        = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("descantToggle"));              jassert (descantIsOn);
    descantThresh      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantThresh"));              jassert (descantThresh);
    descantInterval    = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantInterval"));            jassert (descantInterval);
    concertPitchHz     = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("concertPitch"));               jassert (concertPitchHz);
    voiceStealing      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("voiceStealing"));              jassert (voiceStealing);
    inputGain          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("inputGain"));                  jassert (inputGain);
    outputGain         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("outputGain"));                 jassert (outputGain);
    limiterToggle      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("limiterIsOn"));                jassert (limiterToggle);
    noiseGateThreshold = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("noiseGateThresh"));            jassert (noiseGateThreshold);
    noiseGateToggle    = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("noiseGateIsOn"));              jassert (noiseGateToggle);
    compressorToggle   = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("compressorToggle"));           jassert (compressorToggle);
    compressorAmount   = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("compressorAmount"));           jassert (compressorAmount);
    vocalRangeType     = dynamic_cast<juce::AudioParameterChoice*>(tree.getParameter("vocalRangeType"));            jassert (vocalRangeType);
    aftertouchGainToggle = dynamic_cast<juce::AudioParameterBool*>(tree.getParameter("aftertouchGainToggle"));      jassert (aftertouchGainToggle);
    deEsserToggle      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("deEsserIsOn"));                jassert (deEsserToggle);
    deEsserThresh      = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("deEsserThresh"));              jassert (deEsserThresh);
    deEsserAmount      = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("deEsserAmount"));              jassert (deEsserAmount);
    reverbToggle       = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("reverbIsOn"));                 jassert (reverbToggle);
    reverbDryWet       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("reverbDryWet"));               jassert (reverbDryWet);
    reverbDecay        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("reverbDecay"));                jassert (reverbDecay);
    reverbDuck         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("reverbDuck"));                 jassert (reverbDuck);
    reverbLoCut        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("reverbLoCut"));                jassert (reverbLoCut);
    reverbHiCut        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("reverbHiCut"));                jassert (reverbHiCut);
}


void ImogenAudioProcessor::initializeParameterListeners()
{
    addParameterMessenger ("mainBypass", mainBypassID);
    addParameterMessenger ("leadBypass", leadBypassID);
    addParameterMessenger ("harmonyBypass", harmonyBypassID);
    addParameterMessenger ("dryPan", dryPanID);
    addParameterMessenger ("masterDryWet", dryWetID);
    addParameterMessenger ("adsrAttack", adsrAttackID);
    addParameterMessenger ("adsrDecay", adsrDecayID);
    addParameterMessenger ("adsrSustain", adsrSustainID);
    addParameterMessenger ("adsrRelease", adsrReleaseID);
    addParameterMessenger ("adsrOnOff", adsrToggleID);
    addParameterMessenger ("stereoWidth", stereoWidthID);
    addParameterMessenger ("lowestPan", lowestPannedID);
    addParameterMessenger ("midiVelocitySensitivity", velocitySensID);
    addParameterMessenger ("PitchBendRange", pitchBendRangeID);
    addParameterMessenger ("pedalPitchToggle", pedalPitchIsOnID);
    addParameterMessenger ("pedalPitchThresh", pedalPitchThreshID);
    addParameterMessenger ("pedalPitchInterval", pedalPitchIntervalID);
    addParameterMessenger ("descantToggle", descantIsOnID);
    addParameterMessenger ("descantThresh", descantThreshID);
    addParameterMessenger ("descantInterval", descantIntervalID);
    addParameterMessenger ("concertPitch", concertPitchHzID);
    addParameterMessenger ("voiceStealing", voiceStealingID);
    addParameterMessenger ("inputGain", inputGainID);
    addParameterMessenger ("outputGain", outputGainID);
    addParameterMessenger ("limiterIsOn", limiterToggleID);
    addParameterMessenger ("noiseGateIsOn", noiseGateToggleID);
    addParameterMessenger ("noiseGateThresh", noiseGateThresholdID);
    addParameterMessenger ("compressorToggle", compressorToggleID);
    addParameterMessenger ("compressorAmount", compressorAmountID);
    addParameterMessenger ("vocalRangeType", vocalRangeTypeID);
    addParameterMessenger ("aftertouchGainToggle", aftertouchGainToggleID);
    addParameterMessenger ("deEsserIsOn", deEsserToggleID);
    addParameterMessenger ("deEsserThresh", deEsserThreshID);
    addParameterMessenger ("deEsserAmount", deEsserAmountID);
    addParameterMessenger ("reverbIsOn", reverbToggleID);
    addParameterMessenger ("reverbDryWet", reverbDryWetID);
    addParameterMessenger ("reverbDecay", reverbDecayID);
    addParameterMessenger ("reverbDuck", reverbDuckID);
    addParameterMessenger ("reverbLoCut", reverbLoCutID);
    addParameterMessenger ("reverbHiCut", reverbHiCutID);
}

void ImogenAudioProcessor::addParameterMessenger (juce::String stringID, int paramID)
{
    auto& messenger { parameterMessengers.emplace_back (paramChangesForProcessor, paramChangesForEditor, paramID) };
    tree.addParameterListener (stringID, &messenger);
}


void ImogenAudioProcessor::updateParameterDefaults()
{
    defaultLeadBypass.store (leadBypass->get());
    defaultHarmonyBypass.store (harmonyBypass->get());
    defaultDryPan.store (dryPan->get());
    defaultDryWet.store (dryWet->get());
    defaultStereoWidth.store (stereoWidth->get());
    defaultLowestPannedNote.store (lowestPanned->get());
    defaultVelocitySensitivity.store (velocitySens->get());
    defaultPitchbendRange.store (pitchBendRange->get());
    defaultPedalPitchToggle.store (pedalPitchIsOn->get());
    defaultPedalPitchThresh.store (pedalPitchThresh->get());
    defaultPedalPitchInterval.store (pedalPitchInterval->get());
    defaultDescantToggle.store (descantIsOn->get());
    defaultDescantThresh.store (descantThresh->get());
    defaultDescantInterval.store (descantInterval->get());
    defaultConcertPitchHz.store (concertPitchHz->get());
    defaultAdsrToggle.store (adsrToggle->get());
    defaultAdsrAttack.store (adsrAttack->get());
    defaultAdsrDecay.store (adsrDecay->get());
    defaultAdsrSustain.store (adsrSustain->get());
    defaultAdsrRelease.store (adsrRelease->get());
    defaultInputGain.store (inputGain->get());
    defaultOutputGain.store (outputGain->get());
    defaultNoiseGateThresh.store (noiseGateThreshold->get());
    defaultNoiseGateToggle.store (noiseGateToggle->get());
    defaultCompressorAmount.store (compressorAmount->get());
    defaultCompressorToggle.store (compressorToggle->get());
    defaultDeEsserThresh.store (deEsserThresh->get());
    defaultDeEsserAmount.store (deEsserAmount->get());
    defaultDeEsserToggle.store (deEsserToggle->get());
    defaultReverbToggle.store (reverbToggle->get());
    defaultReverbDryWet.store (reverbDryWet->get());
    defaultReverbDecay.store (reverbDecay->get());
    defaultReverbDuck.store (reverbDuck->get());
    defaultReverbLoCut.store (reverbLoCut->get());
    defaultReverbHiCut.store (reverbHiCut->get());
    defaultLimiterToggle.store (limiterToggle->get());
    defaultVoiceStealingToggle.store (voiceStealing->get());
    defaultAftertouchGainToggle.store (aftertouchGainToggle->get());
    defaultVocalRangeIndex.store (vocalRangeType->getIndex());
    
    if (isUsingDoublePrecision())
    {
        defaultModulatorSource.store (doubleEngine.getModulatorSource());
        defaultNumVoices.store (doubleEngine.getCurrentNumVoices());
    }
    else
    {
        defaultModulatorSource.store (floatEngine.getModulatorSource());
        defaultNumVoices.store (floatEngine.getCurrentNumVoices());
    }
    
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
    updateVocalRangeType (vocalRangeType->getCurrentChoiceName());
    
    activeEngine.updateBypassStates (leadBypass->get(), harmonyBypass->get());
    
    activeEngine.updateInputGain (juce::Decibels::decibelsToGain (inputGain->get()));
    activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (outputGain->get()));
    activeEngine.updateDryVoxPan (dryPan->get());
    activeEngine.updateDryWet (dryWet->get());
    activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
    activeEngine.updateMidiVelocitySensitivity (velocitySens->get());
    activeEngine.updatePitchbendRange (pitchBendRange->get());
    activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), descantInterval->get());
    activeEngine.updateConcertPitch (concertPitchHz->get());
    activeEngine.updateNoteStealing (voiceStealing->get());
    activeEngine.updateLimiter (limiterToggle->get());
    activeEngine.updateNoiseGate (noiseGateThreshold->get(), noiseGateToggle->get());
    activeEngine.updateAftertouchGainOnOff (aftertouchGainToggle->get());
    activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), deEsserToggle->get());
    
    updateCompressor (activeEngine, compressorToggle->get(), compressorAmount->get());
    
    activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                               reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
}


template<typename SampleType>
void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine)
{
    currentMessages.clearQuick();
    
    // retrieve all the messages available
    while (! paramChangesForProcessor.isEmpty())
        currentMessages.add (paramChangesForProcessor.popMessage());
    
    // we're going to process only the most recent message of each type
    bav::MessageQueue::flushRepeatedMessages (currentMessages);
    
    for (const auto msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        jassert (msg.value() >= 0.0f && msg.value() <= 1.0f);
        
#define _BOOL_MSG_ msg.value() >= 0.5f   // converts a message's float value to a boolean true/false
        switch (msg.type())
        {
            case (leadBypassID):
                activeEngine.updateBypassStates (_BOOL_MSG_, harmonyBypass->get());
            case (harmonyBypassID):
                activeEngine.updateBypassStates (leadBypass->get(), _BOOL_MSG_);
            case (dryPanID):
                activeEngine.updateDryVoxPan (juce::roundToInt (dryPan->getNormalisableRange().convertFrom0to1(msg.value())));
            case (dryWetID):
                activeEngine.updateDryWet (juce::roundToInt (dryWet->getNormalisableRange().convertFrom0to1(msg.value())));
            case (adsrAttackID):
                activeEngine.updateAdsr (adsrAttack->getNormalisableRange().convertFrom0to1(msg.value()),
                                         adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrDecayID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->getNormalisableRange().convertFrom0to1(msg.value()),
                                         adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrSustainID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->getNormalisableRange().convertFrom0to1(msg.value()),
                                         adsrRelease->get(), adsrToggle->get());
            case (adsrReleaseID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(),
                                         adsrRelease->getNormalisableRange().convertFrom0to1(msg.value()), adsrToggle->get());
            case (adsrToggleID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), _BOOL_MSG_);
            case (stereoWidthID):
                activeEngine.updateStereoWidth (juce::roundToInt (stereoWidth->getNormalisableRange().convertFrom0to1(msg.value())),
                                                lowestPanned->get());
            case (lowestPannedID):
                activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
            case (velocitySensID):
                activeEngine.updateMidiVelocitySensitivity (juce::roundToInt (velocitySens->getNormalisableRange().convertFrom0to1(msg.value())));
            case (pitchBendRangeID):
                activeEngine.updatePitchbendRange (juce::roundToInt (pitchBendRange->getNormalisableRange().convertFrom0to1(msg.value())));
            case (pedalPitchIsOnID):
                activeEngine.updatePedalPitch (_BOOL_MSG_, pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchThreshID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(),
                                               juce::roundToInt (pedalPitchThresh->getNormalisableRange().convertFrom0to1(msg.value())),
                                               pedalPitchInterval->get());
            case (pedalPitchIntervalID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(),
                                               juce::roundToInt (pedalPitchInterval->getNormalisableRange().convertFrom0to1(msg.value())));
            case (descantIsOnID):
                activeEngine.updateDescant (_BOOL_MSG_, descantThresh->get(), descantInterval->get());
            case (descantThreshID):
                activeEngine.updateDescant (descantIsOn->get(), juce::roundToInt (descantThresh->getNormalisableRange().convertFrom0to1(msg.value())),
                                            descantInterval->get());
            case (descantIntervalID):
                activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(),
                                            juce::roundToInt (descantInterval->getNormalisableRange().convertFrom0to1(msg.value())));
            case (concertPitchHzID):
                activeEngine.updateConcertPitch (juce::roundToInt (concertPitchHz->getNormalisableRange().convertFrom0to1(msg.value())));
            case (voiceStealingID):
                activeEngine.updateNoteStealing (_BOOL_MSG_);
            case (inputGainID):
                activeEngine.updateInputGain (juce::Decibels::decibelsToGain (inputGain->getNormalisableRange().convertFrom0to1(msg.value())));
            case (outputGainID):
                activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (outputGain->getNormalisableRange().convertFrom0to1(msg.value())));
            case (limiterToggleID):
                activeEngine.updateLimiter (_BOOL_MSG_);
            case (noiseGateToggleID):
                activeEngine.updateNoiseGate (noiseGateThreshold->get(), _BOOL_MSG_);
            case (noiseGateThresholdID):
                activeEngine.updateNoiseGate (noiseGateThreshold->getNormalisableRange().convertFrom0to1(msg.value()),
                                              noiseGateToggle->get());
            case (compressorToggleID):
                updateCompressor (activeEngine, _BOOL_MSG_, compressorAmount->get());
            case (compressorAmountID):
                updateCompressor (activeEngine, compressorToggle->get(),
                                  compressorAmount->getNormalisableRange().convertFrom0to1(msg.value()));
            case (vocalRangeTypeID):
                updateVocalRangeType (floatParamToVocalRangeType (msg.value()));
            case (aftertouchGainToggleID):
                activeEngine.updateAftertouchGainOnOff (_BOOL_MSG_);
            case (deEsserToggleID):
                activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), _BOOL_MSG_);
            case (deEsserThreshID):
                activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->getNormalisableRange().convertFrom0to1(msg.value()),
                                            deEsserToggle->get());
            case (deEsserAmountID):
                activeEngine.updateDeEsser (deEsserAmount->getNormalisableRange().convertFrom0to1(msg.value()),
                                            deEsserThresh->get(), deEsserToggle->get());
            case (reverbToggleID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), _BOOL_MSG_);
            case (reverbDryWetID):
                activeEngine.updateReverb (juce::roundToInt (reverbDryWet->getNormalisableRange().convertFrom0to1(msg.value())),
                                           reverbDecay->get(), reverbDuck->get(), reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDecayID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->getNormalisableRange().convertFrom0to1(msg.value()),
                                           reverbDuck->get(), reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDuckID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(),
                                           reverbDuck->getNormalisableRange().convertFrom0to1(msg.value()),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbLoCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->getNormalisableRange().convertFrom0to1(msg.value()),
                                           reverbHiCut->get(), reverbToggle->get());
            case (reverbHiCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->getNormalisableRange().convertFrom0to1(msg.value()),
                                           reverbToggle->get());
            case (midiLatchID):
                activeEngine.updateMidiLatch (_BOOL_MSG_);
            case (modulatorSourceID):
                activeEngine.setModulatorSource (floatParamToModulatorSource (msg.value()));
            case (killAllMidiID):
                if (msg.value() > 0.0f) activeEngine.killAllMidi();
            case (pitchbendFromEditorID):
                activeEngine.recieveExternalPitchbend (juce::roundToInt (juce::jmap (msg.value(), 0.0f, 127.0f)));
            case (numVoicesID):
                updateNumVoices (floatParamToNumVoices (msg.value()));
                
            default: jassertfalse;  // an unknown parameter ID results in an error
        }
    }
#undef _BOOL_MSG_
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<double>& activeEngine);



float ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass->get() ? 1.0f : 0.0f;
        case (leadBypassID):            return leadBypass->get() ? 1.0f : 0.0f;
        case (harmonyBypassID):         return harmonyBypass->get() ? 1.0f : 0.0f;
        case (dryPanID):                return dryPan->getNormalisableRange().convertTo0to1 (dryPan->get());
        case (dryWetID):                return dryWet->getNormalisableRange().convertTo0to1 (dryWet->get());
        case (adsrAttackID):            return adsrAttack->getNormalisableRange().convertTo0to1 (adsrAttack->get());
        case (adsrDecayID):             return adsrDecay->getNormalisableRange().convertTo0to1 (adsrDecay->get());
        case (adsrSustainID):           return adsrSustain->getNormalisableRange().convertTo0to1 (adsrSustain->get());
        case (adsrReleaseID):           return adsrRelease->getNormalisableRange().convertTo0to1 (adsrRelease->get());
        case (adsrToggleID):            return adsrToggle->get() ? 1.0f : 0.0f;
        case (stereoWidthID):           return stereoWidth->getNormalisableRange().convertTo0to1 (stereoWidth->get());
        case (lowestPannedID):          return lowestPanned->getNormalisableRange().convertTo0to1 (lowestPanned->get());
        case (velocitySensID):          return velocitySens->getNormalisableRange().convertTo0to1 (velocitySens->get());
        case (pitchBendRangeID):        return pitchBendRange->getNormalisableRange().convertTo0to1 (pitchBendRange->get());
        case (pedalPitchIsOnID):        return pedalPitchIsOn->get() ? 1.0f : 0.0f;
        case (pedalPitchThreshID):      return pedalPitchThresh->getNormalisableRange().convertTo0to1 (pedalPitchThresh->get());
        case (pedalPitchIntervalID):    return pedalPitchInterval->getNormalisableRange().convertTo0to1 (pedalPitchInterval->get());
        case (descantIsOnID):           return descantIsOn->get() ? 1.0f : 0.0f;
        case (descantThreshID):         return descantThresh->getNormalisableRange().convertTo0to1 (descantThresh->get());
        case (descantIntervalID):       return descantInterval->getNormalisableRange().convertTo0to1 (descantInterval->get());
        case (concertPitchHzID):        return concertPitchHz->getNormalisableRange().convertTo0to1 (concertPitchHz->get());
        case (voiceStealingID):         return voiceStealing->get() ? 1.0f : 0.0f;
        case (inputGainID):             return inputGain->getNormalisableRange().convertTo0to1 (inputGain->get());
        case (outputGainID):            return outputGain->getNormalisableRange().convertTo0to1 (outputGain->get());
        case (limiterToggleID):         return limiterToggle->get() ? 1.0f : 0.0f;
        case (noiseGateToggleID):       return noiseGateToggle->get() ? 1.0f : 0.0f;
        case (noiseGateThresholdID):    return noiseGateThreshold->getNormalisableRange().convertTo0to1 (noiseGateThreshold->get());
        case (compressorToggleID):      return compressorToggle->get() ? 1.0f : 0.0f;
        case (compressorAmountID):      return compressorAmount->getNormalisableRange().convertTo0to1 (compressorAmount->get());
        case (aftertouchGainToggleID):  return aftertouchGainToggle->get() ? 1.0f : 0.0f;
        case (deEsserToggleID):         return deEsserToggle->get() ? 1.0f : 0.0f;
        case (deEsserThreshID):         return deEsserThresh->getNormalisableRange().convertTo0to1 (deEsserThresh->get());
        case (deEsserAmountID):         return deEsserAmount->getNormalisableRange().convertTo0to1 (deEsserAmount->get());
        case (reverbToggleID):          return reverbToggle->get() ? 1.0f : 0.0f;
        case (reverbDryWetID):          return reverbDryWet->getNormalisableRange().convertTo0to1 (reverbDryWet->get());
        case (reverbDecayID):           return reverbDecay->getNormalisableRange().convertTo0to1 (reverbDecay->get());
        case (reverbDuckID):            return reverbDuck->getNormalisableRange().convertTo0to1 (reverbDuck->get());
        case (reverbLoCutID):           return reverbLoCut->getNormalisableRange().convertTo0to1 (reverbLoCut->get());
        case (reverbHiCutID):           return reverbHiCut->getNormalisableRange().convertTo0to1 (reverbHiCut->get());
        case (vocalRangeTypeID):        return vocalRangeTypeToFloatParam (vocalRangeType->getCurrentChoiceName());
            
        // message types that aren't actually automated parameters
        case (pitchbendFromEditorID):   return 0.5f;
        case (killAllMidiID):           return 0.0f;
        
        // specially keyed/converted values
        case (midiLatchID):
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched() ? 1.0f : 0.0f;
            else return floatEngine.isMidiLatched() ? 1.0f : 0.0f;
            
        case (modulatorSourceID):
            if (isUsingDoublePrecision()) return modulatorSourceToFloatParam (doubleEngine.getModulatorSource());
            else return modulatorSourceToFloatParam (floatEngine.getModulatorSource());
            
        case (numVoicesID):
            if (isUsingDoublePrecision()) return numVoicesToFloatParam (doubleEngine.getCurrentNumVoices());
            else return numVoicesToFloatParam (floatEngine.getCurrentNumVoices());
            
        default: jassertfalse;  // an unknown parameter ID results in an error
    }
}


float ImogenAudioProcessor::getDefaultParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return 0.0f; // from the plugin's point of view, the default state for the main bypass is false (not bypassed)
        case (leadBypassID):            return defaultLeadBypass.load() ? 1.0f : 0.0f;
        case (harmonyBypassID):         return defaultHarmonyBypass.load() ? 1.0f : 0.0f;
        case (dryPanID):                return dryPan->getNormalisableRange().convertTo0to1 (defaultDryPan.load());
        case (dryWetID):                return dryWet->getNormalisableRange().convertTo0to1 (defaultDryWet.load());
        case (adsrAttackID):            return adsrAttack->getNormalisableRange().convertTo0to1 (defaultAdsrAttack.load());
        case (adsrDecayID):             return adsrDecay->getNormalisableRange().convertTo0to1 (defaultAdsrDecay.load());
        case (adsrSustainID):           return adsrSustain->getNormalisableRange().convertTo0to1 (defaultAdsrSustain.load());
        case (adsrReleaseID):           return adsrRelease->getNormalisableRange().convertTo0to1 (defaultAdsrRelease.load());
        case (adsrToggleID):            return defaultAdsrToggle.load() ? 1.0f : 0.0f;
        case (stereoWidthID):           return stereoWidth->getNormalisableRange().convertTo0to1 (defaultStereoWidth.load());
        case (lowestPannedID):          return lowestPanned->getNormalisableRange().convertTo0to1 (defaultLowestPannedNote.load());
        case (velocitySensID):          return velocitySens->getNormalisableRange().convertTo0to1 (defaultVelocitySensitivity.load());
        case (pitchBendRangeID):        return pitchBendRange->getNormalisableRange().convertTo0to1 (defaultPitchbendRange.load());
        case (pedalPitchIsOnID):        return defaultPedalPitchToggle.load() ? 1.0f : 0.0f;
        case (pedalPitchThreshID):      return pedalPitchThresh->getNormalisableRange().convertTo0to1 (defaultPedalPitchThresh.load());
        case (pedalPitchIntervalID):    return pedalPitchInterval->getNormalisableRange().convertTo0to1 (defaultPedalPitchInterval.load());
        case (descantIsOnID):           return defaultDescantToggle.load() ? 1.0f : 0.0f;
        case (descantThreshID):         return descantThresh->getNormalisableRange().convertTo0to1 (defaultDescantThresh.load());
        case (descantIntervalID):       return descantInterval->getNormalisableRange().convertTo0to1 (defaultDescantInterval.load());
        case (concertPitchHzID):        return defaultConcertPitchHz.load();
        case (voiceStealingID):         return defaultVoiceStealingToggle.load() ? 1.0f : 0.0f;
        case (inputGainID):             return inputGain->getNormalisableRange().convertTo0to1 (defaultInputGain.load());
        case (outputGainID):            return outputGain->getNormalisableRange().convertTo0to1 (defaultOutputGain.load());
        case (limiterToggleID):         return defaultLimiterToggle.load() ? 1.0f : 0.0f;
        case (noiseGateToggleID):       return defaultNoiseGateToggle.load() ? 1.0f : 0.0f;
        case (noiseGateThresholdID):    return noiseGateThreshold->getNormalisableRange().convertTo0to1 (defaultNoiseGateThresh.load());
        case (compressorToggleID):      return defaultCompressorToggle.load() ? 1.0f : 0.0f;
        case (compressorAmountID):      return compressorAmount->getNormalisableRange().convertTo0to1 (defaultCompressorAmount.load());
        case (aftertouchGainToggleID):  return defaultAftertouchGainToggle.load() ? 1.0f : 0.0f;
        case (deEsserToggleID):         return defaultDeEsserToggle.load() ? 1.0f : 0.0f;
        case (deEsserThreshID):         return deEsserThresh->getNormalisableRange().convertTo0to1 (defaultDeEsserThresh.load());
        case (deEsserAmountID):         return deEsserAmount->getNormalisableRange().convertTo0to1 (defaultDeEsserAmount.load());
        case (reverbToggleID):          return defaultReverbToggle.load() ? 1.0f : 0.0f;
        case (reverbDryWetID):          return reverbDryWet->getNormalisableRange().convertTo0to1 (defaultReverbDryWet.load());
        case (reverbDecayID):           return reverbDecay->getNormalisableRange().convertTo0to1 (defaultReverbDecay);
        case (reverbDuckID):            return reverbDuck->getNormalisableRange().convertTo0to1 (defaultReverbDuck.load());
        case (reverbLoCutID):           return reverbLoCut->getNormalisableRange().convertTo0to1 (defaultReverbLoCut.load());
        case (reverbHiCutID):           return reverbHiCut->getNormalisableRange().convertTo0to1 (defaultReverbHiCut.load());
        case (vocalRangeTypeID):        return vocalRangeTypeToFloatParam (vocalRangeTypes[defaultVocalRangeIndex.load()]);
        case (numVoicesID):             return numVoicesToFloatParam (defaultNumVoices.load());
        case (modulatorSourceID):       return modulatorSourceToFloatParam (defaultModulatorSource.load());
        case (killAllMidiID):           return 0.0f;
        case (pitchbendFromEditorID):   return 0.5f;
        
        case (midiLatchID):  // for midi latch, its state should never be changed by a query for a "default"
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched() ? 1.0f : 0.0f;
            else return floatEngine.isMidiLatched() ? 1.0f : 0.0f;
            
        default: jassertfalse;  // an unknown parameter ID results in an error
    }
}


void ImogenAudioProcessor::updateVocalRangeType (juce::String newRangeType)
{
    int minHz, maxHz;
    
    if (newRangeType.equalsIgnoreCase ("Soprano"))
    {
        minHz = bav::math::midiToFreq (57);
        maxHz = bav::math::midiToFreq (88);
    }
    else if (newRangeType.equalsIgnoreCase ("Alto"))
    {
        minHz = bav::math::midiToFreq (50);
        maxHz = bav::math::midiToFreq (81);
    }
    else if (newRangeType.equalsIgnoreCase ("Tenor"))
    {
        minHz = bav::math::midiToFreq (43);
        maxHz = bav::math::midiToFreq (76);
    }
    else
    {
        jassert (newRangeType.equalsIgnoreCase ("Bass"));
        minHz = bav::math::midiToFreq (36);
        maxHz = bav::math::midiToFreq (67);
    }
    
    suspendProcessing (true);
    
    if (isUsingDoublePrecision())
    {
        doubleEngine.updatePitchDetectionHzRange (minHz, maxHz);
        setLatencySamples (doubleEngine.reportLatency());
    }
    else
    {
        floatEngine.updatePitchDetectionHzRange (minHz, maxHz);
        setLatencySamples (floatEngine.reportLatency());
    }
    
    suspendProcessing (false);
}


template<typename SampleType>
void ImogenAudioProcessor::updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                                             bool compressorIsOn, float knobValue)
{
    jassert (knobValue >= 0.0f && knobValue <= 1.0f);
    
    activeEngine.updateCompressor (juce::jmap (knobValue, 0.0f, -60.0f),  // threshold (dB)
                                   juce::jmap (knobValue, 1.0f, 10.0f),  // ratio
                                   compressorIsOn);
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



// functions for preset & state management system ---------------------------------------------------------------------------------------------------

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
    
    xml->setAttribute ("numberOfVoices", activeEngine.getCurrentNumVoices());
    xml->setAttribute ("modulatorInputSource", activeEngine.getModulatorSource());
    
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
    return true;
}
