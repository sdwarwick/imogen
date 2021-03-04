/*
    This file defines a user interface control panel containing controls linked to the harmonizer's various MIDI settings, controls, and functions.
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginSources/PluginProcessor.h"
#include "GUI/LookAndFeel.h"


namespace bav

{

    
class MidiControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    MidiControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~MidiControlPanel() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void updateNumVoicesCombobox(const int newNumVoices);
    
    
private:
    
    // MIDI-triggered ADSR
    juce::Slider adsrAttack;
    juce::Label attackLabel;
    juce::Slider adsrDecay;
    juce::Label decayLabel;
    juce::Slider adsrSustain;
    juce::Label sustainLabel;
    juce::Slider adsrRelease;
    juce::Label releaseLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseLink;
    juce::ToggleButton adsrOnOff;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> adsrOnOffLink;
    
    // Midi latch
    juce::ToggleButton latchToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> latchToggleLink;
    
    // interval lock
    juce::ToggleButton intervalLock;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> intervalLockLink;
    
    // stereo width of harmony output
    juce::Slider stereoWidth;
    juce::Label stereowidthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
    // sets threshold for lowest panned midiPitch. set to 0 to turn off (pan all notes); set to 127 to bypass panning entirely (pan no notes)
    juce::Slider lowestPan;
    juce::Label lowestpanLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowestPanLink;
    
    // MIDI velocity sensitivity dial
    juce::Slider midiVelocitySens;
    juce::Label midivelocitysensLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midiVelocitySensLink;
    
    // MIDI pitch bend range up/down controls
    juce::ComboBox pitchBendUp;
    juce::Label pitchbendUpLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendUpLink;
    juce::ComboBox pitchBendDown;
    juce::Label pitchbendDownLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendDownLink;
    
    // voice stealing on/off toggle
    juce::ToggleButton voiceStealing;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> voiceStealingLink;
    
    // kill all MIDI button
    juce::TextButton midiKill;
    
    // set quick kill ms - amount of time it takes for voices to ramp to 0 if "kill all" button is pressed
    juce::Slider quickKillMs;
    juce::Label quickKillmsLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> quickKillMsLink;
    
    // set concert pitch in Hz
    juce::Slider concertPitch;
    juce::Label concertPitchLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> concertPitchLink;
    
    // # of harmony voices
    juce::ComboBox numberOfVoices;
    juce::Label numVoicesLabel;
    
    // pedal pitch
    juce::ToggleButton pedalPitchToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pedalPitchToggleLink;
    juce::Slider pedalPitchThreshold;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pedalPitchThreshLink;
    juce::Label pedalPitchThreshLabel;
    juce::ComboBox pedalPitchInterval;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> pedalPitchIntervalLink;
    juce::Label pedalPitchIntervalLabel;
    
    // descant
    juce::ToggleButton descantToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> descantToggleLink;
    juce::Slider descantThreshold;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> descantThresholdLink;
    juce::Label descantThreshLabel;
    juce::ComboBox descantInterval;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> descantIntervalLink;
    juce::Label descantIntervalLabel;
    
    void buildIntervalCombobox(juce::ComboBox& box);
    void buildVoicesCombobox(juce::ComboBox& box);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiControlPanel)
};


}  // namespace
