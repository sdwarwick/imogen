/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/StaffDisplay/StaffDisplay.h"
#include "../../Source/GUI/MidiControlPanel/MidiControlPanel.h"
#include "../../Source/GUI/IOControlPanel/IOControlPanel.h"
#include "../../Source/GUI/LookAndFeel.h"
#include "../../Source/GUI/HelpScreen/HelpScreen.h"
#include "../../Source/GUI/EnableSidechainWarning.h"



class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor&);
    ~ImogenAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    void updateNumVoicesCombobox(const int newNumVoices);
    
    
private:
    
    ImogenAudioProcessor& audioProcessor;
    
    void skinSelectorChanged();
    
    void helpButtonClicked();
    
    void newPresetSelected();
    
    void makePresetMenu(juce::ComboBox& box);
    
    ImogenLookAndFeel lookAndFeel;
    ImogenLookAndFeel::Skin currentSkin;
    ImogenLookAndFeel::Skin prevSkin;
    
    juce::ComboBox selectSkin;
    juce::Label skinLabel;
    
    MidiControlPanel midiPanel;
    IOControlPanel ioPanel;
    StaffDisplay staffDisplay;
    
    HelpScreen helpScreen;
    juce::TextButton helpButton;
    
    bool viewHelp;  // bool to control visibility of help/documentation screen
    
    juce::ComboBox selectPreset;
    
    juce::ComboBox modulatorInputSource;
    
    juce::PluginHostType host;
    
    bool sidechainWarningShowing;
    
    void changeModulatorInputSource();
    
    EnableSidechainWarning sidechainWarning;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
