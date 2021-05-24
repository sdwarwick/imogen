
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


template < typename SampleType >
void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< SampleType >& engine)
{
    using namespace Imogen;

//    getParameterPntr (adsrAttackID)->setAction ([&engine] (float value) { engine.updateAdsrAttack (value); });
//    getParameterPntr (adsrDecayID)->setAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
//    getParameterPntr (adsrDecayID)->setAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
//    getParameterPntr (adsrSustainID)->setAction ([&engine] (float value) { engine.updateAdsrSustain (value); });
//    getParameterPntr (adsrReleaseID)->setAction ([&engine] (float value) { engine.updateAdsrRelease (value); });
//    getParameterPntr (inputGainID)->setAction ([&engine] (float value) { engine.updateInputGain (value); });
//    getParameterPntr (outputGainID)->setAction ([&engine] (float value) { engine.updateOutputGain (value); });
//    getParameterPntr (noiseGateThresholdID)->setAction ([&engine] (float value) { engine.updateNoiseGateThresh (value); });
//    getParameterPntr (compressorAmountID)->setAction ([&engine] (float value) { engine.updateCompressorAmount (value); });
//    getParameterPntr (deEsserThreshID)->setAction ([&engine] (float value) { engine.updateDeEsserThresh (value); });
//    getParameterPntr (deEsserAmountID)->setAction ([&engine] (float value) { engine.updateDeEsserAmount (value); });
//    getParameterPntr (reverbDecayID)->setAction ([&engine] (float value) { engine.updateReverbDecay (value); });
//    getParameterPntr (reverbDuckID)->setAction ([&engine] (float value) { engine.updateReverbDuck (value); });
//    getParameterPntr (reverbLoCutID)->setAction ([&engine] (float value) { engine.updateReverbLoCut (value); });
//    getParameterPntr (reverbHiCutID)->setAction ([&engine] (float value) { engine.updateReverbHiCut (value); });
//
//    getParameterPntr (inputSourceID)->setAction ([&engine] (int value) { engine.setModulatorSource (value); });
//    getParameterPntr (dryPanID)->setAction ([&engine] (int value) { engine.updateDryVoxPan (value); });
//    getParameterPntr (stereoWidthID)->setAction ([&engine] (int value) { engine.updateStereoWidth (value); });
//    getParameterPntr (lowestPannedID)->setAction ([&engine] (int value) { engine.updateLowestPannedNote (value); });
//    getParameterPntr (velocitySensID)->setAction ([&engine] (int value) { engine.updateMidiVelocitySensitivity (value); });
//    getParameterPntr (pitchBendRangeID)->setAction ([&engine] (int value) { engine.updatePitchbendRange (value); });
//    getParameterPntr (pedalPitchThreshID)->setAction ([&engine] (int value) { engine.updatePedalThresh (value); });
//    getParameterPntr (pedalPitchIntervalID)->setAction ([&engine] (int value) { engine.updatePedalInterval (value); });
//    getParameterPntr (descantThreshID)->setAction ([&engine] (int value) { engine.updateDescantThresh (value); });
//    getParameterPntr (descantIntervalID)->setAction ([&engine] (int value) { engine.updateDescantInterval (value); });
//    getParameterPntr (reverbDryWetID)->setAction ([&engine] (int value) { engine.updateReverbDryWet (value); });
//    getParameterPntr (delayDryWetID)->setAction ([&engine] (int value) { engine.updateDelayDryWet (value); });
//
//    getParameterPntr (leadBypassID)->setAction ([&engine] (bool value) { engine.updateLeadBypass (value); });
//    getParameterPntr (harmonyBypassID)->setAction ([&engine] (bool value) { engine.updateHarmonyBypass (value); });
//    getParameterPntr (pedalPitchIsOnID)->setAction ([&engine] (bool value) { engine.updatePedalToggle (value); });
//    getParameterPntr (descantIsOnID)->setAction ([&engine] (bool value) { engine.updateDescantToggle (value); });
//    getParameterPntr (voiceStealingID)->setAction ([&engine] (bool value) { engine.updateNoteStealing (value); });
//    getParameterPntr (limiterToggleID)->setAction ([&engine] (bool value) { engine.updateLimiter (value); });
//    getParameterPntr (noiseGateToggleID)->setAction ([&engine] (bool value) { engine.updateNoiseGateToggle (value); });
//    getParameterPntr (compressorToggleID)->setAction ([&engine] (bool value) { engine.updateCompressorToggle (value); });
//    getParameterPntr (aftertouchGainToggleID)->setAction ([&engine] (bool value) { engine.updateAftertouchGainOnOff (value); });
//    getParameterPntr (deEsserToggleID)->setAction ([&engine] (bool value) { engine.updateDeEsserToggle (value); });
//    getParameterPntr (reverbToggleID)->setAction ([&engine] (bool value) { engine.updateReverbToggle (value); });
//    getParameterPntr (delayToggleID)->setAction ([&engine] (bool value) { engine.updateDelayToggle (value); });
}
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< float >&);
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< double >&);


/*===========================================================================================================================
 ============================================================================================================================*/

bav::Parameter* ImogenAudioProcessor::getParameterPntr (const ParameterID paramID) const
{
    for (auto* pntr : parameterPointers)
        if (static_cast< ParameterID > (pntr->key) == paramID) return pntr;

    return nullptr;
}


inline bav::Parameter* ImogenAudioProcessor::getMeterParamPntr (const MeterID meterID) const
{
    for (auto* pntr : meterParameterPointers)
        if (static_cast< MeterID > (pntr->key) == meterID) return pntr;

    return nullptr;
}

