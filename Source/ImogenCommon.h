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


static inline juce::String getParameterNameShort (ParameterID param)
{
    switch (param)
    {
        case (inputSourceID):          return TRANS ("Input source");
        case (mainBypassID):           return TRANS ("Main bypass");
        case (leadBypassID):           return TRANS ("Lead bypass");
        case (harmonyBypassID):        return TRANS ("Harmony bypass");
        case (dryPanID):               return TRANS ("Lead pan");
        case (dryWetID):               return TRANS ("Dry/wet");
        case (adsrAttackID):           return TRANS ("Attack");
        case (adsrDecayID):            return TRANS ("Decay");
        case (adsrSustainID):          return TRANS ("Sustain");
        case (adsrReleaseID):          return TRANS ("Release");
        case (stereoWidthID):          return TRANS ("Stereo width");
        case (lowestPannedID):         return TRANS ("Lowest panned note");
        case (velocitySensID):         return TRANS ("MIDI velocity sensitivity");
        case (pitchBendRangeID):       return TRANS ("st");
        case (pedalPitchIsOnID):       return TRANS ("Toggle");
        case (pedalPitchThreshID):     return TRANS ("Threshold");
        case (pedalPitchIntervalID):   return TRANS ("Interval");
        case (descantIsOnID):          return TRANS ("Toggle");
        case (descantThreshID):        return TRANS ("Threshold");
        case (descantIntervalID):      return TRANS ("Interval");
        case (voiceStealingID):        return TRANS ("Voice stealing on/off");
        case (inputGainID):            return TRANS ("In");
        case (outputGainID):           return TRANS ("Out");
        case (limiterToggleID):        return TRANS ("Toggle");
        case (noiseGateToggleID):      return TRANS ("Toggle");
        case (noiseGateThresholdID):   return TRANS ("Threshold");
        case (compressorToggleID):     return TRANS ("Toggle");
        case (compressorAmountID):     return TRANS ("Amount");
        case (aftertouchGainToggleID): return TRANS ("Aftertouch gain toggle");
        case (deEsserToggleID):        return TRANS ("Toggle");
        case (deEsserAmountID):        return TRANS ("Amount");
        case (deEsserThreshID):        return TRANS ("Thresh");
        case (reverbToggleID):         return TRANS ("Toggle");
        case (reverbDryWetID):         return TRANS ("Dry/wet");
        case (reverbDecayID):          return TRANS ("Decay");
        case (reverbDuckID):           return TRANS ("Duck");
        case (reverbLoCutID):          return TRANS ("Low cut");
        case (reverbHiCutID):          return TRANS ("High cut");
        case (delayToggleID):          return TRANS ("Toggle");
        case (delayDryWetID):          return TRANS ("Dry/wet");
    }
}


static inline juce::String getParameterNameVerbose (ParameterID param)
{
    switch (param)
    {
        case (inputSourceID):          return TRANS ("Input source");
        case (mainBypassID):           return TRANS ("Main bypass");
        case (leadBypassID):           return TRANS ("Lead bypass");
        case (harmonyBypassID):        return TRANS ("Harmony bypass");
        case (dryPanID):               return TRANS ("Lead pan");
        case (dryWetID):               return TRANS ("Dry/wet");
        case (adsrAttackID):           return TRANS ("ADSR attack");
        case (adsrDecayID):            return TRANS ("ADSR decay");
        case (adsrSustainID):          return TRANS ("ADSR sustain");
        case (adsrReleaseID):          return TRANS ("ADSR release");
        case (stereoWidthID):          return TRANS ("Stereo width");
        case (lowestPannedID):         return TRANS ("Lowest panned note");
        case (velocitySensID):         return TRANS ("MIDI velocity sensitivity");
        case (pitchBendRangeID):       return TRANS ("Pitchbend range (st)");
        case (pedalPitchIsOnID):       return TRANS ("Pedal pitch toggle");
        case (pedalPitchThreshID):     return TRANS ("Pedal pitch upper threshold");
        case (pedalPitchIntervalID):   return TRANS ("Pedal pitch interval");
        case (descantIsOnID):          return TRANS ("Descant toggle");
        case (descantThreshID):        return TRANS ("Descant lower threshold");
        case (descantIntervalID):      return TRANS ("Descant interval");
        case (voiceStealingID):        return TRANS ("Voice stealing on/off");
        case (inputGainID):            return TRANS ("Input gain");
        case (outputGainID):           return TRANS ("Output gain");
        case (limiterToggleID):        return TRANS ("Limiter toggle");
        case (noiseGateToggleID):      return TRANS ("Noise gate toggle");
        case (noiseGateThresholdID):   return TRANS ("Noise gate threshold");
        case (compressorToggleID):     return TRANS ("Compressor toggle");
        case (compressorAmountID):     return TRANS ("Compressor amount");
        case (aftertouchGainToggleID): return TRANS ("Aftertouch gain toggle");
        case (deEsserToggleID):        return TRANS ("De-esser toggle");
        case (deEsserAmountID):        return TRANS ("De-esser amount");
        case (deEsserThreshID):        return TRANS ("De-esser thresh");
        case (reverbToggleID):         return TRANS ("Reverb toggle");
        case (reverbDryWetID):         return TRANS ("Reverb dry/wet");
        case (reverbDecayID):          return TRANS ("Reverb decay");
        case (reverbDuckID):           return TRANS ("Reverb duck");
        case (reverbLoCutID):          return TRANS ("Reverb low cut");
        case (reverbHiCutID):          return TRANS ("Reverb high cut");
        case (delayToggleID):          return TRANS ("Delay toggle");
        case (delayDryWetID):          return TRANS ("Delay dry/wet");
    }
}


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

