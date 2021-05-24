
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

    getParameterPntr (adsrAttackID)->setFloatAction ([&engine] (float value) { engine.updateAdsrAttack (value); });
    getParameterPntr (adsrDecayID)->setFloatAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
    getParameterPntr (adsrDecayID)->setFloatAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
    getParameterPntr (adsrSustainID)->setFloatAction ([&engine] (float value) { engine.updateAdsrSustain (value); });
    getParameterPntr (adsrReleaseID)->setFloatAction ([&engine] (float value) { engine.updateAdsrRelease (value); });
    getParameterPntr (inputGainID)->setFloatAction ([&engine] (float value) { engine.updateInputGain (value); });
    getParameterPntr (outputGainID)->setFloatAction ([&engine] (float value) { engine.updateOutputGain (value); });
    getParameterPntr (noiseGateThresholdID)->setFloatAction ([&engine] (float value) { engine.updateNoiseGateThresh (value); });
    getParameterPntr (compressorAmountID)->setFloatAction ([&engine] (float value) { engine.updateCompressorAmount (value); });
    getParameterPntr (deEsserThreshID)->setFloatAction ([&engine] (float value) { engine.updateDeEsserThresh (value); });
    getParameterPntr (deEsserAmountID)->setFloatAction ([&engine] (float value) { engine.updateDeEsserAmount (value); });
    getParameterPntr (reverbDecayID)->setFloatAction ([&engine] (float value) { engine.updateReverbDecay (value); });
    getParameterPntr (reverbDuckID)->setFloatAction ([&engine] (float value) { engine.updateReverbDuck (value); });
    getParameterPntr (reverbLoCutID)->setFloatAction ([&engine] (float value) { engine.updateReverbLoCut (value); });
    getParameterPntr (reverbHiCutID)->setFloatAction ([&engine] (float value) { engine.updateReverbHiCut (value); });

    getParameterPntr (inputSourceID)->setIntAction ([&engine] (int value) { engine.setModulatorSource (value); });
    getParameterPntr (dryPanID)->setIntAction ([&engine] (int value) { engine.updateDryVoxPan (value); });
    getParameterPntr (stereoWidthID)->setIntAction ([&engine] (int value) { engine.updateStereoWidth (value); });
    getParameterPntr (lowestPannedID)->setIntAction ([&engine] (int value) { engine.updateLowestPannedNote (value); });
    getParameterPntr (velocitySensID)->setIntAction ([&engine] (int value) { engine.updateMidiVelocitySensitivity (value); });
    getParameterPntr (pitchBendRangeID)->setIntAction ([&engine] (int value) { engine.updatePitchbendRange (value); });
    getParameterPntr (pedalPitchThreshID)->setIntAction ([&engine] (int value) { engine.updatePedalThresh (value); });
    getParameterPntr (pedalPitchIntervalID)->setIntAction ([&engine] (int value) { engine.updatePedalInterval (value); });
    getParameterPntr (descantThreshID)->setIntAction ([&engine] (int value) { engine.updateDescantThresh (value); });
    getParameterPntr (descantIntervalID)->setIntAction ([&engine] (int value) { engine.updateDescantInterval (value); });
    getParameterPntr (reverbDryWetID)->setIntAction ([&engine] (int value) { engine.updateReverbDryWet (value); });
    getParameterPntr (delayDryWetID)->setIntAction ([&engine] (int value) { engine.updateDelayDryWet (value); });

    getParameterPntr (leadBypassID)->setBoolAction ([&engine] (bool value) { engine.updateLeadBypass (value); });
    getParameterPntr (harmonyBypassID)->setBoolAction ([&engine] (bool value) { engine.updateHarmonyBypass (value); });
    getParameterPntr (pedalPitchIsOnID)->setBoolAction ([&engine] (bool value) { engine.updatePedalToggle (value); });
    getParameterPntr (descantIsOnID)->setBoolAction ([&engine] (bool value) { engine.updateDescantToggle (value); });
    getParameterPntr (voiceStealingID)->setBoolAction ([&engine] (bool value) { engine.updateNoteStealing (value); });
    getParameterPntr (limiterToggleID)->setBoolAction ([&engine] (bool value) { engine.updateLimiter (value); });
    getParameterPntr (noiseGateToggleID)->setBoolAction ([&engine] (bool value) { engine.updateNoiseGateToggle (value); });
    getParameterPntr (compressorToggleID)->setBoolAction ([&engine] (bool value) { engine.updateCompressorToggle (value); });
    getParameterPntr (aftertouchGainToggleID)->setBoolAction ([&engine] (bool value) { engine.updateAftertouchGainOnOff (value); });
    getParameterPntr (deEsserToggleID)->setBoolAction ([&engine] (bool value) { engine.updateDeEsserToggle (value); });
    getParameterPntr (reverbToggleID)->setBoolAction ([&engine] (bool value) { engine.updateReverbToggle (value); });
    getParameterPntr (delayToggleID)->setBoolAction ([&engine] (bool value) { engine.updateDelayToggle (value); });
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

