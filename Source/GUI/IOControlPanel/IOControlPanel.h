/*
 ==============================================================================
 
 IOControlPanel.h
 Created: 7 Dec 2020 11:18:06pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/IOControlPanel/LimiterControlPanel.h"
#include "../../Source/GUI/LookAndFeel.h"


//==============================================================================
/*
 */
class IOControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    IOControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~IOControlPanel() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // dry vox (modulator) pan (in midiPan)
    juce::Slider dryPan;
    juce::Label drypanLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryPanLink;
    
    // master dry/wet
    juce::Slider masterDryWet;
    juce::Label drywetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterDryWetLink;
    
    // modulator input gain (gain applied before mod signal is sent into harmony algorithm)
    juce::Slider inputGain;
    juce::Label inputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
    
    // master output gain
    juce::Slider outputGain;
    juce::Label outputgainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
    
    // set input channel [plugin only accepts a single mono input source]
    juce::Slider inputChannel;
    juce::Label inputChannelLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputChannelLink;
    
    LimiterControlPanel limiterPanel;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOControlPanel)
};
