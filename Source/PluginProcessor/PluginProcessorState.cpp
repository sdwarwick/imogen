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
    const auto prevProgramName = getProgramName (currentProgram.load());
    
    availablePresets.clearQuick();
    
    for (auto entry  :   juce::RangedDirectoryIterator (getPresetsFolder(), false))
    {
        const auto file = entry.getFile();
        const auto filename = file.getFileName();
        
        if (filename.endsWith (".xml"))
            availablePresets.add (filename.dropLastCharacters(4));
    }
    
    if (! prevProgramName.isEmpty())
        currentProgram.store (indexOfProgram (prevProgramName));
}


void ImogenAudioProcessor::recieveDeletePreset (const juce::String& presetName)
{
    auto name = presetName;
    
    if (! name.endsWith(".xml"))
        name += ".xml";
    
    auto presetToDelete = getPresetsFolder().getChildFile (name);
    
    if (presetToDelete.existsAsFile())
        if (! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    
    rescanPresetsFolder();
    updateHostDisplay();
}


juce::String ImogenAudioProcessor::getActivePresetName()
{
    const auto program = currentProgram.load();
    
    if (program == -1)
        return {};
    
    return getProgramName (program);
}



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

void ImogenAudioProcessor::recieveSavePreset (const juce::String& presetName) // this function can be used both to save new preset files or to update existing ones
{
    auto name = presetName;
    
    if (name.endsWith(".xml"))
        name = name.dropLastCharacters(4);
    
    if (tree.state.hasProperty("editorSize"))
        tree.state.removeProperty ("editorSize", nullptr);
    
    auto xml = tree.copyState().createXml();
    
    xml->setAttribute ("presetName", name);
    
    name += ".xml";
    
    xml->writeTo (getPresetsFolder().getChildFile (name));
    
    rescanPresetsFolder();
    updateHostDisplay();
}


/*===========================================================================================================================
    Functions for state loading
 ============================================================================================================================*/

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlElement (getXmlFromBinary (data, sizeInBytes));
    
    if (isUsingDoublePrecision())
        updatePluginInternalState (*xmlElement, doubleEngine, false);
    else
        updatePluginInternalState (*xmlElement, floatEngine, false);
    
    currentProgram.store (-1);
}

void ImogenAudioProcessor::recieveLoadPreset (const juce::String& presetName)
{
    if (presetName.isEmpty())
    {
        sendErrorCode (ErrorCode::loadingPresetFailed);
        return;
    }
    
    rescanPresetsFolder();
    
    auto name = presetName;
    
    if (! name.endsWith(".xml"))
        name += ".xml";
    
    auto presetToLoad = getPresetsFolder().getChildFile (name);
    
    if (! presetToLoad.existsAsFile())
    {
        sendErrorCode (ErrorCode::loadingPresetFailed);
        return;
    }
    
    auto xmlElement = juce::parseXML (presetToLoad);
    
    const auto result = isUsingDoublePrecision() ? updatePluginInternalState (*xmlElement, doubleEngine, true)
                                                 : updatePluginInternalState (*xmlElement, floatEngine,  true);
    
    if (result)
    {
        if (availablePresets.isEmpty())
            currentProgram.store (-1);
        else
            currentProgram.store (indexOfProgram (name.dropLastCharacters(4)));
    }
    else
    {
        sendErrorCode (ErrorCode::loadingPresetFailed);
    }
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
    return std::max (1, availablePresets.size());
}

int ImogenAudioProcessor::getCurrentProgram()
{
    return currentProgram.load();
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    if (index >= getNumPrograms())
        return;
    
    if (index != currentProgram.load())
        recieveLoadPreset (getProgramName (index));
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
    if (availablePresets.isEmpty() || index < 0 || index >= availablePresets.size())
        return {};
    
    const auto name = availablePresets.getUnchecked(index).getFileName();
    
    return (name.endsWith(".xml")) ? name.dropLastCharacters(4) : name;
}

int ImogenAudioProcessor::indexOfProgram (const juce::String& name) const
{
    return availablePresets.indexOf (name);
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    recieveDeletePreset (getProgramName (index));
    
    if (index == getCurrentProgram())
        recieveSavePreset (newName);
    
    rescanPresetsFolder();
}
