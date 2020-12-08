/*
  ==============================================================================

    IOControlPanel.cpp
    Created: 7 Dec 2020 11:18:06pm
    Author:  Ben Vining

  ==============================================================================
*/

#include <JuceHeader.h>
#include "IOControlPanel.h"

//==============================================================================
IOControlPanel::IOControlPanel(ImogenAudioProcessor& p): audioProcessor(p)
{
	// dry pan
	{
		dryPan.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		dryPan.setRange(0, 127);
		dryPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&dryPan);
		dryPanLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "dryPan", dryPan);
		dryPan.setValue(64);
	}
	
	// master dry/wet
	{
		masterDryWet.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		masterDryWet.setRange(0, 100);
		masterDryWet.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&masterDryWet);
		masterDryWetLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "masterDryWet", masterDryWet);
		masterDryWet.setValue(100);
	}
	
	// input gain
	{
		inputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
		inputGain.setRange(-60.0f, 0.0f);
		inputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&inputGain);
		inputGainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "inputGain", inputGain);
		inputGain.setValue(0.0f);
	}
	
	// output gain
	{
		outputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
		outputGain.setRange(-60.0f, 0.0f);
		outputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&outputGain);
		outputGainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "outputGain", outputGain);
		outputGain.setValue(-4.0f);
	}
	
	// set input channel
	{
		inputChannel.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
		inputChannel.setRange(0, 99);
		inputChannel.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&inputChannel);
		inputChannelLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "inputChan", inputChannel);
		inputChannel.setValue(0);
	}
	
	// output limiter
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
}

IOControlPanel::~IOControlPanel()
{
}

void IOControlPanel::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("IOControlPanel", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void IOControlPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
