/*
    This file defines a user interface control panel containing controls linked to the audio processor's built-in limiter.
    Parent file: IOControlPanel.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginSources/PluginProcessor.h"
#include "GUI/LookAndFeel.h"


namespace bav

{
    
    using APVTS = juce::AudioProcessorValueTreeState;
    

class LimiterControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    LimiterControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
        audioProcessor(p), lookAndFeel(l),
        limiterThreshLink (std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "limiterThresh", limiterThresh)),
        limiterReleaseLink(std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "limiterRelease", limiterRelease)),
        limiterToggleLink (std::make_unique<APVTS::ButtonAttachment>(audioProcessor.tree, "limiterIsOn", limiterToggle))
    {
        lookAndFeel.initializeSlider (limiterThresh, juce::Slider::SliderStyle::LinearVertical, audioProcessor.getLimiterThresh(),
                                      audioProcessor.getDefaultLimiterThresh(), true);
        addAndMakeVisible(limiterThresh);
        lookAndFeel.initializeLabel(threshLabel, "Threshold");
        addAndMakeVisible(threshLabel);
        
        lookAndFeel.initializeSlider (limiterRelease, juce::Slider::SliderStyle::LinearBarVertical, audioProcessor.getLimiterRelease(),
                                      audioProcessor.getDefaultLimiterRelease(), true);
        addAndMakeVisible(limiterRelease);
        lookAndFeel.initializeLabel(releaseLabel, "Release time");
        addAndMakeVisible(releaseLabel);
        
        limiterToggle.setButtonText("Output limiter");
        addAndMakeVisible(limiterToggle);
        limiterToggle.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsLimiterOn()));
    }
    
    ~LimiterControlPanel() override
    {
        this->setLookAndFeel(nullptr);
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::insetPanelColourId));
    }
    
    void resized() override
    {
        limiterToggle.setBounds(90, 0, 200, 35);
        
        threshLabel.setBounds  (45, 25, 75, 35);
        limiterThresh.setBounds(60, 45, 50, 95);
        
        releaseLabel.setBounds  (150, 25, 100, 35);
        limiterRelease.setBounds(180, 55, 35, 35);
    }
    
    
private:
    
    // threshold, in dBFS
    juce::Slider limiterThresh;
    juce::Label threshLabel;
    std::unique_ptr<APVTS::SliderAttachment> limiterThreshLink;
    
    // release time, in ms
    juce::Slider limiterRelease;
    juce::Label releaseLabel;
    std::unique_ptr<APVTS::SliderAttachment> limiterReleaseLink;
    
    // toggle limiter on/off
    juce::ToggleButton limiterToggle;
    std::unique_ptr<APVTS::ButtonAttachment> limiterToggleLink;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterControlPanel)
};


}  // namespace
