/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginProcessor.h
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/StaffDisplay/StaffDisplay.h"
#include "../../Source/GUI/MidiControlPanel/MidiControlPanel.h"
#include "../../Source/GUI/IOControlPanel/IOControlPanel.h"
#include "../../Source/GUI/LookAndFeel.h"
#include "../../Source/GUI/HelpScreen/HelpScreen.h"
#include "../../Source/GUI/touchOnceAndForgetSettingsPanel.h"

#if ! IMOGEN_ONLY_BUILDING_STANDALONE
  #include "../../Source/GUI/EnableSidechainWarning.h"
#endif


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
    
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
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
    
    bav::ImogenLookAndFeel lookAndFeel;
    bav::ImogenLookAndFeel::Skin currentSkin;
    bav::ImogenLookAndFeel::Skin prevSkin;
    
    juce::ComboBox selectSkin;
    juce::Label skinLabel;
    
    bav::MidiControlPanel midiPanel;
    bav::IOControlPanel ioPanel;
    
    bav::HelpScreen helpScreen;
    juce::TextButton helpButton;
    
    juce::ComboBox selectPreset;
    
    juce::ComboBox modulatorInputSource;

    void changeModulatorInputSource();
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    bav::EnableSidechainWarning sidechainWarning;
    bool sidechainWarningShowing;
#endif
    
    bav::StaffDisplay staffDisplay;
    
    // bav::Imogen_touchOnceAndForgetSettings touchOnceSettings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
