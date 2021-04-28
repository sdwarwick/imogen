/*==========================================================================================================================
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
 
 ImogenCommon.h: This file defines some common constants that the various modules of Imogen all refer to.
 
==========================================================================================================================*/


#pragma once

#ifndef IMOGEN_REMOTE_APP
  #define IMOGEN_REMOTE_APP 0
#endif


namespace Imogen
{


enum ParameterID
{
    inputSourceID,
    mainBypassID,
    leadBypassID,
    harmonyBypassID,
    dryPanID,
    dryWetID,
    adsrAttackID,
    adsrDecayID,
    adsrSustainID,
    adsrReleaseID,
    stereoWidthID,
    lowestPannedID,
    velocitySensID,
    pitchBendRangeID,
    pedalPitchIsOnID,
    pedalPitchThreshID,
    pedalPitchIntervalID,
    descantIsOnID,
    descantThreshID,
    descantIntervalID,
    voiceStealingID,
    inputGainID,
    outputGainID,
    limiterToggleID,
    noiseGateToggleID,
    noiseGateThresholdID,
    compressorToggleID,
    compressorAmountID,
    aftertouchGainToggleID,
    deEsserToggleID,
    deEsserThreshID,
    deEsserAmountID,
    reverbToggleID,
    reverbDryWetID,
    reverbDecayID,
    reverbDuckID,
    reverbLoCutID,
    reverbHiCutID,
    delayToggleID,
    delayDryWetID
};
static constexpr int numParams = delayDryWetID + 1;


enum EventID
{
    killAllMidiID,
    midiLatchID,
    pitchBendFromEditorID
};
static constexpr int numEventIDs = pitchBendFromEditorID + 1;



enum ErrorCode
{
    loadingPresetFailed
};


}  // namespace


/*
 */


struct ImogenEventReciever
{
    virtual ~ImogenEventReciever() = default;
    
    virtual void recieveParameterChange             (Imogen::ParameterID paramID, float newValue) = 0;
    virtual void recieveParameterChangeGestureStart (Imogen::ParameterID paramID) = 0;
    virtual void recieveParameterChangeGestureEnd   (Imogen::ParameterID paramID) = 0;
    
    virtual void recieveLoadPreset   (const juce::String& presetName) = 0;
    virtual void recieveSavePreset   (const juce::String& presetName) = 0;
    virtual void recieveDeletePreset (const juce::String& presetName) = 0;
    
    virtual void recieveAbletonLinkChange (bool isNowEnabled) = 0;
    
    virtual void recieveEditorPitchbendEvent (int wheelValue) = 0;
    virtual void recieveMidiLatchEvent (bool isNowLatched) = 0;
    virtual void recieveKillAllMidiEvent() = 0;
    
    virtual void recieveMTSconnectionChange (bool isNowConnected) = 0;
    virtual void recieveMTSscaleChange (const juce::String& newScaleName) = 0;
    
    virtual void recieveErrorCode (Imogen::ErrorCode code) = 0;
};


/*
 */


struct ImogenEventSender
{
    virtual ~ImogenEventSender() = default;
    
    virtual void sendParameterChange             (Imogen::ParameterID paramID, float newValue) = 0;
    virtual void sendParameterChangeGestureStart (Imogen::ParameterID paramID) = 0;
    virtual void sendParameterChangeGestureEnd   (Imogen::ParameterID paramID) = 0;
    
    virtual void sendEditorPitchbend (int wheelValue) = 0;
    virtual void sendMidiLatch (bool shouldBeLatched) = 0;
    virtual void sendKillAllMidiEvent() = 0;
    
    virtual void sendLoadPreset   (const juce::String& presetName) = 0;
    virtual void sendSavePreset   (const juce::String& presetName) = 0;
    virtual void sendDeletePreset (const juce::String& presetName) = 0;
    
    virtual void sendEnableAbletonLink (bool shouldBeEnabled) = 0;
    
    virtual void sendErrorCode (Imogen::ErrorCode code) = 0;
};
