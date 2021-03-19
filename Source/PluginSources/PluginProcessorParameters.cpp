
#include "PluginProcessor.h"


juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
    
    params.push_back (std::make_unique<juce::AudioParameterBool> ("mainBypass", "Bypass", false));
    params.push_back (std::make_unique<BoolParameter>  ("leadBypass", "Lead bypass", false));
    params.push_back (std::make_unique<BoolParameter>  ("harmonyBypass", "Harmony bypass", false));
    params.push_back (std::make_unique<IntParameter>   ("numVoices", "Number of voices", 1, 20, 12));
    params.push_back (std::make_unique<IntParameter>   ("inputSource", "Input source", 1, 3, 1));
    params.push_back (std::make_unique<IntParameter>   ("dryPan", "Dry vox pan", 0, 127, 64));
    params.push_back (std::make_unique<FloatParameter> ("adsrAttack", "ADSR Attack", msRange, 0.35f));
    params.push_back (std::make_unique<FloatParameter> ("adsrDecay", "ADSR Decay", msRange, 0.06f));
    params.push_back (std::make_unique<FloatParameter> ("adsrSustain", "ADSR Sustain", zeroToOneRange, 0.8f));
    params.push_back (std::make_unique<FloatParameter> ("adsrRelease", "ADSR Release", msRange, 0.1f));
    params.push_back (std::make_unique<BoolParameter>  ("adsrOnOff", "ADSR on/off", true));
    params.push_back (std::make_unique<IntParameter>   ("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back (std::make_unique<IntParameter>   ("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    params.push_back (std::make_unique<IntParameter>   ("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back (std::make_unique<IntParameter>   ("PitchBendRange", "Pitch bend range (st)", 0, 12, 2));
    params.push_back (std::make_unique<IntParameter>   ("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    params.push_back (std::make_unique<BoolParameter>  ("voiceStealing", "Voice stealing", false));
    params.push_back (std::make_unique<BoolParameter>  ("aftertouchGainToggle", "Aftertouch gain on/off", true));
    params.push_back (std::make_unique<BoolParameter>  ("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back (std::make_unique<IntParameter>   ("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back (std::make_unique<IntParameter>   ("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    params.push_back (std::make_unique<BoolParameter>  ("descantToggle", "Descant on/off", false));
    params.push_back (std::make_unique<IntParameter>   ("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back (std::make_unique<IntParameter>   ("descantInterval", "Descant interval", 1, 12, 12));
    params.push_back (std::make_unique<IntParameter>   ("masterDryWet", "% wet", 0, 100, 100));
    params.push_back (std::make_unique<FloatParameter> ("inputGain", "Input gain",   gainRange, 0.0f));
    params.push_back (std::make_unique<FloatParameter> ("outputGain", "Output gain", gainRange, -4.0f));
    params.push_back (std::make_unique<BoolParameter>  ("limiterIsOn", "Limiter on/off", true));
    params.push_back (std::make_unique<BoolParameter>  ("noiseGateIsOn", "Noise gate toggle", true));
    params.push_back (std::make_unique<FloatParameter> ("noiseGateThresh", "Noise gate threshold", gainRange, -20.0f));
    params.push_back (std::make_unique<BoolParameter>  ("deEsserIsOn", "De-esser toggle", true));
    params.push_back (std::make_unique<FloatParameter> ("deEsserThresh", "De-esser thresh", gainRange, -6.0f));
    params.push_back (std::make_unique<FloatParameter> ("deEsserAmount", "De-esser amount", zeroToOneRange, 0.5f));
    params.push_back (std::make_unique<BoolParameter>  ("compressorToggle", "Compressor on/off", false));
    params.push_back (std::make_unique<FloatParameter> ("compressorAmount", "Compressor amount", zeroToOneRange, 0.35f));
    params.push_back (std::make_unique<BoolParameter>  ("reverbIsOn", "Reverb toggle", false));
    params.push_back (std::make_unique<IntParameter>   ("reverbDryWet", "Reverb dry/wet", 0, 100, 35));
    params.push_back (std::make_unique<FloatParameter> ("reverbDecay", "Reverb decay", zeroToOneRange, 0.6f));
    params.push_back (std::make_unique<FloatParameter> ("reverbDuck", "Duck amount", zeroToOneRange, 0.3f));
    params.push_back (std::make_unique<FloatParameter> ("reverbLoCut", "Reverb low cut", hzRange, 80.0f));
    params.push_back (std::make_unique<FloatParameter> ("reverbHiCut", "Reverb high cut", hzRange, 5500.0f));
    
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
    mainBypass           = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter ("mainBypass"));       jassert (mainBypass);
    leadBypass           = dynamic_cast<BoolParamPtr>  (tree.getParameter ("leadBypass"));                   jassert (leadBypass);
    harmonyBypass        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("harmonyBypass"));                jassert (harmonyBypass);
    numVoices            = dynamic_cast<IntParamPtr>   (tree.getParameter ("numVoices"));                    jassert (numVoices);
    inputSource          = dynamic_cast<IntParamPtr>   (tree.getParameter ("inputSource"));                  jassert (inputSource);
    dryPan               = dynamic_cast<IntParamPtr>   (tree.getParameter ("dryPan"));                       jassert (dryPan);
    dryWet               = dynamic_cast<IntParamPtr>   (tree.getParameter ("masterDryWet"));                 jassert (dryWet);
    adsrAttack           = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrAttack"));                   jassert (adsrAttack);
    adsrDecay            = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrDecay"));                    jassert (adsrDecay);
    adsrSustain          = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrSustain"));                  jassert (adsrSustain);
    adsrRelease          = dynamic_cast<FloatParamPtr> (tree.getParameter ("adsrRelease"));                  jassert (adsrRelease);
    adsrToggle           = dynamic_cast<BoolParamPtr>  (tree.getParameter ("adsrOnOff"));                    jassert (adsrToggle);
    stereoWidth          = dynamic_cast<IntParamPtr>   (tree.getParameter ("stereoWidth"));                  jassert (stereoWidth);
    lowestPanned         = dynamic_cast<IntParamPtr>   (tree.getParameter ("lowestPan"));                    jassert (lowestPanned);
    velocitySens         = dynamic_cast<IntParamPtr>   (tree.getParameter ("midiVelocitySensitivity"));      jassert (velocitySens);
    pitchBendRange       = dynamic_cast<IntParamPtr>   (tree.getParameter ("PitchBendRange"));               jassert (pitchBendRange);
    pedalPitchIsOn       = dynamic_cast<BoolParamPtr>  (tree.getParameter ("pedalPitchToggle"));             jassert (pedalPitchIsOn);
    pedalPitchThresh     = dynamic_cast<IntParamPtr>   (tree.getParameter ("pedalPitchThresh"));             jassert (pedalPitchThresh);
    pedalPitchInterval   = dynamic_cast<IntParamPtr>   (tree.getParameter ("pedalPitchInterval"));           jassert (pedalPitchInterval);
    descantIsOn          = dynamic_cast<BoolParamPtr>  (tree.getParameter ("descantToggle"));                jassert (descantIsOn);
    descantThresh        = dynamic_cast<IntParamPtr>   (tree.getParameter ("descantThresh"));                jassert (descantThresh);
    descantInterval      = dynamic_cast<IntParamPtr>   (tree.getParameter ("descantInterval"));              jassert (descantInterval);
    concertPitchHz       = dynamic_cast<IntParamPtr>   (tree.getParameter ("concertPitch"));                 jassert (concertPitchHz);
    voiceStealing        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("voiceStealing"));                jassert (voiceStealing);
    inputGain            = dynamic_cast<FloatParamPtr> (tree.getParameter ("inputGain"));                    jassert (inputGain);
    outputGain           = dynamic_cast<FloatParamPtr> (tree.getParameter ("outputGain"));                   jassert (outputGain);
    limiterToggle        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("limiterIsOn"));                  jassert (limiterToggle);
    noiseGateThreshold   = dynamic_cast<FloatParamPtr> (tree.getParameter ("noiseGateThresh"));              jassert (noiseGateThreshold);
    noiseGateToggle      = dynamic_cast<BoolParamPtr>  (tree.getParameter ("noiseGateIsOn"));                jassert (noiseGateToggle);
    compressorToggle     = dynamic_cast<BoolParamPtr>  (tree.getParameter ("compressorToggle"));             jassert (compressorToggle);
    compressorAmount     = dynamic_cast<FloatParamPtr> (tree.getParameter ("compressorAmount"));             jassert (compressorAmount);
    aftertouchGainToggle = dynamic_cast<BoolParamPtr>  (tree.getParameter ("aftertouchGainToggle"));         jassert (aftertouchGainToggle);
    deEsserToggle        = dynamic_cast<BoolParamPtr>  (tree.getParameter ("deEsserIsOn"));                  jassert (deEsserToggle);
    deEsserThresh        = dynamic_cast<FloatParamPtr> (tree.getParameter ("deEsserThresh"));                jassert (deEsserThresh);
    deEsserAmount        = dynamic_cast<FloatParamPtr> (tree.getParameter ("deEsserAmount"));                jassert (deEsserAmount);
    reverbToggle         = dynamic_cast<BoolParamPtr>  (tree.getParameter ("reverbIsOn"));                   jassert (reverbToggle);
    reverbDryWet         = dynamic_cast<IntParamPtr>   (tree.getParameter ("reverbDryWet"));                 jassert (reverbDryWet);
    reverbDecay          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbDecay"));                  jassert (reverbDecay);
    reverbDuck           = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbDuck"));                   jassert (reverbDuck);
    reverbLoCut          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbLoCut"));                  jassert (reverbLoCut);
    reverbHiCut          = dynamic_cast<FloatParamPtr> (tree.getParameter ("reverbHiCut"));                  jassert (reverbHiCut);
    vocalRangeType       = dynamic_cast<juce::AudioParameterChoice*> (tree.getParameter ("vocalRangeType")); jassert (vocalRangeType);
}


void ImogenAudioProcessor::initializeParameterListeners()
{
    addParameterMessenger ("mainBypass",                mainBypassID);
    addParameterMessenger ("leadBypass",                leadBypassID);
    addParameterMessenger ("harmonyBypass",             harmonyBypassID);
    addParameterMessenger ("numVoices",                 numVoicesID);
    addParameterMessenger ("inputSource",               inputSourceID);
    addParameterMessenger ("dryPan",                    dryPanID);
    addParameterMessenger ("masterDryWet",              dryWetID);
    addParameterMessenger ("adsrAttack",                adsrAttackID);
    addParameterMessenger ("adsrDecay",                 adsrDecayID);
    addParameterMessenger ("adsrSustain",               adsrSustainID);
    addParameterMessenger ("adsrRelease",               adsrReleaseID);
    addParameterMessenger ("adsrOnOff",                 adsrToggleID);
    addParameterMessenger ("stereoWidth",               stereoWidthID);
    addParameterMessenger ("lowestPan",                 lowestPannedID);
    addParameterMessenger ("midiVelocitySensitivity",   velocitySensID);
    addParameterMessenger ("PitchBendRange",            pitchBendRangeID);
    addParameterMessenger ("pedalPitchToggle",          pedalPitchIsOnID);
    addParameterMessenger ("pedalPitchThresh",          pedalPitchThreshID);
    addParameterMessenger ("pedalPitchInterval",        pedalPitchIntervalID);
    addParameterMessenger ("descantToggle",             descantIsOnID);
    addParameterMessenger ("descantThresh",             descantThreshID);
    addParameterMessenger ("descantInterval",           descantIntervalID);
    addParameterMessenger ("concertPitch",              concertPitchHzID);
    addParameterMessenger ("voiceStealing",             voiceStealingID);
    addParameterMessenger ("inputGain",                 inputGainID);
    addParameterMessenger ("outputGain",                outputGainID);
    addParameterMessenger ("limiterIsOn",               limiterToggleID);
    addParameterMessenger ("noiseGateIsOn",             noiseGateToggleID);
    addParameterMessenger ("noiseGateThresh",           noiseGateThresholdID);
    addParameterMessenger ("compressorToggle",          compressorToggleID);
    addParameterMessenger ("compressorAmount",          compressorAmountID);
    addParameterMessenger ("vocalRangeType",            vocalRangeTypeID);
    addParameterMessenger ("aftertouchGainToggle",      aftertouchGainToggleID);
    addParameterMessenger ("deEsserIsOn",               deEsserToggleID);
    addParameterMessenger ("deEsserThresh",             deEsserThreshID);
    addParameterMessenger ("deEsserAmount",             deEsserAmountID);
    addParameterMessenger ("reverbIsOn",                reverbToggleID);
    addParameterMessenger ("reverbDryWet",              reverbDryWetID);
    addParameterMessenger ("reverbDecay",               reverbDecayID);
    addParameterMessenger ("reverbDuck",                reverbDuckID);
    addParameterMessenger ("reverbLoCut",               reverbLoCutID);
    addParameterMessenger ("reverbHiCut",               reverbHiCutID);
}

void ImogenAudioProcessor::addParameterMessenger (juce::String stringID, int paramID)
{
    auto& messenger { parameterMessengers.emplace_back (paramChangesForProcessor, paramChangesForEditor, paramID) };
    tree.addParameterListener (stringID, &messenger);
}


void ImogenAudioProcessor::updateParameterDefaults()
{
    leadBypass->refreshDefault();
    harmonyBypass->refreshDefault();
    numVoices->refreshDefault();
    inputSource->refreshDefault();
    dryPan->refreshDefault();
    dryWet->refreshDefault();
    stereoWidth->refreshDefault();
    lowestPanned->refreshDefault();
    velocitySens->refreshDefault();
    pitchBendRange->refreshDefault();
    pedalPitchIsOn->refreshDefault();
    pedalPitchThresh->refreshDefault();
    pedalPitchInterval->refreshDefault();
    descantIsOn->refreshDefault();
    descantThresh->refreshDefault();
    descantInterval->refreshDefault();
    concertPitchHz->refreshDefault();
    adsrToggle->refreshDefault();
    adsrAttack->refreshDefault();
    adsrDecay->refreshDefault();
    adsrSustain->refreshDefault();
    adsrRelease->refreshDefault();
    inputGain->refreshDefault();
    outputGain->refreshDefault();
    noiseGateToggle->refreshDefault();
    noiseGateThreshold->refreshDefault();
    compressorToggle->refreshDefault();
    compressorAmount->refreshDefault();
    deEsserToggle->refreshDefault();
    deEsserThresh->refreshDefault();
    deEsserAmount->refreshDefault();
    reverbToggle->refreshDefault();
    reverbDecay->refreshDefault();
    reverbDuck->refreshDefault();
    reverbLoCut->refreshDefault();
    reverbHiCut->refreshDefault();
    reverbDryWet->refreshDefault();
    limiterToggle->refreshDefault();
    voiceStealing->refreshDefault();
    aftertouchGainToggle->refreshDefault();
    defaultVocalRangeIndex.store (vocalRangeType->getIndex());
    
    parameterDefaultsAreDirty.store (true);
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
    
    updateNumVoices (numVoices->get());
    
    activeEngine.setModulatorSource (inputSource->get());
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

// converts a message's value to a boolean true/false
#define _BOOL_MSG_ msg.value() >= 0.5f

// converts a message's value to a float using the normalisable range of the specified parameter
#define _FLOAT_MSG_(p) p->getNormalisableRange().convertFrom0to1(msg.value())
        
// converts a message's value to an integer using the normalisable range of the specified parameter
#define _INT_MSG_(p) juce::roundToInt (_FLOAT_MSG_(p))
        
        switch (msg.type())
        {
            case (leadBypassID):
                activeEngine.updateBypassStates (_BOOL_MSG_, harmonyBypass->get());
            case (harmonyBypassID):
                activeEngine.updateBypassStates (leadBypass->get(), _BOOL_MSG_);
            case (dryPanID):
                activeEngine.updateDryVoxPan (_INT_MSG_(dryPan));
            case (dryWetID):
                activeEngine.updateDryWet (_INT_MSG_(dryWet));
            case (adsrAttackID):
                activeEngine.updateAdsr (_FLOAT_MSG_(adsrAttack), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrDecayID):
                activeEngine.updateAdsr (adsrAttack->get(), _FLOAT_MSG_(adsrDecay), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
            case (adsrSustainID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), _FLOAT_MSG_(adsrSustain), adsrRelease->get(), adsrToggle->get());
            case (adsrReleaseID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), _FLOAT_MSG_(adsrRelease), adsrToggle->get());
            case (adsrToggleID):
                activeEngine.updateAdsr (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), _BOOL_MSG_);
            case (stereoWidthID):
                activeEngine.updateStereoWidth (_INT_MSG_(stereoWidth), lowestPanned->get());
            case (lowestPannedID):
                activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
            case (velocitySensID):
                activeEngine.updateMidiVelocitySensitivity (_INT_MSG_(velocitySens));
            case (pitchBendRangeID):
                activeEngine.updatePitchbendRange (_INT_MSG_(pitchBendRange));
            case (pedalPitchIsOnID):
                activeEngine.updatePedalPitch (_BOOL_MSG_, pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchThreshID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(), _INT_MSG_(pedalPitchThresh), pedalPitchInterval->get());
            case (pedalPitchIntervalID):
                activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), _INT_MSG_(pedalPitchInterval));
            case (descantIsOnID):
                activeEngine.updateDescant (_BOOL_MSG_, descantThresh->get(), descantInterval->get());
            case (descantThreshID):
                activeEngine.updateDescant (descantIsOn->get(), _INT_MSG_(descantThresh), descantInterval->get());
            case (descantIntervalID):
                activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), _INT_MSG_(descantInterval));
            case (concertPitchHzID):
                activeEngine.updateConcertPitch (_INT_MSG_(concertPitchHz));
            case (voiceStealingID):
                activeEngine.updateNoteStealing (_BOOL_MSG_);
            case (inputGainID):
                activeEngine.updateInputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG_(inputGain)));
            case (outputGainID):
                activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG_(outputGain)));
            case (limiterToggleID):
                activeEngine.updateLimiter (_BOOL_MSG_);
            case (noiseGateToggleID):
                activeEngine.updateNoiseGate (noiseGateThreshold->get(), _BOOL_MSG_);
            case (noiseGateThresholdID):
                activeEngine.updateNoiseGate (_FLOAT_MSG_(noiseGateThreshold), noiseGateToggle->get());
            case (compressorToggleID):
                updateCompressor (activeEngine, _BOOL_MSG_, compressorAmount->get());
            case (compressorAmountID):
                updateCompressor (activeEngine, compressorToggle->get(), _FLOAT_MSG_(compressorAmount));
            case (vocalRangeTypeID):
                updateVocalRangeType (floatParamToVocalRangeType (msg.value()));
            case (aftertouchGainToggleID):
                activeEngine.updateAftertouchGainOnOff (_BOOL_MSG_);
            case (deEsserToggleID):
                activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), _BOOL_MSG_);
            case (deEsserThreshID):
                activeEngine.updateDeEsser (deEsserAmount->get(), _FLOAT_MSG_(deEsserThresh), deEsserToggle->get());
            case (deEsserAmountID):
                activeEngine.updateDeEsser (_FLOAT_MSG_(deEsserAmount), deEsserThresh->get(), deEsserToggle->get());
            case (reverbToggleID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), _BOOL_MSG_);
            case (reverbDryWetID):
                activeEngine.updateReverb (_INT_MSG_(reverbDryWet), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDecayID):
                activeEngine.updateReverb (reverbDryWet->get(), _FLOAT_MSG_(reverbDecay), reverbDuck->get(),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbDuckID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), _FLOAT_MSG_(reverbDuck),
                                           reverbLoCut->get(), reverbHiCut->get(), reverbToggle->get());
            case (reverbLoCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           _FLOAT_MSG_(reverbLoCut), reverbHiCut->get(), reverbToggle->get());
            case (reverbHiCutID):
                activeEngine.updateReverb (reverbDryWet->get(), reverbDecay->get(), reverbDuck->get(),
                                           reverbLoCut->get(), _FLOAT_MSG_(reverbHiCut), reverbToggle->get());
            case (midiLatchID):
                activeEngine.updateMidiLatch (_BOOL_MSG_);
            case (inputSourceID):
                activeEngine.setModulatorSource (_INT_MSG_(inputSource));
            case (killAllMidiID):
                if (msg.value() > 0.0f) activeEngine.killAllMidi();
            case (pitchbendFromEditorID):
                activeEngine.recieveExternalPitchbend (juce::roundToInt (pitchBendNormRange.convertFrom0to1 (msg.value())));
                
            case (numVoicesID):
                updateNumVoices (_INT_MSG_(numVoices));
        }
    }
