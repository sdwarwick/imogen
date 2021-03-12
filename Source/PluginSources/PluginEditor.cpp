/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "PluginEditor.h"
#include "BinaryData.h"


#define bvi_GRAPHICS_FRAMERATE_HZ 60


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), audioProcessor (p),
    midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel)
{
    this->setBufferedToImage (true);
    
    midiPanel   .setLookAndFeel(&lookAndFeel);
    ioPanel     .setLookAndFeel(&lookAndFeel);
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    modulatorInputSource.addItem ("Left",  1);
    modulatorInputSource.addItem ("Right", 2);
    modulatorInputSource.addItem ("Mix to mono", 3);
    modulatorInputSource.setSelectedId (audioProcessor.getCurrentModulatorInputSource(),
                                        juce::NotificationType::dontSendNotification);
    modulatorInputSource.onChange = [this] { audioProcessor.updateModulatorInputSource (modulatorInputSource.getSelectedId()); };
    
    addAndMakeVisible(midiPanel);
    addAndMakeVisible(ioPanel);
    
    //addAndMakeVisible(selectPreset);
    //addAndMakeVisible(modulatorInputSource);
    
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
    midiPanel.setBounds(10, 10, 300, 415);
    ioPanel  .setBounds(320, 10, 300, 415);
    
    //selectPreset.setBounds(x, y, w, h);
    
    //modulatorInputSource.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.hasUpdatedParamDefaults())
        updateParameterDefaults();
}


void ImogenAudioProcessorEditor::updateNumVoicesCombobox (const int newNumVoices)
{
    midiPanel.updateNumVoicesCombobox(newNumVoices);
}


inline void ImogenAudioProcessorEditor::newPresetSelected()
{
    auto preset = selectPreset.getItemText (selectPreset.getSelectedId());
    
    audioProcessor.loadPreset (preset);
}


inline void ImogenAudioProcessorEditor::makePresetMenu (juce::ComboBox& box)
{
    int id = 1;
    
    for (juce::DirectoryEntry entry : juce::RangedDirectoryIterator(audioProcessor.getPresetsFolder(), false))
    {
        box.addItem(entry.getFile().getFileName(), id);
        ++id;
    }
}


void ImogenAudioProcessorEditor::updateParameterDefaults()
{
    ioPanel.updateParameterDefaults();
    midiPanel.updateParameterDefaults();
}
