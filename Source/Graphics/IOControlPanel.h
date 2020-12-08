/*
  ==============================================================================

    IOControlPanel.h
    Created: 7 Dec 2020 11:18:06pm
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
class IOControlPanel  : public juce::Component
{
public:
    IOControlPanel(ImogenAudioProcessor& p);
    ~IOControlPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	// dry vox (modulator) pan (in midiPan)
	Slider dryPan;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> dryPanLink;
	
	// master dry/wet
	Slider masterDryWet;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> masterDryWetLink;
	
	// modulator input gain (gain applied before mod signal is sent into harmony algorithm
	Slider inputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
	
	// master output gain
	Slider outputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
	
	// set input channel [plugin only accepts a single mono input source]
	Slider inputChannel;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputChannelLink;
	
	// output limiter threshold, in dBFS
	Slider limiterThresh;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterThreshLink;
	// limiter release time, in ms
	Slider limiterRelease;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> limiterReleaseLink;
	// toggle limiter on/off
	ToggleButton limiterToggle;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> limiterToggleLink;

private:
	ImogenAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOControlPanel)
};