#undef _BOOL_MSG_
#undef _INT_MSG_
#undef _FLOAT_MSG_
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<double>& activeEngine);



float ImogenAudioProcessor::getCurrentParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass->get() ? 1.0f : 0.0f;
        case (leadBypassID):            return leadBypass->getCurrentNormalizedValue();
        case (harmonyBypassID):         return harmonyBypass->getCurrentNormalizedValue();
        case (dryPanID):                return dryPan->getCurrentNormalizedValue();
        case (dryWetID):                return dryWet->getCurrentNormalizedValue();
        case (adsrAttackID):            return adsrAttack->getCurrentNormalizedValue();
        case (adsrDecayID):             return adsrDecay->getCurrentNormalizedValue();
        case (adsrSustainID):           return adsrSustain->getCurrentNormalizedValue();
        case (adsrReleaseID):           return adsrRelease->getCurrentNormalizedValue();
        case (adsrToggleID):            return adsrToggle->getCurrentNormalizedValue();
        case (stereoWidthID):           return stereoWidth->getCurrentNormalizedValue();
        case (lowestPannedID):          return lowestPanned->getCurrentNormalizedValue();
        case (velocitySensID):          return velocitySens->getCurrentNormalizedValue();
        case (pitchBendRangeID):        return pitchBendRange->getCurrentNormalizedValue();
        case (pedalPitchIsOnID):        return pedalPitchIsOn->getCurrentNormalizedValue();
        case (pedalPitchThreshID):      return pedalPitchThresh->getCurrentNormalizedValue();
        case (pedalPitchIntervalID):    return pedalPitchInterval->getCurrentNormalizedValue();
        case (descantIsOnID):           return descantIsOn->getCurrentNormalizedValue();
        case (descantThreshID):         return descantThresh->getCurrentNormalizedValue();
        case (descantIntervalID):       return descantInterval->getCurrentNormalizedValue();
        case (concertPitchHzID):        return concertPitchHz->getCurrentNormalizedValue();
        case (voiceStealingID):         return voiceStealing->getCurrentNormalizedValue();
        case (inputGainID):             return inputGain->getCurrentNormalizedValue();
        case (outputGainID):            return outputGain->getCurrentNormalizedValue();
        case (limiterToggleID):         return limiterToggle->getCurrentNormalizedValue();
        case (noiseGateToggleID):       return noiseGateToggle->getCurrentNormalizedValue();
        case (noiseGateThresholdID):    return noiseGateThreshold->getCurrentNormalizedValue();
        case (compressorToggleID):      return compressorToggle->getCurrentNormalizedValue();
        case (compressorAmountID):      return compressorAmount->getCurrentNormalizedValue();
        case (aftertouchGainToggleID):  return aftertouchGainToggle->getCurrentNormalizedValue();
        case (deEsserToggleID):         return deEsserToggle->getCurrentNormalizedValue();
        case (deEsserThreshID):         return deEsserThresh->getCurrentNormalizedValue();
        case (deEsserAmountID):         return deEsserAmount->getCurrentNormalizedValue();
        case (reverbToggleID):          return reverbToggle->getCurrentNormalizedValue();
        case (reverbDryWetID):          return reverbDryWet->getCurrentNormalizedValue();
        case (reverbDecayID):           return reverbDecay->getCurrentNormalizedValue();
        case (reverbDuckID):            return reverbDuck->getCurrentNormalizedValue();
        case (reverbLoCutID):           return reverbLoCut->getCurrentNormalizedValue();
        case (reverbHiCutID):           return reverbHiCut->getCurrentNormalizedValue();
        case (numVoicesID):             return numVoices->getCurrentNormalizedValue();
        case (inputSourceID):           return inputSource->getCurrentNormalizedValue();
        case (vocalRangeTypeID):        return vocalRangeTypeToFloatParam (vocalRangeType->getCurrentChoiceName());
            
        // message types that aren't actually automated parameters
        case (pitchbendFromEditorID):   return 0.5f;
        case (killAllMidiID):           return 0.0f;
        
        // specially keyed/converted values
        case (midiLatchID):
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched() ? 1.0f : 0.0f;
            else return floatEngine.isMidiLatched() ? 1.0f : 0.0f;
    }
}


