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
    if (isUsingDoublePrecision())
        updatePluginInternalState (*(getXmlFromBinary (data, sizeInBytes)), doubleEngine, false);
    else
        updatePluginInternalState (*(getXmlFromBinary (data, sizeInBytes)), floatEngine, false);
}


template<typename SampleType>
inline bool ImogenAudioProcessor::updatePluginInternalState (juce::XmlElement& newState,
                                                             bav::ImogenEngine<SampleType>& activeEngine,
                                                             bool isPresetChange)
{
    if (! newState.hasTagName (tree.state.getType()))
        return false;
           
    auto newTree = juce::ValueTree::fromXml (newState);
           
    if (! newTree.isValid())
        return false;
    
    suspendProcessing (true);
    
    tree.replaceState (newTree);
    
    updateAllParameters (activeEngine);
    
    updateParameterDefaults();
    
    if (! isPresetChange)
    {
        auto editor = tree.state.getChildWithName ("editorSize");
        
        if (editor.isValid())
        {
            savedEditorSize.setX (editor.getProperty ("editorSize_X", 900));
            savedEditorSize.setY (editor.getProperty ("editorSize_Y", 500));
            
            if (auto* activeEditor = getActiveEditor())
                activeEditor->setSize (savedEditorSize.x, savedEditorSize.y);
        }
    }
    
    suspendProcessing (false);
    
    updateHostDisplay();
    return true;
}


/*===========================================================================================================================
    Functions for managing programs
 ==========================================================================================================================*/

int ImogenAudioProcessor::getNumPrograms()
{
    //return std::max (1, availablePresets.size());
}

int ImogenAudioProcessor::getCurrentProgram()
{
 //   return currentProgram.load();
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    if (index >= getNumPrograms())
        return;
    
//    if (index != currentProgram.load())
//        internalEventHandler.recieveLoadPreset (getProgramName (index));
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
//    if (availablePresets.isEmpty() || index < 0 || index >= availablePresets.size())
//        return {};
//
//    return removePresetFileExtensionIfThere (availablePresets.getUnchecked(index).getFileName());
}

int ImogenAudioProcessor::indexOfProgram (const juce::String& name) const
{
//    return name.isEmpty() ? -1 : availablePresets.indexOf (name);
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
//    renamePreset (getProgramName (index), newName);
}
