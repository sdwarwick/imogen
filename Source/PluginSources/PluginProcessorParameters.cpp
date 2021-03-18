
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
    updateVocalRangeType (vocalRangeType->getIndex());
    
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
#define _BOOL_MSG_ msg.value() >= 0.5f   // converts a message's float value to a boolean true/false
#define _INT_0_100 juce::roundToInt (msg.value() * 100.0f)  // converts a message's float value to an integer in the range 0 to 100
        switch (msg.type())
        {
            case (leadBypassID):
                activeEngine.updateBypassStates (_BOOL_MSG_, harmonyBypass->get());
            case (harmonyBypassID):
                activeEngine.updateBypassStates (leadBypass->get(), _BOOL_MSG_);
            case (dryPanID):
                activeEngine.updateDryVoxPan (dryPan->get());
            case (dryWetID):
                activeEngine.updateDryWet (_INT_0_100);
            case (adsrAttackID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrDecayID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrSustainID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), msg.value(), adsrRelease->get(), adsrToggle->get());
            case (adsrReleaseID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrToggleID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), _BOOL_MSG_);
            case (stereoWidthID):
                activeEngine.updateStereoWidth (_INT_0_100, lowestPanned->get());
            case (lowestPannedID):
                activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
            case (velocitySensID):
                activeEngine.updateMidiVelocitySensitivity (_INT_0_100);
            case (pitchBendRangeID):
                activeEngine.updatePitchbendRange (pitchBendRange->get());
            case (pedalPitchIsOnID):
                activeEngine.updatePedalPitch (_BOOL_MSG_, pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchThreshID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchIntervalID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
            case (descantIsOnID):
                activeEngine.updateDescant (_BOOL_MSG_, descantThresh->get(), descantInterval->get());
            case (descantThreshID):
                activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), descantInterval->get());
            case (descantIntervalID):
                activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), descantInterval->get());
            case (concertPitchHzID):
                activeEngine.updateConcertPitch (concertPitchHz->get());
            case (voiceStealingID):
                activeEngine.updateNoteStealing (_BOOL_MSG_);
            case (inputGainID):
                activeEngine.updateInputGain (juce::Decibels::decibelsToGain (inputGain->get()));
            case (outputGainID):
                activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (outputGain->get()));
            case (limiterToggleID):
                activeEngine.updateLimiter (_BOOL_MSG_);
            case (noiseGateToggleID):
                activeEngine.updateNoiseGate (noiseGateThreshold->get(), _BOOL_MSG_);
            case (noiseGateThresholdID):
                activeEngine.updateNoiseGate (noiseGateThreshold->get(), noiseGateToggle->get());
            case (compressorToggleID):
                updateCompressor (activeEngine, _BOOL_MSG_, compressorAmount->get());
            case (compressorAmountID):
                updateCompressor (activeEngine, compressorToggle->get(), msg.value());
            case (vocalRangeTypeID):
                updateVocalRangeType (vocalRangeType->getIndex());
            case (aftertouchGainToggleID):
                activeEngine.updateAftertouchGainOnOff (_BOOL_MSG_);
            case (deEsserToggleID):
                activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), _BOOL_MSG_);
            case (deEsserThreshID):
                activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), deEsserToggle->get());
            case (deEsserAmountID):
                activeEngine.updateDeEsser (msg.value(), deEsserThresh->get(), deEsserToggle->get());
            case (reverbToggleID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), _BOOL_MSG_);
            case (reverbDryWetID):
                activeEngine.updateReverb (_INT_0_100, reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDecayID):
                activeEngine.updateReverb (reverbDryWet->get(), msg.value(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDuckID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), msg.value(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbLoCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbHiCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (midiLatchID):
                activeEngine.updateMidiLatch (_BOOL_MSG_);
            case (modulatorSourceID):
                activeEngine.setModulatorSource (floatParamToModulatorSource (msg.value()));
            case (killAllMidiID):
                activeEngine.killAllMidi();
            case (pitchbendFromEditorID):
                activeEngine.recieveExternalPitchbend (juce::roundToInt (juce::jmap (msg.value(), 0.0f, 127.0f)));
            case (numVoicesID):
                updateNumVoices (floatParamToNumVoices (msg.value()));
                
            default: jassertfalse;  // an unknown parameter ID results in an error
        }
    }
