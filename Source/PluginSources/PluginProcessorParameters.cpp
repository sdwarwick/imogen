
#include "PluginProcessor.h"

///  This file contains Imogen's parameter management and event handling system.  ///


/*
    Functions for state saving and loading
*/


// functions for saving state info.....................................

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml = tree.copyState().createXml();
    copyXmlToBinary (*xml, destData);
}

void ImogenAudioProcessor::savePreset (juce::String presetName) // this function can be used both to save new preset files or to update existing ones
{
    std::unique_ptr<juce::XmlElement> xml = tree.copyState().createXml();
    
    xml->setAttribute ("presetName", presetName);
    
    presetName += ".xml";
    
    xml->writeTo (getPresetsFolder().getChildFile (presetName));
    updateHostDisplay();
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
    
    updateAllParameters (activeEngine);
    
    updateParameterDefaults();
    
    suspendProcessing (false);
    
    updateHostDisplay();
    return true;
}


void ImogenAudioProcessor::deletePreset (juce::String presetName)
{
    presetName += ".xml";
    
    juce::File presetToDelete = getPresetsFolder().getChildFile (presetName);
    
    if (presetToDelete.existsAsFile())
        if (! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    
    updateHostDisplay();
}


/*
    Functions for managing "programs".
*/

int ImogenAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
}

