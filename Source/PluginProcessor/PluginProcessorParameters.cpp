
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 PluginProcessorParameters.cpp: This file contains functions dealing with parameter creation, management, and updating.
 
======================================================================================================================================================*/


#include "PluginProcessor.h"


template <typename SampleType>
void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<SampleType>& engine)
{
    // these functions will all be called with the current denormalized value as a float -- so just static_cast to int, etc
    getParameterPntr(inputSourceID)->actionableFunction   = [&engine](float value) { engine.setModulatorSource (juce::roundToInt (value)); };
    getParameterPntr(mainBypassID)->actionableFunction    = [&engine](float value) { engine.updateLeadBypass (value >= 0.5f); };
    getParameterPntr(leadBypassID)->actionableFunction    = [&engine](float value) { engine.updateLeadBypass (value >= 0.5f); };
    getParameterPntr(harmonyBypassID)->actionableFunction = [&engine](float value) { engine.updateHarmonyBypass (value >= 0.5f); };
    getParameterPntr(dryPanID)->actionableFunction = [&engine](float value) { engine.updateDryVoxPan (juce::roundToInt (value)); };
    getParameterPntr(adsrAttackID)->actionableFunction  = [&engine](float value) { engine.updateAdsrAttack (value); };
    getParameterPntr(adsrDecayID)->actionableFunction   = [&engine](float value) { engine.updateAdsrDecay (value); };
    getParameterPntr(adsrSustainID)->actionableFunction = [&engine](float value) { engine.updateAdsrSustain (value); };
    getParameterPntr(adsrReleaseID)->actionableFunction = [&engine](float value) { engine.updateAdsrRelease (value); };
    getParameterPntr(stereoWidthID)->actionableFunction = [&engine](float value) { engine.updateStereoWidth (juce::roundToInt (value)); };
    getParameterPntr(lowestPannedID)->actionableFunction = [&engine](float value) { engine.updateLowestPannedNote (juce::roundToInt (value)); };
    getParameterPntr(velocitySensID)->actionableFunction = [&engine](float value) { engine.updateMidiVelocitySensitivity(juce::roundToInt(value)); };
    getParameterPntr(pitchBendRangeID)->actionableFunction     = [&engine](float value) { engine.updatePitchbendRange (juce::roundToInt (value)); };
    getParameterPntr(pedalPitchIsOnID)->actionableFunction     = [&engine](float value) { engine.updatePedalToggle (value >= 0.5f); };
    getParameterPntr(pedalPitchThreshID)->actionableFunction   = [&engine](float value) { engine.updatePedalThresh (juce::roundToInt (value)); };
    getParameterPntr(pedalPitchIntervalID)->actionableFunction = [&engine](float value) { engine.updatePedalInterval (juce::roundToInt (value)); };
    getParameterPntr(descantIsOnID)->actionableFunction        = [&engine](float value) { engine.updateDescantToggle (value >= 0.5f); };
    getParameterPntr(descantThreshID)->actionableFunction      = [&engine](float value) { engine.updateDescantThresh (juce::roundToInt (value)); };
    getParameterPntr(descantIntervalID)->actionableFunction    = [&engine](float value) { engine.updateDescantInterval (juce::roundToInt (value)); };
    getParameterPntr(voiceStealingID)->actionableFunction      = [&engine](float value) { engine.updateNoteStealing (value >= 0.5f); };
    getParameterPntr(inputGainID)->actionableFunction          = [&engine](float value) { engine.updateInputGain (value); };
    getParameterPntr(outputGainID)->actionableFunction         = [&engine](float value) { engine.updateOutputGain (value); };
    getParameterPntr(limiterToggleID)->actionableFunction      = [&engine](float value) { engine.updateLimiter (value >= 0.5f); };
    getParameterPntr(noiseGateToggleID)->actionableFunction    = [&engine](float value) { engine.updateNoiseGateToggle (value >= 0.5f); };
    getParameterPntr(noiseGateThresholdID)->actionableFunction = [&engine](float value) { engine.updateNoiseGateThresh (value); };
    getParameterPntr(compressorToggleID)->actionableFunction   = [&engine](float value) { engine.updateCompressorToggle (value >= 0.5f); };
    getParameterPntr(compressorAmountID)->actionableFunction   = [&engine](float value) { engine.updateCompressorAmount (value); };
    getParameterPntr(aftertouchGainToggleID)->actionableFunction = [&engine](float value) { engine.updateAftertouchGainOnOff (value >= 0.5f); };
    getParameterPntr(deEsserToggleID)->actionableFunction = [&engine](float value) { engine.updateDeEsserToggle (value >= 0.5f); };
    getParameterPntr(deEsserThreshID)->actionableFunction = [&engine](float value) { engine.updateDeEsserThresh (value); };
    getParameterPntr(deEsserAmountID)->actionableFunction = [&engine](float value) { engine.updateDeEsserAmount (value); };
    getParameterPntr(reverbToggleID)->actionableFunction  = [&engine](float value) { engine.updateReverbToggle (value >= 0.5f); };
    getParameterPntr(reverbDryWetID)->actionableFunction  = [&engine](float value) { engine.updateReverbDryWet (juce::roundToInt (value)); };
    getParameterPntr(reverbDecayID)->actionableFunction   = [&engine](float value) { engine.updateReverbDecay (value); };
    getParameterPntr(reverbDuckID)->actionableFunction    = [&engine](float value) { engine.updateReverbDuck (value); };
    getParameterPntr(reverbLoCutID)->actionableFunction   = [&engine](float value) { engine.updateReverbLoCut (value); };
    getParameterPntr(reverbHiCutID)->actionableFunction   = [&engine](float value) { engine.updateReverbHiCut (value); };
    getParameterPntr(delayToggleID)->actionableFunction   = [&engine](float value) { engine.updateDelayToggle (value >= 0.5f); };
    getParameterPntr(delayDryWetID)->actionableFunction   = [&engine](float value) { engine.updateDelayDryWet (juce::roundToInt (value)); };
}
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<float>& engine);
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<double>& engine);

/*===========================================================================================================================
    Returns one of the processor's parameter objects, referenced by its parameterID.
 ============================================================================================================================*/

inline bav::Parameter* ImogenAudioProcessor::getParameterPntr (const ParameterID paramID) const
{
    for (auto* pntr : parameterPointers)
        if (static_cast<ParameterID>(pntr->key()) == paramID)
            return pntr;
    
    return nullptr;
}


inline bav::Parameter* ImogenAudioProcessor::getMeterParamPntr (const MeterID meterID) const
{
    for (auto* pntr : meterParameterPointers)
        if (static_cast<MeterID>(pntr->key()) == meterID)
            return pntr;
    
    return nullptr;
}


/*===========================================================================================================================
    Processes all the non-parameter events in the message queue.
 ============================================================================================================================*/

template<typename SampleType>
inline void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine)
{
    nonParamEvents.getReadyMessages (currentMessages);
    
    for (const auto& msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        const auto value = msg.value();
        
        jassert (value >= 0.0f && value <= 1.0f);
        
        switch (msg.type())
        {
            default: continue;
            case (killAllMidiID): activeEngine.killAllMidi();  // any message of this type triggers this, regardless of its value
            case (midiLatchID):   changeMidiLatchState (value >= 0.5f);
            case (pitchBendFromEditorID): activeEngine.recieveExternalPitchbend (juce::roundToInt (pitchbendNormalizedRange.convertFrom0to1 (value)));
        }
    }
}
///function template instantiations...
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<float>& activeEngine);
template void ImogenAudioProcessor::processQueuedNonParamEvents (bav::ImogenEngine<double>& activeEngine);

