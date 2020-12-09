/*
  ==============================================================================

    LimiterControlPanel.h
    Created: 8 Dec 2020 12:42:36am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GlobalDefinitions.h"

//==============================================================================
/*
*/
class LimiterControlPanel  : public juce::Component
{
public:
	ImogenAudioProcessor& audioProcessor;
	LimiterControlPanel(ImogenAudioProcessor& p): audioProcessor(p),
		limiterThreshLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterThresh", limiterThresh)),
		limiterReleaseLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterRelease", limiterRelease)),
		limiterToggleLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "limiterIsOn", limiterToggle))
    {
		// threshold
		{
			limiterThresh.setSliderStyle(Slider::SliderStyle::LinearVertical);
			limiterThresh.setRange(-60.0f, 0.0f);
			limiterThresh.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
			addAndMakeVisible(limiterThresh);
			limiterThresh.setValue(-2.0f);
			initializeLabel(threshLabel, "Threshold");
			addAndMakeVisible(threshLabel);
		}
		
		// release
		{
			limiterRelease.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
			limiterRelease.setRange(1, 250);
			limiterRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(limiterRelease);
			limiterRelease.setValue(10);
			initializeLabel(releaseLabel, "Release time");
			addAndMakeVisible(releaseLabel);
		}
		
		// toggle
		{
			limiterToggle.setButtonText("Output limiter");
			addAndMakeVisible(limiterToggle);
			limiterToggle.triggerClick();
		}
    }

    ~LimiterControlPanel() override
    {
    }
	
	// threshold, in dBFS
	Slider limiterThresh;
	Label threshLabel;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterThreshLink;
	
	// release time, in ms
	Slider limiterRelease;
	Label releaseLabel;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterReleaseLink;
	
	// toggle limiter on/off
	ToggleButton limiterToggle;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> limiterToggleLink;

    void paint (juce::Graphics& g) override
    {
		g.fillAll (juce::Colours::steelblue);
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
	
	void initializeLabel(Label& label, String labelText)
	{
		label.setFont(juce::Font(14.0f, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		label.setColour(juce::Label::textColourId, juce::Colours::black);
		label.setText(labelText, juce::dontSendNotification);
	};
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterControlPanel)
};
