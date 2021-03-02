/*
    This file defines a user interface control panel containing controls linked to the audio processor's built-in limiter.
    Parent file: IOControlPanel.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/LookAndFeel.h"


namespace bav

{
    

class LimiterControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    LimiterControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
        audioProcessor(p), lookAndFeel(l),
        limiterThreshLink (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterThresh", limiterThresh)),
        limiterReleaseLink(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterRelease", limiterRelease)),
        limiterToggleLink (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "limiterIsOn", limiterToggle))
    {
        // threshold
        limiterThresh.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        limiterThresh.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
        addAndMakeVisible(limiterThresh);
        limiterThresh.setValue(audioProcessor.limiterThresh->get(), juce::NotificationType::dontSendNotification);
        limiterThresh.onValueChange = [this] { audioProcessor.updateLimiter(); };
        lookAndFeel.initializeLabel(threshLabel, "Threshold");
        addAndMakeVisible(threshLabel);
        
        // release
        limiterRelease.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
        limiterRelease.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(limiterRelease);
        limiterRelease.setValue(audioProcessor.limiterRelease->get(), juce::NotificationType::dontSendNotification);
        limiterRelease.onValueChange = [this] { audioProcessor.updateLimiter(); };
        lookAndFeel.initializeLabel(releaseLabel, "Release time");
        addAndMakeVisible(releaseLabel);
        
        // toggle
        limiterToggle.setButtonText("Output limiter");
        addAndMakeVisible(limiterToggle);
        limiterToggle.onClick = [this] { audioProcessor.updateLimiter(); };
        
        juce::Button::ButtonState initState = bav::GuiUtils::buttonStateFromBool (audioProcessor.limiterToggle->get());
        limiterToggle.setState (initState);
    }
    
    ~LimiterControlPanel() override
    {
        setLookAndFeel(nullptr);
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> limiterThreshLink;
    
    // release time, in ms
    juce::Slider limiterRelease;
    juce::Label releaseLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> limiterReleaseLink;
    
    // toggle limiter on/off
    juce::ToggleButton limiterToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterToggleLink;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterControlPanel)
};


}  // namespace