float ImogenAudioProcessor::getDefaultParameterValue (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return 0.0f; // from the plugin's point of view, the default state for the main bypass is false (not bypassed)
        case (leadBypassID):            return leadBypass->getNormalizedDefault();
        case (harmonyBypassID):         return harmonyBypass->getNormalizedDefault();
        case (dryPanID):                return dryPan->getNormalizedDefault();
        case (dryWetID):                return dryWet->getNormalizedDefault();
        case (adsrAttackID):            return adsrAttack->getNormalizedDefault();
        case (adsrDecayID):             return adsrDecay->getNormalizedDefault();
        case (adsrSustainID):           return adsrSustain->getNormalizedDefault();
        case (adsrReleaseID):           return adsrRelease->getNormalizedDefault();
        case (adsrToggleID):            return adsrToggle->getNormalizedDefault();
        case (stereoWidthID):           return stereoWidth->getNormalizedDefault();
        case (lowestPannedID):          return lowestPanned->getNormalizedDefault();
        case (velocitySensID):          return velocitySens->getNormalizedDefault();
        case (pitchBendRangeID):        return pitchBendRange->getNormalizedDefault();
        case (pedalPitchIsOnID):        return pedalPitchIsOn->getNormalizedDefault();
        case (pedalPitchThreshID):      return pedalPitchThresh->getNormalizedDefault();
        case (pedalPitchIntervalID):    return pedalPitchInterval->getNormalizedDefault();
        case (descantIsOnID):           return descantIsOn->getNormalizedDefault();
        case (descantThreshID):         return descantThresh->getNormalizedDefault();
        case (descantIntervalID):       return descantInterval->getNormalizedDefault();
        case (concertPitchHzID):        return concertPitchHz->getNormalizedDefault();
        case (voiceStealingID):         return voiceStealing->getNormalizedDefault();
        case (inputGainID):             return inputGain->getNormalizedDefault();
        case (outputGainID):            return outputGain->getNormalizedDefault();
        case (limiterToggleID):         return limiterToggle->getNormalizedDefault();
        case (noiseGateToggleID):       return noiseGateToggle->getNormalizedDefault();
        case (noiseGateThresholdID):    return noiseGateThreshold->getNormalizedDefault();
        case (compressorToggleID):      return compressorToggle->getNormalizedDefault();
        case (compressorAmountID):      return compressorAmount->getNormalizedDefault();
        case (aftertouchGainToggleID):  return aftertouchGainToggle->getNormalizedDefault();
        case (deEsserToggleID):         return deEsserToggle->getCurrentNormalizedValue();
        case (deEsserThreshID):         return deEsserThresh->getNormalizedDefault();
        case (deEsserAmountID):         return deEsserAmount->getNormalizedDefault();
        case (reverbToggleID):          return reverbToggle->getNormalizedDefault();
        case (reverbDryWetID):          return reverbDryWet->getNormalizedDefault();
        case (reverbDecayID):           return reverbDecay->getNormalizedDefault();
        case (reverbDuckID):            return reverbDuck->getNormalizedDefault();
        case (reverbLoCutID):           return reverbLoCut->getNormalizedDefault();
        case (reverbHiCutID):           return reverbHiCut->getNormalizedDefault();
        case (vocalRangeTypeID):        return vocalRangeTypeToFloatParam (vocalRangeTypes[defaultVocalRangeIndex.load()]);
        case (numVoicesID):             return numVoices->getNormalizedDefault();
        case (inputSourceID):           return inputSource->getNormalizedDefault();
        case (killAllMidiID):           return 0.0f;
        case (pitchbendFromEditorID):   return 0.5f;
        
        case (midiLatchID):  // for midi latch, its state should never be changed by a query for a "default"
            if (isUsingDoublePrecision()) return doubleEngine.isMidiLatched() ? 1.0f : 0.0f;
            else return floatEngine.isMidiLatched() ? 1.0f : 0.0f;
    }
}


