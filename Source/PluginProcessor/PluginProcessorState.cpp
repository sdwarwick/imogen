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
    auto editorSize = tree.state.getOrCreateChildWithName ("editorSize", nullptr);
    editorSize.setProperty ("editorSize_X", savedEditorSize.x, nullptr);
    editorSize.setProperty ("editorSize_Y", savedEditorSize.y, nullptr);
    
    auto xml = tree.copyState().createXml();
    
    if (xml->hasAttribute("presetName"))
        xml->removeAttribute("presetName");
    
    copyXmlToBinary (*xml, destData);
}


/*===========================================================================================================================
    Functions for state loading
 ============================================================================================================================*/

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto newState = *(getXmlFromBinary (data, sizeInBytes));
    
    if (! newState.hasTagName (tree.state.getType()))
        return;
    
    auto newTree = juce::ValueTree::fromXml (newState);
    
    if (! newTree.isValid())
        return;
    
    suspendProcessing (true);
    
    tree.replaceState (newTree);
    
    auto editor = tree.state.getChildWithName ("editorSize");
    
    if (editor.isValid())
    {
        savedEditorSize.setX (editor.getProperty ("editorSize_X", 900));
        savedEditorSize.setY (editor.getProperty ("editorSize_Y", 500));
        
        if (auto* activeEditor = getActiveEditor())
            activeEditor->setSize (savedEditorSize.x, savedEditorSize.y);
    }
    
    suspendProcessing (false);
    
    updateHostDisplay();
}


/*===========================================================================================================================
    Functions for managing programs
 ==========================================================================================================================*/

int ImogenAudioProcessor::getNumPrograms()
{
    return 1;
}

int ImogenAudioProcessor::getCurrentProgram()
{
    return 1;
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return { };
}

int ImogenAudioProcessor::indexOfProgram (const juce::String& name) const
{
    juce::ignoreUnused (name);
    return 0;
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}