#undef _BOOL_MSG_
#undef _INT_0_100
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<double>& activeEngine);


template<typename ValueType>
ValueType ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass->get();
        case (leadBypassID):            return leadBypass->get();
        case (harmonyBypassID):         return harmonyBypass->get();
        case (dryPanID):                return dryPan->get();
        case (dryWetID):                return dryWet->get();
        case (adsrAttackID):            return adsrAttack->get();
        case (adsrDecayID):             return adsrDecay->get();
        case (adsrSustainID):           return adsrSustain->get();
        case (adsrReleaseID):           return adsrRelease->get();
        case (adsrToggleID):            return adsrToggle->get();
        case (stereoWidthID):           return stereoWidth->get();
        case (lowestPannedID):          return lowestPanned->get();
        case (velocitySensID):          return velocitySens->get();
        case (pitchBendRangeID):        return pitchBendRange->get();
        case (pedalPitchIsOnID):        return pedalPitchIsOn->get();
        case (pedalPitchThreshID):      return pedalPitchThresh->get();
        case (pedalPitchIntervalID):    return pedalPitchInterval->get();
        case (descantIsOnID):           return descantIsOn->get();
        case (descantThreshID):         return descantThresh->get();
        case (descantIntervalID):       return descantInterval->get();
        case (concertPitchHzID):        return concertPitchHz->get();
        case (voiceStealingID):         return voiceStealing->get();
        case (inputGainID):             return inputGain->get();
        case (outputGainID):            return outputGain->get();
        case (limiterToggleID):         return limiterToggle->get();
        case (noiseGateToggleID):       return noiseGateToggle->get();
        case (noiseGateThresholdID):    return noiseGateThreshold->get();
        case (compressorToggleID):      return compressorToggle->get();
        case (compressorAmountID):      return compressorAmount->get();
        case (aftertouchGainToggleID):  return aftertouchGainToggle->get();
        case (deEsserToggleID):         return deEsserToggle->get();
        case (deEsserThreshID):         return deEsserThresh->get();
        case (deEsserAmountID):         return deEsserAmount->get();
        case (reverbToggleID):          return reverbToggle->get();
        case (reverbDryWetID):          return reverbDryWet->get();
        case (reverbDecayID):           return reverbDecay->get();
        case (reverbDuckID):            return reverbDuck->get();
        case (reverbLoCutID):           return reverbLoCut->get();
        case (reverbHiCutID):           return reverbHiCut->get();
        case (vocalRangeTypeID):        return vocalRangeType->getCurrentChoiceName();  // this one returns the name of the currently selected vocal range as a juce::String
        case (midiLatchID):
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched();
            else return floatEngine.isMidiLatched();
        case (modulatorSourceID):
            if (isUsingDoublePrecision()) return doubleEngine.getModulatorSource();
            else return floatEngine.getModulatorSource();
        case (killAllMidiID): return 0.0f;
        case (pitchbendFromEditorID): return 64;
        case (numVoicesID):
            if (isUsingDoublePrecision()) return doubleEngine.getCurrentNumVoices();
            else return floatEngine.getCurrentNumVoices();
    }
}
///function template instantiations...
//template float ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const;
//template int ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const;
//template bool ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const;


