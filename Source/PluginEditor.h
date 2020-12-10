#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "PluginProcessor.h"
#include "StaffDisplay.h"
#include "MidiControlPanel.h"
#include "IOControlPanel.h"
#include "HelpScreen.h"
#include "LookAndFeel.h"

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
	
	ImogenLookAndFeel lookAndFeel;
	void initializeLookAndFeel(ImogenLookAndFeel& lookAndFeel);
	
	MidiControlPanel midiPanel;
	IOControlPanel ioPanel;
	StaffDisplay staffDisplay;
	
	HelpScreen helpScreen;
	
	bool viewHelp;  // bool to control visibility of help/documentation screen

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