int ImogenAudioProcessor::getCurrentProgram()
{
    return 1;
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    ignoreUnused (index, newName);
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Functions for controlling individual parameters (those that require their own functions in this class)
*/


// updates the vocal input range type. This controls the pitch detection Hz range, and, thus, the latency of the pitch detector and the latency of the entire plugin. The lower the min possible Hz for the pitch detector, the higher the plugin's latency.
void ImogenAudioProcessor::updateVocalRangeType (int newRangeType)
{
    jassert (newRangeType >= 0 && newRangeType <= 3);
    
    int minHz, maxHz;
    
    switch (newRangeType)
    {
        default:
            minHz = bav::math::midiToFreq (57);
            maxHz = bav::math::midiToFreq (88);
        case (1):
            minHz = bav::math::midiToFreq (50);
            maxHz = bav::math::midiToFreq (81);
        case (2):
            minHz = bav::math::midiToFreq (43);
            maxHz = bav::math::midiToFreq (76);
        case (3):
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


juce::String ImogenAudioProcessor::getCurrentVocalRange() const
{
    switch (vocalRangeType->get())
    {
        default:  return { "Soprano" };
        case (1): return { "Alto" };
        case (2): return { "Tenor" };
        case (3): return { "Bass" };
    }
}


// converts the "compressor-knob" value to threshold and ratio control values and passes these to the ImogenEngine
template<typename SampleType>
void ImogenAudioProcessor::updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                                             bool compressorIsOn, float knobValue)
{
    jassert (knobValue >= 0.0f && knobValue <= 1.0f);
    
    activeEngine.updateCompressor (juce::jmap (knobValue, 0.0f, -60.0f),  // threshold (dB)
                                   juce::jmap (knobValue, 1.0f, 10.0f),  // ratio
                                   compressorIsOn);
}


// update the number of concurrently running instances of the harmony algorithm
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



/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Functions for updating all parameters / changes
*/


// refreshes all parameter values, without consulting the FIFO message queue.
template<typename SampleType>
void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine)
{
    updateVocalRangeType (vocalRangeType->get());
    updateNumVoices (numVoices->get());
    
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
    
    activeEngine.setModulatorSource (inputSource->get());
}


// reads all available messages from the FIFO queue, and processes the most recent of each type.
template<typename SampleType>
void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine)
{
    paramChangesForProcessor.getReadyMessages (currentMessages, false);
    
    // converts a message's value to a boolean true/false
#define _BOOL_MSG_ msg.value() >= 0.5f
    
    // converts a message's value to a float using the normalisable range of the specified parameter
#define _FLOAT_MSG_(p) p->getNormalisableRange().convertFrom0to1(msg.value())
    
    // converts a message's value to an integer using the normalisable range of the specified parameter
#define _INT_MSG_(p) juce::roundToInt (_FLOAT_MSG_(p))
    
    bool adsr = false;
    float adsrA = adsrAttack->get(), adsrD = adsrDecay->get(), adsrS = adsrSustain->get(), adsrR = adsrRelease->get();
    bool adsrT = adsrToggle->get();
    
    bool reverb = false;
    int rDryWet = reverbDryWet->get();
    float rDecay = reverbDecay->get(), rDuck = reverbDuck->get(), rLoCut = reverbLoCut->get(), rHiCut = reverbHiCut->get();
    bool rToggle = reverbToggle->get();
    
    for (const auto msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        jassert (msg.value() >= 0.0f && msg.value() <= 1.0f);
        
        switch (msg.type())
        {
            default:             continue;
            case (mainBypassID): continue;

            case (adsrAttackID):  adsrA = _FLOAT_MSG_(adsrAttack);  adsr = true;
            case (adsrDecayID):   adsrD = _FLOAT_MSG_(adsrDecay);   adsr = true;
            case (adsrSustainID): adsrS = _FLOAT_MSG_(adsrSustain); adsr = true;
            case (adsrReleaseID): adsrR = _FLOAT_MSG_(adsrRelease); adsr = true;
            case (adsrToggleID):  adsrT = _BOOL_MSG_; adsr = true;
                
            case (leadBypassID):     activeEngine.updateBypassStates (_BOOL_MSG_, harmonyBypass->get());
            case (harmonyBypassID):  activeEngine.updateBypassStates (leadBypass->get(), _BOOL_MSG_);
            case (dryPanID):         activeEngine.updateDryVoxPan (_INT_MSG_(dryPan));
            case (dryWetID):         activeEngine.updateDryWet (_INT_MSG_(dryWet));
            case (stereoWidthID):    activeEngine.updateStereoWidth (_INT_MSG_(stereoWidth), lowestPanned->get());
            case (lowestPannedID):   activeEngine.updateStereoWidth (stereoWidth->get(), lowestPanned->get());
            case (velocitySensID):   activeEngine.updateMidiVelocitySensitivity (_INT_MSG_(velocitySens));
            case (pitchBendRangeID): activeEngine.updatePitchbendRange (_INT_MSG_(pitchBendRange));
                
            case (pedalPitchIsOnID):     activeEngine.updatePedalPitch (_BOOL_MSG_, pedalPitchThresh->get(), pedalPitchInterval->get());
            case (pedalPitchThreshID):   activeEngine.updatePedalPitch (pedalPitchIsOn->get(), _INT_MSG_(pedalPitchThresh), pedalPitchInterval->get());
            case (pedalPitchIntervalID): activeEngine.updatePedalPitch (pedalPitchIsOn->get(), pedalPitchThresh->get(), _INT_MSG_(pedalPitchInterval));
                
            case (descantIsOnID):     activeEngine.updateDescant (_BOOL_MSG_, descantThresh->get(), descantInterval->get());
            case (descantThreshID):   activeEngine.updateDescant (descantIsOn->get(), _INT_MSG_(descantThresh), descantInterval->get());
            case (descantIntervalID): activeEngine.updateDescant (descantIsOn->get(), descantThresh->get(), _INT_MSG_(descantInterval));
                
            case (concertPitchHzID):        activeEngine.updateConcertPitch (_INT_MSG_(concertPitchHz));
            case (voiceStealingID):         activeEngine.updateNoteStealing (_BOOL_MSG_);
            case (inputGainID):             activeEngine.updateInputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG_(inputGain)));
            case (outputGainID):            activeEngine.updateOutputGain (juce::Decibels::decibelsToGain (_FLOAT_MSG_(outputGain)));
            case (limiterToggleID):         activeEngine.updateLimiter (_BOOL_MSG_);
            case (noiseGateToggleID):       activeEngine.updateNoiseGate (noiseGateThreshold->get(), _BOOL_MSG_);
            case (noiseGateThresholdID):    activeEngine.updateNoiseGate (_FLOAT_MSG_(noiseGateThreshold), noiseGateToggle->get());
            case (compressorToggleID):      updateCompressor (activeEngine, _BOOL_MSG_, compressorAmount->get());
            case (compressorAmountID):      updateCompressor (activeEngine, compressorToggle->get(), _FLOAT_MSG_(compressorAmount));
            case (vocalRangeTypeID):        updateVocalRangeType (_INT_MSG_(vocalRangeType));
            case (aftertouchGainToggleID):  activeEngine.updateAftertouchGainOnOff (_BOOL_MSG_);
            case (deEsserToggleID):         activeEngine.updateDeEsser (deEsserAmount->get(), deEsserThresh->get(), _BOOL_MSG_);
            case (deEsserThreshID):         activeEngine.updateDeEsser (deEsserAmount->get(), _FLOAT_MSG_(deEsserThresh), deEsserToggle->get());
            case (deEsserAmountID):         activeEngine.updateDeEsser (_FLOAT_MSG_(deEsserAmount), deEsserThresh->get(), deEsserToggle->get());
                
            case (reverbToggleID): rToggle = _BOOL_MSG_; reverb = true;
            case (reverbDryWetID): rDryWet = _INT_MSG_(reverbDryWet);  reverb = true;
            case (reverbDecayID):  rDecay  = _FLOAT_MSG_(reverbDecay); reverb = true;
            case (reverbDuckID):   rDuck   = _FLOAT_MSG_(reverbDuck);  reverb = true;
            case (reverbLoCutID):  rLoCut  = _FLOAT_MSG_(reverbLoCut); reverb = true;
            case (reverbHiCutID):  rHiCut  = _FLOAT_MSG_(reverbHiCut); reverb = true;
                
            case (inputSourceID):  activeEngine.setModulatorSource (_INT_MSG_(inputSource));
            case (numVoicesID):    updateNumVoices (_INT_MSG_(numVoices));
        }
    }
    
    if (adsr)
        activeEngine.updateAdsr (adsrA, adsrD, adsrS, adsrR, adsrT);
    
    if (reverb)
        activeEngine.updateReverb (rDryWet, rDecay, rDuck, rLoCut, rHiCut, rToggle);
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedParameterChanges (bav::ImogenEngine<double>& activeEngine);

