/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginProcessor.h
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "GUI/MidiControlPanel/MidiControlPanel.h"
#include "GUI/IOControlPanel/IOControlPanel.h"


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
    
public:
    
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
    
    ~ImogenAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    void updateNumVoicesCombobox (const int newNumVoices);
    
    
private:
    
    ImogenAudioProcessor& audioProcessor;
    
    inline void newPresetSelected();
    
    inline void makePresetMenu (juce::ComboBox& box);
    
    bav::ImogenLookAndFeel lookAndFeel;
    
    bav::MidiControlPanel midiPanel;
    bav::IOControlPanel ioPanel;
    
    juce::ComboBox selectPreset;
    
    juce::ComboBox modulatorInputSource;

    void updateParameterDefaults();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
