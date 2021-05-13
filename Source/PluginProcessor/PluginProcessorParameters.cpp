
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
    using namespace Imogen;
    
    getFloatParameterPntr (adsrAttackID)->onAction = [&engine](float value) { engine.updateAdsrAttack (value); };
    getFloatParameterPntr (adsrDecayID)->onAction = [&engine](float value) { engine.updateAdsrDecay (value); };
    getFloatParameterPntr (adsrSustainID)->onAction = [&engine](float value) { engine.updateAdsrSustain (value); };
    getFloatParameterPntr (adsrReleaseID)->onAction = [&engine](float value) { engine.updateAdsrRelease (value); };
    getFloatParameterPntr (inputGainID)->onAction = [&engine](float value) { engine.updateInputGain (value); };
    getFloatParameterPntr (outputGainID)->onAction = [&engine](float value) { engine.updateOutputGain (value); };
    getFloatParameterPntr (noiseGateThresholdID)->onAction = [&engine](float value) { engine.updateNoiseGateThresh (value); };
    getFloatParameterPntr (compressorAmountID)->onAction = [&engine](float value) { engine.updateCompressorAmount (value); };
    getFloatParameterPntr (deEsserThreshID)->onAction = [&engine](float value) { engine.updateDeEsserThresh (value); };
    getFloatParameterPntr (deEsserAmountID)->onAction = [&engine](float value) { engine.updateDeEsserAmount (value); };
    getFloatParameterPntr (reverbDecayID)->onAction = [&engine](float value) { engine.updateReverbDecay (value); };
    getFloatParameterPntr (reverbDuckID)->onAction = [&engine](float value) { engine.updateReverbDuck (value); };
    getFloatParameterPntr (reverbLoCutID)->onAction = [&engine](float value) { engine.updateReverbLoCut (value); };
    getFloatParameterPntr (reverbHiCutID)->onAction = [&engine](float value) { engine.updateReverbHiCut (value); };
    
    getIntParameterPntr (inputSourceID)->onAction = [&engine](int value) { engine.setModulatorSource (value); };
    getIntParameterPntr (dryPanID)->onAction = [&engine](int value) { engine.updateDryVoxPan(value); };
    getIntParameterPntr (stereoWidthID)->onAction = [&engine](int value) { engine.updateStereoWidth (value); };
    getIntParameterPntr (lowestPannedID)->onAction = [&engine](int value) { engine.updateLowestPannedNote (value); };
    getIntParameterPntr (velocitySensID)->onAction = [&engine](int value) { engine.updateMidiVelocitySensitivity (value); };
    getIntParameterPntr (pitchBendRangeID)->onAction = [&engine](int value) { engine.updatePitchbendRange (value); };
    getIntParameterPntr (pedalPitchThreshID)->onAction = [&engine](int value) { engine.updatePedalThresh (value); };
    getIntParameterPntr (pedalPitchIntervalID)->onAction = [&engine](int value) { engine.updatePedalInterval (value); };
    getIntParameterPntr (descantThreshID)->onAction = [&engine](int value) { engine.updateDescantThresh (value); };
    getIntParameterPntr (descantIntervalID)->onAction = [&engine](int value) { engine.updateDescantInterval (value); };
    getIntParameterPntr (reverbDryWetID)->onAction = [&engine](int value) { engine.updateReverbDryWet (value); };
    getIntParameterPntr (delayDryWetID)->onAction = [&engine](int value) { engine.updateDelayDryWet (value); };
    
    getBoolParameterPntr (leadBypassID)->onAction = [&engine](bool value) { engine.updateLeadBypass (value); };
    getBoolParameterPntr (harmonyBypassID)->onAction = [&engine](bool value) { engine.updateHarmonyBypass (value); };
    getBoolParameterPntr (pedalPitchIsOnID)->onAction = [&engine](bool value) { engine.updatePedalToggle (value); };
    getBoolParameterPntr (descantIsOnID)->onAction = [&engine](bool value) { engine.updateDescantToggle (value); };
    getBoolParameterPntr (voiceStealingID)->onAction = [&engine](bool value) { engine.updateNoteStealing (value); };
    getBoolParameterPntr (limiterToggleID)->onAction = [&engine](bool value) { engine.updateLimiter (value); };
    getBoolParameterPntr (noiseGateToggleID)->onAction = [&engine](bool value) { engine.updateNoiseGateToggle (value); };
    getBoolParameterPntr (compressorToggleID)->onAction = [&engine](bool value) { engine.updateCompressorToggle (value); };
    getBoolParameterPntr (aftertouchGainToggleID)->onAction = [&engine](bool value) { engine.updateAftertouchGainOnOff (value); };
    getBoolParameterPntr (deEsserToggleID)->onAction = [&engine](bool value) { engine.updateDeEsserToggle (value); };
    getBoolParameterPntr (reverbToggleID)->onAction = [&engine](bool value) { engine.updateReverbToggle (value); };
    getBoolParameterPntr (delayToggleID)->onAction = [&engine](bool value) { engine.updateDelayToggle (value); };
}
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<float>&);
template void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine<double>&);