template<typename ValueType>
ValueType ImogenAudioProcessor::getDefaultParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return false; // from the plugin's point of view, the default state for the main bypass is false
        case (leadBypassID):            return defaultLeadBypass.load();
        case (harmonyBypassID):         return defaultHarmonyBypass.load();
        case (dryPanID):                return defaultDryPan.load();
        case (dryWetID):                return defaultDryWet.load();
        case (adsrAttackID):            return defaultAdsrAttack.load();
        case (adsrDecayID):             return defaultAdsrDecay.load();
        case (adsrSustainID):           return defaultAdsrSustain.load();
        case (adsrReleaseID):           return defaultAdsrRelease.load();
        case (adsrToggleID):            return defaultAdsrToggle.load();
        case (stereoWidthID):           return defaultStereoWidth.load();
        case (lowestPannedID):          return defaultLowestPannedNote.load();
        case (velocitySensID):          return defaultVelocitySensitivity.load();
        case (pitchBendRangeID):        return defaultPitchbendRange.load();
        case (pedalPitchIsOnID):        return defaultPedalPitchToggle.load();
        case (pedalPitchThreshID):      return defaultPedalPitchThresh.load();
        case (pedalPitchIntervalID):    return defaultPedalPitchInterval.load();
        case (descantIsOnID):           return defaultDescantToggle.load();
        case (descantThreshID):         return defaultDescantThresh.load();
        case (descantIntervalID):       return defaultDescantInterval.load();
        case (concertPitchHzID):        return defaultConcertPitchHz.load();
        case (voiceStealingID):         return defaultVoiceStealingToggle.load();
        case (inputGainID):             return defaultInputGain.load();
        case (outputGainID):            return defaultOutputGain.load();
        case (limiterToggleID):         return defaultLimiterToggle.load();
        case (noiseGateToggleID):       return defaultNoiseGateToggle.load();
        case (noiseGateThresholdID):    return defaultNoiseGateThresh.load();
        case (compressorToggleID):      return defaultCompressorToggle.load();
        case (compressorAmountID):      return defaultCompressorAmount.load();
        case (aftertouchGainToggleID):  return defaultAftertouchGainToggle.load();
        case (deEsserToggleID):         return defaultDeEsserToggle.load();
        case (deEsserThreshID):         return defaultDeEsserThresh.load();
        case (deEsserAmountID):         return defaultDeEsserAmount.load();
        case (reverbToggleID):          return defaultReverbToggle.load();
        case (reverbDryWetID):          return defaultReverbDryWet.load();
        case (reverbDecayID):           return defaultReverbDecay.load();
        case (reverbDuckID):            return defaultReverbDuck.load();
        case (reverbLoCutID):           return defaultReverbLoCut.load();
        case (reverbHiCutID):           return defaultReverbHiCut.load();
        case (modulatorSourceID):       return defaultModulatorSource.load();
        case (vocalRangeTypeID):        return vocalRangeTypes[defaultVocalRangeIndex.load()]; // this one returns the name of the default vocal range as a juce::String
        case (midiLatchID):  // for midi latch, its state should never be changed by a query for a "default"
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched();
            else return floatEngine.isMidiLatched();
        case (killAllMidiID): return 0.0f;
        case (pitchbendFromEditorID): return 64;
        case (numVoicesID): return defaultNumVoices.load();
    }
}


void ImogenAudioProcessor::updateVocalRangeType (int rangeTypeIndex)
{
    if (prevRangeTypeIndex == rangeTypeIndex)
        return;
    
    const juce::String rangeType = vocalRangeTypes[rangeTypeIndex];
    
    int minHz = 80;
    int maxHz = 2400;
    
    if (rangeType.equalsIgnoreCase ("Soprano"))
    {
        minHz = bav::math::midiToFreq (57);
        maxHz = bav::math::midiToFreq (88);
    }
    else if (rangeType.equalsIgnoreCase ("Alto"))
    {
        minHz = bav::math::midiToFreq (50);
        maxHz = bav::math::midiToFreq (81);
    }
    else if (rangeType.equalsIgnoreCase ("Tenor"))
    {
        minHz = bav::math::midiToFreq (43);
        maxHz = bav::math::midiToFreq (76);
    }
    else if (rangeType.equalsIgnoreCase ("Bass"))
    {
        minHz = bav::math::midiToFreq (36);
        maxHz = bav::math::midiToFreq (67);
    }
    
    updatePitchDetectionHzRange (minHz, maxHz);
    prevRangeTypeIndex = rangeTypeIndex;
}


template<typename SampleType>
void ImogenAudioProcessor::updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                                             bool compressorIsOn, float knobValue)
{
    jassert (knobValue >= 0.0f && knobValue <= 1.0f);
    
    activeEngine.updateCompressor (juce::jmap (knobValue, 0.0f, 1.0f, 0.0f, -60.0f),  // threshold (dB)
                                   knobValue * 10.0f,  // ratio
                                   compressorIsOn);
}


void ImogenAudioProcessor::updatePitchDetectionHzRange (int minHz, int maxHz)
{
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
