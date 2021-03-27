
/*===================================================================================================================================================
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
 
 PluginEditor.cpp: This file defines implementation details for Imogen's top-level GUI component

===================================================================================================================================================*/


#include "PluginEditor.h"
#include "bv_SharedCode/BinaryDataHelpers.h"


#define bvi_GRAPHICS_FRAMERATE_HZ 60


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), imgnProcessor(p)
{
    this->setBufferedToImage (true);
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    //addAndMakeVisible(selectPreset);
    
    setSize (940, 435);
    
    Timer::startTimerHz (bvi_GRAPHICS_FRAMERATE_HZ);
}

#undef bvi_GRAPHICS_FRAMERATE_HZ


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
    this->setLookAndFeel(nullptr);
    Timer::stopTimer();
}


void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}


void ImogenAudioProcessorEditor::resized()
{
    //selectPreset.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    if (imgnProcessor.hasUpdatedParamDefaults())
        updateParameterDefaults();
}


inline void ImogenAudioProcessorEditor::newPresetSelected()
{
    imgnProcessor.loadPreset (selectPreset.getItemText (selectPreset.getSelectedId()));
}


inline void ImogenAudioProcessorEditor::makePresetMenu (juce::ComboBox& box)
{
    int id = 1;
    
    for  (juce::DirectoryEntry entry  :   juce::RangedDirectoryIterator (imgnProcessor.getPresetsFolder(), false))
    {
        box.addItem (entry.getFile().getFileName(), id);
        ++id;
    }
}


void ImogenAudioProcessorEditor::updateParameterDefaults()
{
    
}
