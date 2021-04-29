
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
 
 PluginProcessorNetworking.cpp: This file contains the implementations for the processor's ImogenEventSender and ImogenEventReciever interfaces.
 
 ======================================================================================================================================================*/

#include "PluginProcessor.h"


/*=========================================================================================================
    ImogenEventReciever functions
 =========================================================================================================*/

void ImogenAudioProcessor::recieveParameterChange (ParameterID paramID, float newValue)
{
    getParameterPntr(paramID)->orig()->setValueNotifyingHost (newValue);
}

void ImogenAudioProcessor::recieveParameterChangeGestureStart (ParameterID paramID)
{
    getParameterPntr(paramID)->orig()->beginChangeGesture();
}

void ImogenAudioProcessor::recieveParameterChangeGestureEnd (ParameterID paramID)
{
    getParameterPntr(paramID)->orig()->endChangeGesture();
}

void ImogenAudioProcessor::recieveMidiLatchEvent (bool isNowLatched)
{
    const auto value = isNowLatched ? 1.0f : 0.0f;
    nonParamEvents.pushMessage (midiLatchID, value);
}

void ImogenAudioProcessor::recieveKillAllMidiEvent()
{
    nonParamEvents.pushMessage (killAllMidiID, 1.0f);
}

void ImogenAudioProcessor::recieveEditorPitchbendEvent (int wheelValue)
{
    nonParamEvents.pushMessage (pitchBendFromEditorID,
                                pitchbendNormalizedRange.convertTo0to1 (float (wheelValue)));
}

void ImogenAudioProcessor::recieveAbletonLinkChange (bool isNowEnabled)
{
    juce::ignoreUnused (isNowEnabled);
}



/*=========================================================================================================
    ImogenEventSender functions
 =========================================================================================================*/

void ImogenAudioProcessor::sendParameterChange (ParameterID paramID, float newValue)
{
    currentProgram.store (-1);
    
    if (auto* editor = getActiveGui())
        editor->recieveParameterChange (paramID, newValue);
    
    oscSender.sendParameterChange (paramID, newValue);
}

void ImogenAudioProcessor::sendParameterChangeGestureStart (ParameterID paramID)
{
    if (auto* editor = getActiveGui())
        editor->recieveParameterChangeGestureStart (paramID);
    
    oscSender.sendParameterChangeGestureStart (paramID);
}

void ImogenAudioProcessor::sendParameterChangeGestureEnd (ParameterID paramID)
{
    if (auto* editor = getActiveGui())
        editor->recieveParameterChangeGestureEnd (paramID);
    
    oscSender.sendParameterChangeGestureEnd (paramID);
}

void ImogenAudioProcessor::sendErrorCode (ErrorCode code)
{
    if (auto* editor = getActiveGui())
        editor->recieveErrorCode (code);
    
    oscSender.sendErrorCode (code);
}

void ImogenAudioProcessor::sendLoadPreset (const juce::String& presetName)
{
    if (auto* editor = getActiveGui())
        editor->recieveLoadPreset (presetName);
    
    oscSender.sendLoadPreset (presetName);
}

void ImogenAudioProcessor::sendMidiLatch (bool isLatched)
{
    if (auto* editor = getActiveGui())
        editor->recieveMidiLatchEvent (isLatched);
    
    oscSender.sendMidiLatch (isLatched);
}

void ImogenAudioProcessor::sendKillAllMidiEvent()
{
    if (auto* editor = getActiveGui())
        editor->recieveKillAllMidiEvent();
    
    oscSender.sendKillAllMidiEvent();
}

void ImogenAudioProcessor::sendEnableAbletonLink (bool isEnabled)
{
    if (auto* editor = getActiveGui())
        editor->recieveAbletonLinkChange (isEnabled);
    
    oscSender.sendEnableAbletonLink (isEnabled);
}