const juce::NormalisableRange<float>& ImogenAudioProcessor::getParameterRange (const parameterIDs paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass->getNormalisableRange();
        case (leadBypassID):            return leadBypass->getNormalisableRange();
        case (harmonyBypassID):         return harmonyBypass->getNormalisableRange();
        case (dryPanID):                return dryPan->getNormalisableRange();
        case (dryWetID):                return dryWet->getNormalisableRange();
        case (adsrAttackID):            return adsrAttack->getNormalisableRange();
        case (adsrDecayID):             return adsrDecay->getNormalisableRange();
        case (adsrSustainID):           return adsrSustain->getNormalisableRange();
        case (adsrReleaseID):           return adsrRelease->getNormalisableRange();
        case (adsrToggleID):            return adsrToggle->getNormalisableRange();
        case (stereoWidthID):           return stereoWidth->getNormalisableRange();
        case (lowestPannedID):          return lowestPanned->getNormalisableRange();
        case (velocitySensID):          return velocitySens->getNormalisableRange();
        case (pitchBendRangeID):        return pitchBendRange->getNormalisableRange();
        case (pedalPitchIsOnID):        return pedalPitchIsOn->getNormalisableRange();
        case (pedalPitchThreshID):      return pedalPitchThresh->getNormalisableRange();
        case (pedalPitchIntervalID):    return pedalPitchInterval->getNormalisableRange();
        case (descantIsOnID):           return descantIsOn->getNormalisableRange();
        case (descantThreshID):         return descantThresh->getNormalisableRange();
        case (descantIntervalID):       return descantInterval->getNormalisableRange();
        case (concertPitchHzID):        return concertPitchHz->getNormalisableRange();
        case (voiceStealingID):         return voiceStealing->getNormalisableRange();
        case (inputGainID):             return inputGain->getNormalisableRange();
        case (outputGainID):            return outputGain->getNormalisableRange();
        case (limiterToggleID):         return limiterToggle->getNormalisableRange();
        case (noiseGateToggleID):       return noiseGateToggle->getNormalisableRange();
        case (noiseGateThresholdID):    return noiseGateThreshold->getNormalisableRange();
        case (compressorToggleID):      return compressorToggle->getNormalisableRange();
        case (compressorAmountID):      return compressorAmount->getNormalisableRange();
        case (aftertouchGainToggleID):  return aftertouchGainToggle->getNormalisableRange();
        case (deEsserToggleID):         return deEsserToggle->getNormalisableRange();
        case (deEsserThreshID):         return deEsserThresh->getNormalisableRange();
        case (deEsserAmountID):         return deEsserAmount->getNormalisableRange();
        case (reverbToggleID):          return reverbToggle->getNormalisableRange();
        case (reverbDryWetID):          return reverbDryWet->getNormalisableRange();
        case (reverbDecayID):           return reverbDecay->getNormalisableRange();
        case (reverbDuckID):            return reverbDuck->getNormalisableRange();
        case (reverbLoCutID):           return reverbLoCut->getNormalisableRange();
        case (reverbHiCutID):           return reverbHiCut->getNormalisableRange();
        case (vocalRangeTypeID):        return vocalRangeType->getNormalisableRange();
        case (numVoicesID):             return numVoices->getNormalisableRange();
        case (inputSourceID):           return inputSource->getNormalisableRange();
        case (pitchbendFromEditorID):   return pitchBendNormRange;
            
        // these are both non-automatable boolean triggers, so:
        case (midiLatchID):             return leadBypass->getNormalisableRange();
        case (killAllMidiID):           return leadBypass->getNormalisableRange();
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
