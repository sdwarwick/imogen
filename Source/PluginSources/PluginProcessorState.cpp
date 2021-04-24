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


/*
    Scans the presets folder for preset files
*/
void ImogenAudioProcessor::rescanPresetsFolder()
{
    availablePresets.clearQuick();
    
    for (juce::DirectoryEntry entry  :   juce::RangedDirectoryIterator (getPresetsFolder(), false))
    {
        const auto file = entry.getFile();
        const auto filename = file.getFileName();
        
        if (filename.endsWith (".xml"))
            availablePresets.add (file);
    }
}



/*
    Functions for state saving and loading
*/


// functions for saving state info.....................................

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

void ImogenAudioProcessor::savePreset (juce::String presetName) // this function can be used both to save new preset files or to update existing ones
{
    if (presetName.endsWith(".xml"))
        presetName.dropLastCharacters(4);
    
    if (tree.state.hasProperty("editorSize"))
        tree.state.removeProperty ("editorSize", nullptr);
    
    auto xml = tree.copyState().createXml();
    
    xml->setAttribute ("presetName", presetName);
    
    presetName += ".xml";
    
    xml->writeTo (getPresetsFolder().getChildFile (presetName));
    updateHostDisplay();
}


// functions for loading state info.................................

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlElement (getXmlFromBinary (data, sizeInBytes));
    
    if (isUsingDoublePrecision())
        updatePluginInternalState (*xmlElement, doubleEngine, false);
    else
        updatePluginInternalState (*xmlElement, floatEngine, false);
}

bool ImogenAudioProcessor::loadPreset (juce::String presetName)
{
    if (! presetName.endsWith(".xml"))
        presetName += ".xml";
    
    auto presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if (! presetToLoad.existsAsFile())
        return false;
    
    auto xmlElement = juce::parseXML (presetToLoad);
    
    if (isUsingDoublePrecision())
        return updatePluginInternalState (*xmlElement, doubleEngine, true);
    
    return updatePluginInternalState (*xmlElement, floatEngine, true);
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
    
    if (! isPresetChange)  // don't resize the editor for preset changes...
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


void ImogenAudioProcessor::deletePreset (juce::String presetName)
{
    if (! presetName.endsWith(".xml"))
        presetName += ".xml";
    
    auto presetToDelete = getPresetsFolder().getChildFile (presetName);
    
    if (presetToDelete.existsAsFile())
        if (! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    
    updateHostDisplay();
}


/*
    Functions for managing "programs".
*/

int ImogenAudioProcessor::getNumPrograms()
{
    return std::max (1, availablePresets.size());
}

int ImogenAudioProcessor::getCurrentProgram()
{
    return 1;
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    if (index != getCurrentProgram())
        loadPreset (getProgramName (index));
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
    if (availablePresets.isEmpty())
        return {};
    
    const auto name = availablePresets.getUnchecked(index).getFileName();
    
    return (name.endsWith(".xml")) ? name.dropLastCharacters(4) : name;
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index == getCurrentProgram())
    {
        deletePreset (getProgramName (index));
        savePreset (newName);
    }
}

