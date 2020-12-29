#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "PluginProcessor.h"
#include "StaffDisplay.h"
#include "MidiControlPanel.h"
#include "IOControlPanel.h"
#include "LookAndFeel.h"
#include "HelpScreen.h"

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
	
	void updateNumVoicesCombobox(const int newNumVoices);
	
//==============================================================================

private:
    
    ImogenAudioProcessor& audioProcessor;
	
	ImogenLookAndFeel lookAndFeel;
	ImogenLookAndFeel::Skin currentSkin;
	ImogenLookAndFeel::Skin prevSkin;
	
	ComboBox selectSkin;
	Label skinLabel;
	void skinSelectorChanged();
	
	MidiControlPanel midiPanel;
	IOControlPanel ioPanel;
	StaffDisplay staffDisplay;
	
	HelpScreen helpScreen;
	TextButton helpButton;
	void helpButtonClicked();
	bool viewHelp;  // bool to control visibility of help/documentation screen
	
	ComboBox selectPreset;
	void newPresetSelected();
	void makePresetMenu(ComboBox& box);
	
	Label pitchTester;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