#undef _BOOL_MSG_
#undef _INT_MSG_
#undef _FLOAT_MSG_


template<typename SampleType>
void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine)
{
    nonParamEvents.getReadyMessages (currentMessages, true);
    
    for (const auto msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        jassert (msg.value() >= 0.0f && msg.value() <= 1.0f);
        
        switch (msg.type())
        {
            default: continue;
            case (killAllMidi): activeEngine.killAllMidi();
            case (midiLatch):   activeEngine.updateMidiLatch (msg.value() >= 0.5f);
            case (pitchBendFromEditor): activeEngine.recieveExternalPitchbend (juce::roundToInt (juce::jmap (msg.value(), 0.0f, 127.0f)));
        }
    }
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<double>& activeEngine);


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Functions for retrieving parameter values and other attributes.
    These functions may be called by the processor or by the editor.
*/


// Returns the requested parameter's current value, in the noramlized range 0.0f to 1.0f
float ImogenAudioProcessor::getCurrentParameterValue (const parameterID paramID) const
{
    return getParameterPntr(paramID)->getCurrentNormalizedValue();
}


// Returns the requested parameter's default value, in the normalized range 0.0f to 1.0f
float ImogenAudioProcessor::getDefaultParameterValue (const parameterID paramID) const
{
    return getParameterPntr(paramID)->getNormalizedDefault();
}


// Returns a const reference to the requested parameter's NormalisableRange object
const juce::NormalisableRange<float>& ImogenAudioProcessor::getParameterRange (const parameterID paramID) const
{
    return getParameterPntr(paramID)->getRange();
}


// This function reassigns each parameter's internally stored default value to the parameter's current value. Run this function after loading a preset, etc.
void ImogenAudioProcessor::updateParameterDefaults()
{
    for (int paramID = IMGN_FIRST_PARAM; paramID <= IMGN_LAST_PARAM; ++paramID)
        getParameterPntr(parameterID(paramID))->refreshDefault();
    
    parameterDefaultsAreDirty.store (true);
}


