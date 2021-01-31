/*
 ==============================================================================
 
 IOControlPanel.h
 Created: 7 Dec 2020 11:18:06pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
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
    Slider dryPan;
    Label drypanLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> dryPanLink;
    
    // master dry/wet
    Slider masterDryWet;
    Label drywetLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> masterDryWetLink;
    
    // modulator input gain (gain applied before mod signal is sent into harmony algorithm)
    Slider inputGain;
    Label inputGainLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
    
    // master output gain
    Slider outputGain;
    Label outputgainLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
    
    // set input channel [plugin only accepts a single mono input source]
    Slider inputChannel;
    Label inputChannelLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputChannelLink;
    
    LimiterControlPanel limiterPanel;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOControlPanel)
};
