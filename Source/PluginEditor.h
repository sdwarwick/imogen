#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "PluginProcessor.h"
#include "StaffDisplay.h"
#include "MidiControlPanel.h"
#include "IOControlPanel.h"
#include "LimiterControlPanel.h"

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
	
	
	// array to store currently active harmony pitches
	Array<int> currentPitches;
	
	MidiControlPanel midiPanel;
	IOControlPanel ioPanel;
	LimiterControlPanel limiterPanel;
	StaffDisplay staffDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
