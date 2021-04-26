
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
 
 PluginProcessorNetworking.cpp: This file contains functions dealing with networking and communications.
 
 ======================================================================================================================================================*/

#include "PluginProcessor.h"


void ImogenAudioProcessor::recieveParameterValueChange (ParameterID paramID, float newValue)
{
    getParameterPntr(paramID)->orig()->setValueNotifyingHost (newValue);
}

void ImogenAudioProcessor::recieveParameterChangeGestureBegin (ParameterID paramID)
{
    getParameterPntr(paramID)->orig()->beginChangeGesture();
}

void ImogenAudioProcessor::recieveParameterChangeGestureEnd (ParameterID paramID)
{
    getParameterPntr(paramID)->orig()->endChangeGesture();
}

void ImogenAudioProcessor::sendParameterChange (ParameterID paramID, float newValue)
{
    currentProgram.store (-1);
    
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChange (paramID, newValue);
    
    // send param change as OSC message to any recievers...
}

void ImogenAudioProcessor::sendParameterChangeGestureBegin (ParameterID paramID)
{
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChangeGestureStart (paramID);
    
    // send OSC message...
}

void ImogenAudioProcessor::sendParameterChangeGestureEnd (ParameterID paramID)
{
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChangeGestureEnd (paramID);
    
    // send OSC message...
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
