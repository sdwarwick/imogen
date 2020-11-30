#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "PluginProcessor.h"
#include "StaffDisplay.h"
#include "MidiControlPanel.h"


//==============================================================================

class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
									public Timer
{
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor&);
    ~ImogenAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	void timerCallback() override;
	
//==============================================================================

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ImogenAudioProcessor& audioProcessor;
	
	
	
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
	
	// array to store currently active harmony pitches
	Array<int> currentPitches;
	
	MidiControlPanel midiPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