// Tracks whether or not the processor has updated its default parameter values since the last call to this function.
bool ImogenAudioProcessor::hasUpdatedParamDefaults()
{
    const bool hasUpdated = parameterDefaultsAreDirty.load();
    parameterDefaultsAreDirty.store (false);
    return hasUpdated;
}


/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Functions for basic parameter set up & creation
*/


// creates all the needed parameter objects and returns them in a ParameterLayout
juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
    
    params.emplace_back (std::make_unique<BoolParameter>  ("mainBypass", "Bypass", false));
    params.emplace_back (std::make_unique<BoolParameter>  ("leadBypass", "Lead bypass", false));
    params.emplace_back (std::make_unique<BoolParameter>  ("harmonyBypass", "Harmony bypass", false));
    params.emplace_back (std::make_unique<IntParameter>   ("numVoices", "Number of voices", 1, 20, 12));
    params.emplace_back (std::make_unique<IntParameter>   ("inputSource", "Input source", 1, 3, 1));
    params.emplace_back (std::make_unique<IntParameter>   ("dryPan", "Dry vox pan", 0, 127, 64));
    params.emplace_back (std::make_unique<FloatParameter> ("adsrAttack", "ADSR Attack", msRange, 0.35f));
    params.emplace_back (std::make_unique<FloatParameter> ("adsrDecay", "ADSR Decay", msRange, 0.06f));
    params.emplace_back (std::make_unique<FloatParameter> ("adsrSustain", "ADSR Sustain", zeroToOneRange, 0.8f));
    params.emplace_back (std::make_unique<FloatParameter> ("adsrRelease", "ADSR Release", msRange, 0.1f));
    params.emplace_back (std::make_unique<BoolParameter>  ("adsrOnOff", "ADSR on/off", true));
    params.emplace_back (std::make_unique<IntParameter>   ("stereoWidth", "Stereo Width", 0, 100, 100));
    params.emplace_back (std::make_unique<IntParameter>   ("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    params.emplace_back (std::make_unique<IntParameter>   ("midiVelocitySens", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.emplace_back (std::make_unique<IntParameter>   ("PitchBendRange", "Pitch bend range (st)", 0, 12, 2));
    params.emplace_back (std::make_unique<IntParameter>   ("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    params.emplace_back (std::make_unique<BoolParameter>  ("voiceStealing", "Voice stealing", false));
    params.emplace_back (std::make_unique<BoolParameter>  ("aftertouchGainToggle", "Aftertouch gain on/off", true));
    params.emplace_back (std::make_unique<BoolParameter>  ("pedalPitchToggle", "Pedal pitch on/off", false));
    params.emplace_back (std::make_unique<IntParameter>   ("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.emplace_back (std::make_unique<IntParameter>   ("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    params.emplace_back (std::make_unique<BoolParameter>  ("descantToggle", "Descant on/off", false));
    params.emplace_back (std::make_unique<IntParameter>   ("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.emplace_back (std::make_unique<IntParameter>   ("descantInterval", "Descant interval", 1, 12, 12));
    params.emplace_back (std::make_unique<IntParameter>   ("masterDryWet", "% wet", 0, 100, 100));
    params.emplace_back (std::make_unique<FloatParameter> ("inputGain", "Input gain",   gainRange, 0.0f));
    params.emplace_back (std::make_unique<FloatParameter> ("outputGain", "Output gain", gainRange, -4.0f));
    params.emplace_back (std::make_unique<BoolParameter>  ("limiterIsOn", "Limiter on/off", true));
    params.emplace_back (std::make_unique<BoolParameter>  ("noiseGateIsOn", "Noise gate toggle", true));
    params.emplace_back (std::make_unique<FloatParameter> ("noiseGateThresh", "Noise gate threshold", gainRange, -20.0f));
    params.emplace_back (std::make_unique<BoolParameter>  ("deEsserIsOn", "De-esser toggle", true));
    params.emplace_back (std::make_unique<FloatParameter> ("deEsserThresh", "De-esser thresh", gainRange, -6.0f));
    params.emplace_back (std::make_unique<FloatParameter> ("deEsserAmount", "De-esser amount", zeroToOneRange, 0.5f));
    params.emplace_back (std::make_unique<BoolParameter>  ("compressorToggle", "Compressor on/off", false));
    params.emplace_back (std::make_unique<FloatParameter> ("compressorAmount", "Compressor amount", zeroToOneRange, 0.35f));
    params.emplace_back (std::make_unique<BoolParameter>  ("reverbIsOn", "Reverb toggle", false));
    params.emplace_back (std::make_unique<IntParameter>   ("reverbDryWet", "Reverb dry/wet", 0, 100, 35));
    params.emplace_back (std::make_unique<FloatParameter> ("reverbDecay", "Reverb decay", zeroToOneRange, 0.6f));
    params.emplace_back (std::make_unique<FloatParameter> ("reverbDuck", "Duck amount", zeroToOneRange, 0.3f));
    params.emplace_back (std::make_unique<FloatParameter> ("reverbLoCut", "Reverb low cut", hzRange, 80.0f));
    params.emplace_back (std::make_unique<FloatParameter> ("reverbHiCut", "Reverb high cut", hzRange, 5500.0f));
    params.emplace_back (std::make_unique<IntParameter>   ("vocalRangeType", "Input vocal range", 0, 3, 0));
    
    return { params.begin(), params.end() };
}
#undef imogen_DEFAULT_VOCAL_RANGE_TYPE
#undef imgn_VOCAL_RANGE_TYPES


// initializes the member pointers to each actual parameter object
void ImogenAudioProcessor::initializeParameterPointers()
{
    mainBypass           = dynamic_cast<BoolParamPtr>  (tree.getParameter ("mainBypass"));                   jassert (mainBypass);
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
    velocitySens         = dynamic_cast<IntParamPtr>   (tree.getParameter ("midiVelocitySens"));             jassert (velocitySens);
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
    vocalRangeType       = dynamic_cast<IntParamPtr>   (tree.getParameter ("vocalRangeType"));               jassert (vocalRangeType);
}


// creates parameter listeners & messengers for each parameter
void ImogenAudioProcessor::initializeParameterListeners()
{
    addParameterMessenger ("mainBypass",            mainBypassID);
    addParameterMessenger ("leadBypass",            leadBypassID);
    addParameterMessenger ("harmonyBypass",         harmonyBypassID);
    addParameterMessenger ("numVoices",             numVoicesID);
    addParameterMessenger ("inputSource",           inputSourceID);
    addParameterMessenger ("dryPan",                dryPanID);
    addParameterMessenger ("masterDryWet",          dryWetID);
    addParameterMessenger ("adsrAttack",            adsrAttackID);
    addParameterMessenger ("adsrDecay",             adsrDecayID);
    addParameterMessenger ("adsrSustain",           adsrSustainID);
    addParameterMessenger ("adsrRelease",           adsrReleaseID);
    addParameterMessenger ("adsrOnOff",             adsrToggleID);
    addParameterMessenger ("stereoWidth",           stereoWidthID);
    addParameterMessenger ("lowestPan",             lowestPannedID);
    addParameterMessenger ("midiVelocitySens",      velocitySensID);
    addParameterMessenger ("PitchBendRange",        pitchBendRangeID);
    addParameterMessenger ("pedalPitchToggle",      pedalPitchIsOnID);
    addParameterMessenger ("pedalPitchThresh",      pedalPitchThreshID);
    addParameterMessenger ("pedalPitchInterval",    pedalPitchIntervalID);
    addParameterMessenger ("descantToggle",         descantIsOnID);
    addParameterMessenger ("descantThresh",         descantThreshID);
    addParameterMessenger ("descantInterval",       descantIntervalID);
    addParameterMessenger ("concertPitch",          concertPitchHzID);
    addParameterMessenger ("voiceStealing",         voiceStealingID);
    addParameterMessenger ("inputGain",             inputGainID);
    addParameterMessenger ("outputGain",            outputGainID);
    addParameterMessenger ("limiterIsOn",           limiterToggleID);
    addParameterMessenger ("noiseGateIsOn",         noiseGateToggleID);
    addParameterMessenger ("noiseGateThresh",       noiseGateThresholdID);
    addParameterMessenger ("compressorToggle",      compressorToggleID);
    addParameterMessenger ("compressorAmount",      compressorAmountID);
    addParameterMessenger ("vocalRangeType",        vocalRangeTypeID);
    addParameterMessenger ("aftertouchGainToggle",  aftertouchGainToggleID);
    addParameterMessenger ("deEsserIsOn",           deEsserToggleID);
    addParameterMessenger ("deEsserThresh",         deEsserThreshID);
    addParameterMessenger ("deEsserAmount",         deEsserAmountID);
    addParameterMessenger ("reverbIsOn",            reverbToggleID);
    addParameterMessenger ("reverbDryWet",          reverbDryWetID);
    addParameterMessenger ("reverbDecay",           reverbDecayID);
    addParameterMessenger ("reverbDuck",            reverbDuckID);
    addParameterMessenger ("reverbLoCut",           reverbLoCutID);
    addParameterMessenger ("reverbHiCut",           reverbHiCutID);
}


// creates a single parameter listener & messenger for a requested parameter
void ImogenAudioProcessor::addParameterMessenger (juce::String stringID, int paramID)
{
    auto& messenger { parameterMessengers.emplace_back (paramChangesForProcessor, paramChangesForEditor, paramID) };
    tree.addParameterListener (stringID, &messenger);
}



/*===========================================================================================================================
 ============================================================================================================================*/

/*
    Returns a pointer to one of the processor's parameters, indexed by its parameter ID.
*/
bav::Parameter* ImogenAudioProcessor::getParameterPntr (const parameterID paramID) const
{
    switch (paramID)
    {
        case (mainBypassID):            return mainBypass;
        case (leadBypassID):            return leadBypass;
        case (harmonyBypassID):         return harmonyBypass;
        case (dryPanID):                return dryPan;
        case (dryWetID):                return dryWet;
        case (adsrAttackID):            return adsrAttack;
        case (adsrDecayID):             return adsrDecay;
        case (adsrSustainID):           return adsrSustain;
        case (adsrReleaseID):           return adsrRelease;
        case (adsrToggleID):            return adsrToggle;
        case (stereoWidthID):           return stereoWidth;
        case (lowestPannedID):          return lowestPanned;
        case (velocitySensID):          return velocitySens;
        case (pitchBendRangeID):        return pitchBendRange;
        case (pedalPitchIsOnID):        return pedalPitchIsOn;
        case (pedalPitchThreshID):      return pedalPitchThresh;
        case (pedalPitchIntervalID):    return pedalPitchInterval;
        case (descantIsOnID):           return descantIsOn;
        case (descantThreshID):         return descantThresh;
        case (descantIntervalID):       return descantInterval;
        case (concertPitchHzID):        return concertPitchHz;
        case (voiceStealingID):         return voiceStealing;
        case (inputGainID):             return inputGain;
        case (outputGainID):            return outputGain;
        case (limiterToggleID):         return limiterToggle;
        case (noiseGateToggleID):       return noiseGateToggle;
        case (noiseGateThresholdID):    return noiseGateThreshold;
        case (compressorToggleID):      return compressorToggle;
        case (compressorAmountID):      return compressorAmount;
        case (aftertouchGainToggleID):  return aftertouchGainToggle;
        case (deEsserToggleID):         return deEsserToggle;
        case (deEsserThreshID):         return deEsserThresh;
        case (deEsserAmountID):         return deEsserAmount;
        case (reverbToggleID):          return reverbToggle;
        case (reverbDryWetID):          return reverbDryWet;
        case (reverbDecayID):           return reverbDecay;
        case (reverbDuckID):            return reverbDuck;
        case (reverbLoCutID):           return reverbLoCut;
        case (reverbHiCutID):           return reverbHiCut;
        case (numVoicesID):             return numVoices;
        case (inputSourceID):           return inputSource;
        case (vocalRangeTypeID):        return vocalRangeType;
    }
}


