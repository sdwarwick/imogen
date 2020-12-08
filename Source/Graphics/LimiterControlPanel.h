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
	LimiterControlPanel(ImogenAudioProcessor& p): audioProcessor(p)
    {
		// threshold
		{
			limiterThresh.setSliderStyle(Slider::SliderStyle::LinearVertical);
			limiterThresh.setRange(-60.0f, 0.0f);
			limiterThresh.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			//		addAndMakeVisible(&limiterThresh);
			limiterThreshLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterThresh", limiterThresh);
			limiterThresh.setValue(-2.0f);
		}
		
		// release
		{
			limiterRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			limiterRelease.setRange(1, 250);
			limiterRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			//	addAndMakeVisible(&limiterRelease);
			limiterReleaseLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterRelease", limiterRelease);
			limiterRelease.setValue(10);
		}
		
		// toggle on/off
		{
			limiterToggle.setButtonText("Limiter on/off");
			//		addAndMakeVisible(&limiterToggle);
			limiterToggleLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "limiterIsOn", limiterToggle);
			limiterToggle.triggerClick();
		}
    }

    ~LimiterControlPanel() override
    {
    }
	
	// threshold, in dBFS
	Slider limiterThresh;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterThreshLink;
	
	// release time, in ms
	Slider limiterRelease;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterReleaseLink;
	
	// toggle limiter on/off
	ToggleButton limiterToggle;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> limiterToggleLink;

    void paint (juce::Graphics& g) override
    {
		g.fillAll (juce::Colours::burlywood);
		
		g.setColour(juce::Colours::steelblue);
    }

    void resized() override
    {
		
    }

private:
	ImogenAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterControlPanel)
};
