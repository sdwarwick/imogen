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
    Functions for state saving and loading
*/


// functions for saving state info.....................................

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml = tree.copyState().createXml();
    copyXmlToBinary (*xml, destData);
}

void ImogenAudioProcessor::savePreset (juce::String presetName) // this function can be used both to save new preset files or to update existing ones
{
    std::unique_ptr<juce::XmlElement> xml = tree.copyState().createXml();
    
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
        updatePluginInternalState (*xmlElement, doubleEngine);
    else
        updatePluginInternalState (*xmlElement, floatEngine);
}

bool ImogenAudioProcessor::loadPreset (juce::String presetName)
{
    presetName += ".xml";
    
    juce::File presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if (! presetToLoad.existsAsFile())
        return false;
    
    auto xmlElement = juce::parseXML (presetToLoad);
    
    if (isUsingDoublePrecision())
        return updatePluginInternalState (*xmlElement, doubleEngine);
    
    return updatePluginInternalState (*xmlElement, floatEngine);
}


template<typename SampleType>
inline bool ImogenAudioProcessor::updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine)
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
    
    suspendProcessing (false);
    
    updateHostDisplay();
    return true;
}


void ImogenAudioProcessor::deletePreset (juce::String presetName)
{
    presetName += ".xml";
    
    juce::File presetToDelete = getPresetsFolder().getChildFile (presetName);
    
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
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
    juce::ignoreUnused (index);
    return {};
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index == getCurrentProgram())
    {
        deletePreset (getProgramName (index));
        savePreset (newName);
    }
}