/*===================================================================================*/


template <typename SampleType>
void ImogenAudioProcessor::initializePropertyActions (bav::ImogenEngine<SampleType>& engine)
{
    using namespace Imogen;
    
    // getBoolPropertyPntr (linkIsEnabledID)->onAction =
    
    getBoolPropertyPntr (midiLatchID)->onAction = [&engine](bool value) { engine.updateMidiLatch (value); };
    
    getIntPropertyPntr (editorPitchbendID)->onAction = [&engine](int value) { engine.recieveExternalPitchbend (value); };
}
template void ImogenAudioProcessor::initializePropertyActions (bav::ImogenEngine<float>&);
template void ImogenAudioProcessor::initializePropertyActions (bav::ImogenEngine<double>&);


/*===================================================================================*/


void ImogenAudioProcessor::initializePropertyValueUpdatingFunctions()
{
    using namespace Imogen;
    
    // all of these must be thread-safe! non-thread safe updates are handled explicitly in the timer callback
    getIntPropertyPntr (linkNumSessionPeersID)->getNewValueFromExternalSource = [this]() { return getNumAbletonLinkSessionPeers(); };
    getBoolPropertyPntr (mtsEspIsConnectedID)->getNewValueFromExternalSource = [this]() { return isConnectedToMtsEsp(); };
    getBoolPropertyPntr (linkIsEnabledID)->getNewValueFromExternalSource = [this]() { return isAbletonLinkEnabled(); };
    
    //getBoolPropertyPntr (midiLatchID)->getNewValueFromExternalSource =
    //getIntPropertyPntr (lastMovedMidiCCnumberID)->getNewValueFromExternalSource =
    //getIntPropertyPntr (lastMovedMidiCCvalueID)->getNewValueFromExternalSource =
}


/*===========================================================================================================================
 ============================================================================================================================*/

bav::Parameter* ImogenAudioProcessor::getParameterPntr (const ParameterID paramID) const
{
    for (auto* pntr : parameterPointers)
        if (static_cast<ParameterID>(pntr->key()) == paramID)
            return pntr;
    
    return nullptr;
}

bav::FloatParameter* ImogenAudioProcessor::getFloatParameterPntr (const ParameterID paramID) const
{
    return dynamic_cast<bav::FloatParameter*> (getParameterPntr (paramID));
}

bav::IntParameter* ImogenAudioProcessor::getIntParameterPntr (const ParameterID paramID) const
{
    return dynamic_cast<bav::IntParameter*> (getParameterPntr (paramID));
}

bav::BoolParameter* ImogenAudioProcessor::getBoolParameterPntr (const ParameterID paramID) const
{
    return dynamic_cast<bav::BoolParameter*> (getParameterPntr (paramID));
}


inline bav::Parameter* ImogenAudioProcessor::getMeterParamPntr (const MeterID meterID) const
{
    for (auto* pntr : meterParameterPointers)
        if (static_cast<MeterID>(pntr->key()) == meterID)
            return pntr;
    
    return nullptr;
}


/*===================================================================================*/


bav::NonParamValueTreeNode* ImogenAudioProcessor::getPropertyPntr (const NonAutomatableParameterID propID) const
{
    for (auto* pntr : propertyPointers)
        if (static_cast<NonAutomatableParameterID>(pntr->nodeID) == propID)
            return pntr;
    
    return nullptr;
}

bav::IntValueTreeNode* ImogenAudioProcessor::getIntPropertyPntr (const NonAutomatableParameterID propID) const
{
    return dynamic_cast<bav::IntValueTreeNode*> (getPropertyPntr (propID));
}

bav::BoolValueTreeNode* ImogenAudioProcessor::getBoolPropertyPntr (const NonAutomatableParameterID propID) const
{
    return dynamic_cast<bav::BoolValueTreeNode*> (getPropertyPntr (propID));
}

bav::StringValueTreeNode* ImogenAudioProcessor::getStringPropertyPntr (const NonAutomatableParameterID propID) const
{
    return dynamic_cast<bav::StringValueTreeNode*> (getPropertyPntr (propID));
}
