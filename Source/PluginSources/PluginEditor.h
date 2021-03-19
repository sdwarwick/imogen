/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginProcessor.h
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "GUI/LookAndFeel/ImogenLookAndFeel.h"
#include "PluginProcessor.h"
#include "GUI/ParameterFetcher.h"


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
    using ids = ImogenAudioProcessor::parameterID;
    
    
public:
    
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
    
    ~ImogenAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    
private:
    
    inline void newPresetSelected();
    
    inline void makePresetMenu (juce::ComboBox& box);
    
    void updateParameterDefaults();
    
    juce::ComboBox selectPreset;
    
    juce::Array< bav::MessageQueue::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
    bav::ImogenLookAndFeel lookAndFeel;
    
    ImogenAudioProcessor& imgnProcessor; // reference to the processor that created this editor
    
    ImogenParameterFetcher params;  // helper object to fetch & convert various parameter values with respect to their normalizable ranges
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
