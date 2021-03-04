/*
    This file defines a user interface control panel containing controls linked to the audio processor's various I/O controls, options, and functions.
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "LimiterControlPanel.h"
#include "../../Source/GUI/LookAndFeel.h"


namespace bav

{
    

class IOControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    IOControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~IOControlPanel() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
private:
    
    // dry vox (modulator) pan (in midiPan)
    juce::Slider dryPan;
    juce::Label drypanLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryPanLink;
    
    // modulator input gain (gain applied before mod signal is sent into harmony algorithm)
    juce::Slider inputGain;
    juce::Label inputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
    
    // master dry/wet
    juce::Slider masterDryWet;
    juce::Label drywetLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterDryWetLink;
    
    // dry gain
    juce::Slider dryGain;
    juce::Label dryGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryGainLink;
    
    // wet gain
    juce::Slider wetGain;
    juce::Label wetGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetGainLink;
    
    // master output gain
    juce::Slider outputGain;
    juce::Label outputgainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
    
    LimiterControlPanel limiterPanel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOControlPanel)
};


}  // namespace
