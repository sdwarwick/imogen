
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


void ImogenAudioProcessor::recieveParameterValueChange (parameterID paramID, float newValue)
{
    getParameterPntr(paramID)->orig()->setValueNotifyingHost (newValue);
}

void ImogenAudioProcessor::recieveParameterChangeGestureBegin (parameterID paramID)
{
    getParameterPntr(paramID)->orig()->beginChangeGesture();
}

void ImogenAudioProcessor::recieveParameterChangeGestureEnd (parameterID paramID)
{
    getParameterPntr(paramID)->orig()->endChangeGesture();
}

void ImogenAudioProcessor::sendParameterChange (parameterID paramID, float newValue)
{
    currentProgram.store (-1);
    
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChange (paramID, newValue);
    
    // send param change as OSC message to any recievers...
}

void ImogenAudioProcessor::sendParameterChangeGestureBegin (parameterID paramID)
{
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChangeGestureStart (paramID);
    
    // send OSC message...
}

void ImogenAudioProcessor::sendParameterChangeGestureEnd (parameterID paramID)
{
    if (auto* activeEditor = getActiveEditor())
        dynamic_cast<ImogenGuiHolder*>(activeEditor)->recieveParameterChangeGestureEnd (paramID);
    
    // send OSC message...
}

void ImogenAudioProcessor::recieveMidiLatchEvent (bool isNowLatched)
{
    const auto value = isNowLatched ? 1.0f : 0.0f;
    nonParamEvents.pushMessage (midiLatch, value);
}

void ImogenAudioProcessor::recieveKillAllMidiEvent()
{
    nonParamEvents.pushMessage (killAllMidi, 1.0f);
}

void ImogenAudioProcessor::recieveEditorPitchbendEvent (int wheelValue)
{
    nonParamEvents.pushMessage (pitchBendFromEditor,
                                pitchbendNormalizedRange.convertTo0to1 (float (wheelValue)));
}
