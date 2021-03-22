/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "PluginEditor.h"
#include "bv_SharedCode/misc/BinaryDataHelpers.h"


#define bvi_GRAPHICS_FRAMERATE_HZ 60


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), imgnProcessor(p), params(p)
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
