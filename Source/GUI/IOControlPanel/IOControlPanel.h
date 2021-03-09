/*
    This file defines a user interface control panel containing controls linked to the audio processor's various I/O controls, options, and functions.
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginSources/PluginProcessor.h"
#include "GUI/LookAndFeel.h"


namespace bav

{
    
    using APVTS = juce::AudioProcessorValueTreeState;
    

class IOControlPanel  : public juce::Component
{
public:
    
    IOControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~IOControlPanel() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void updateParameterDefaults();
    
    
private:
    
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    
    // dry vox (modulator) pan (in midiPan)
    juce::Slider dryPan;
    juce::Label drypanLabel;
    std::unique_ptr<APVTS::SliderAttachment> dryPanLink;
    
    // modulator input gain (gain applied before mod signal is sent into harmony algorithm)
    juce::Slider inputGain;
    juce::Label inputGainLabel;
    std::unique_ptr<APVTS::SliderAttachment> inputGainLink;
    
    // master dry/wet
    juce::Slider masterDryWet;
    juce::Label drywetLabel;
    std::unique_ptr<APVTS::SliderAttachment> masterDryWetLink;
    
    // master output gain
    juce::Slider outputGain;
    juce::Label outputgainLabel;
    std::unique_ptr<APVTS::SliderAttachment> outputGainLink;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOControlPanel)
};


}  // namespace
