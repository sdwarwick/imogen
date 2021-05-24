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


void ImogenAudioProcessor::applyValueTreeStateChange (const void* encodedChangeData, size_t encodedChangeDataSize)
{
    jassert (juce::MessageManager::getInstance()->isThisTheMessageThread());
    juce::ValueTreeSynchroniser::applyChange (state, encodedChangeData, encodedChangeDataSize, nullptr);
}


/*===========================================================================================================================
    Functions for state saving
 ============================================================================================================================*/

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    saveEditorSizeToValueTree();

    juce::MemoryOutputStream stream (destData, false);
    state.writeToStream (stream);
}


/*===========================================================================================================================
    Functions for state loading
 ============================================================================================================================*/

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto newTree = juce::ValueTree::readFromData (data, static_cast< size_t > (sizeInBytes));

    if (!newTree.isValid() || !newTree.hasType (state.getType())) return;

    suspendProcessing (true);

    state.copyPropertiesAndChildrenFrom (newTree, nullptr);

    parameters.doAllActions();
    
    parameters.refreshAllDefaults();
    
    updateEditorSizeFromValueTree();

    treeSync.sendFullSyncCallback();

    suspendProcessing (false);

    updateHostDisplay();
}
