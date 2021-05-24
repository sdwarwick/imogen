
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
    parameters.adsrAttack.get()->setAction ([&engine] (float value) { engine.updateAdsrAttack (value); });
    parameters.adsrDecay.get()->setAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
    parameters.adsrSustain.get()->setAction ([&engine] (float value) { engine.updateAdsrSustain (value); });
    parameters.adsrRelease.get()->setAction ([&engine] (float value) { engine.updateAdsrRelease (value); });
    parameters.inputGain.get()->setAction ([&engine] (float value) { engine.updateInputGain (value); });
    parameters.outputGain.get()->setAction ([&engine] (float value) { engine.updateOutputGain (value); });
    parameters.noiseGateThresh.get()->setAction ([&engine] (float value) { engine.updateNoiseGateThresh (value); });
    parameters.compAmount.get()->setAction ([&engine] (float value) { engine.updateCompressorAmount (value); });
    parameters.deEsserThresh.get()->setAction ([&engine] (float value) { engine.updateDeEsserThresh (value); });
    parameters.deEsserAmount.get()->setAction ([&engine] (float value) { engine.updateDeEsserAmount (value); });
    parameters.reverbDecay.get()->setAction ([&engine] (float value) { engine.updateReverbDecay (value); });
    parameters.reverbDuck.get()->setAction ([&engine] (float value) { engine.updateReverbDuck (value); });
    parameters.reverbLoCut.get()->setAction ([&engine] (float value) { engine.updateReverbLoCut (value); });
    parameters.reverbHiCut.get()->setAction ([&engine] (float value) { engine.updateReverbHiCut (value); });
    
    parameters.inputMode.get()->setAction ([&engine] (int value) { engine.setModulatorSource (value); });
    parameters.leadPan.get()->setAction ([&engine] (int value) { engine.updateDryVoxPan (value); });
    parameters.stereoWidth.get()->setAction ([&engine] (int value) { engine.updateStereoWidth (value); });
    parameters.lowestPanned.get()->setAction ([&engine] (int value) { engine.updateLowestPannedNote (value); });
    parameters.velocitySens.get()->setAction ([&engine] (int value) { engine.updateMidiVelocitySensitivity (value); });
    parameters.pitchbendRange.get()->setAction ([&engine] (int value) { engine.updatePitchbendRange (value); });
    parameters.pedalThresh.get()->setAction ([&engine] (int value) { engine.updatePedalThresh (value); });
    parameters.pedalInterval.get()->setAction ([&engine] (int value) { engine.updatePedalInterval (value); });
    parameters.descantThresh.get()->setAction ([&engine] (int value) { engine.updateDescantThresh (value); });
    parameters.descantInterval.get()->setAction ([&engine] (int value) { engine.updateDescantInterval (value); });
    parameters.reverbDryWet.get()->setAction ([&engine] (int value) { engine.updateReverbDryWet (value); });
    parameters.delayDryWet.get()->setAction ([&engine] (int value) { engine.updateDelayDryWet (value); });
    
    parameters.leadBypass.get()->setAction ([&engine] (bool value) { engine.updateLeadBypass (value); });
    parameters.harmonyBypass.get()->setAction([&engine] (bool value) { engine.updateHarmonyBypass (value); });
    parameters.pedalToggle.get()->setAction ([&engine] (bool value) { engine.updatePedalToggle (value); });
    parameters.descantToggle.get()->setAction ([&engine] (bool value) { engine.updateDescantToggle (value); });
    parameters.voiceStealing.get()->setAction ([&engine] (bool value) { engine.updateNoteStealing (value); });
    parameters.limiterToggle.get()->setAction ([&engine] (bool value) { engine.updateLimiter (value); });
    parameters.noiseGateToggle.get()->setAction ([&engine] (bool value) { engine.updateNoiseGateToggle (value); });
    parameters.compToggle.get()->setAction ([&engine] (bool value) { engine.updateCompressorToggle (value); });
    parameters.aftertouchToggle.get()->setAction ([&engine] (bool value) { engine.updateAftertouchGainOnOff (value); });
    parameters.deEsserToggle.get()->setAction ([&engine] (bool value) { engine.updateDeEsserToggle (value); });
    parameters.reverbToggle.get()->setAction ([&engine] (bool value) { engine.updateReverbToggle (value); });
    parameters.delayToggle.get()->setAction ([&engine] (bool value) { engine.updateDelayToggle (value); });
}
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< float >&);
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< double >&);
