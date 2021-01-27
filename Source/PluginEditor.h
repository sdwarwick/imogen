#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Graphics/StaffDisplay.h"
#include "Graphics/MidiControlPanel.h"
#include "Graphics/IOControlPanel.h"
#include "Graphics/LookAndFeel.h"
#include "Graphics/HelpScreen.h"
#include "Graphics/EnableSidechainWarning.h"

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
    
    void skinSelectorChanged();
    
    void helpButtonClicked();
    
    void newPresetSelected();
    
    void makePresetMenu(ComboBox& box);
    
    ImogenLookAndFeel lookAndFeel;
    ImogenLookAndFeel::Skin currentSkin;
    ImogenLookAndFeel::Skin prevSkin;
    
    ComboBox selectSkin;
    Label skinLabel;
    
    MidiControlPanel midiPanel;
    IOControlPanel ioPanel;
    StaffDisplay staffDisplay;
    
    HelpScreen helpScreen;
    TextButton helpButton;
    
    bool viewHelp;  // bool to control visibility of help/documentation screen
    
    ComboBox selectPreset;
    
    ComboBox modulatorInputSource;
    
    PluginHostType host;
    
    bool sidechainWarningShowing;
    
    void changeModulatorInputSource();
    
    EnableSidechainWarning sidechainWarning;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
