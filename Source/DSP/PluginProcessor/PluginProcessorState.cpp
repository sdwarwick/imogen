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
 
 PluginProcessorState.cpp: This file contains functions dealing with saving & loading the processor's state.
 
======================================================================================================================================================*/


#include "PluginProcessor.h"


/*===========================================================================================================================
    Functions for state saving
 ============================================================================================================================*/

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    saveEditorSizeToValueTree();

    juce::MemoryOutputStream stream (destData, false);
    parameters.toValueTree().writeToStream (stream);
}


/*===========================================================================================================================
    Functions for state loading
 ============================================================================================================================*/

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto newTree = juce::ValueTree::readFromData (data, static_cast< size_t > (sizeInBytes));

    if (! newTree.isValid()) return;

    suspendProcessing (true);

    parameters.restoreFromValueTree (newTree);

    parameters.doAllActions();
    
    parameters.refreshAllDefaults();
    
    updateEditorSizeFromValueTree();

    suspendProcessing (false);

    updateHostDisplay();
}
