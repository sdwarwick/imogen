/*
 ==============================================================================
 
 MidiControlPanel.h
 Created: 29 Nov 2020 5:31:17pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/LookAndFeel.h"


//==============================================================================
/*
 */
class MidiControlPanel  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    MidiControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~MidiControlPanel() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // MIDI-triggered ADSR
    Slider adsrAttack;
    Label attackLabel;
    Slider adsrDecay;
    Label decayLabel;
    Slider adsrSustain;
    Label sustainLabel;
    Slider adsrRelease;
    Label releaseLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackLink;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> decayLink;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sustainLink;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseLink;
    ToggleButton adsrOnOff;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> adsrOnOffLink;
    
    // Midi latch
    ToggleButton latchToggle;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> latchToggleLink;
    
    // interval lock
    ToggleButton intervalLock;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> intervalLockLink;
    
    // stereo width of harmony output
    Slider stereoWidth;
    Label stereowidthLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
    // sets threshold for lowest panned midiPitch. set to 0 to turn off (pan all notes); set to 127 to bypass panning entirely (pan no notes)
    Slider lowestPan;
    Label lowestpanLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowestPanLink;
    
    // MIDI velocity sensitivity dial
    Slider midiVelocitySens;
    Label midivelocitysensLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midiVelocitySensLink;
    
    // MIDI pitch bend range up/down controls
    ComboBox pitchBendUp;
    Label pitchbendUpLabel;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendUpLink;
    ComboBox pitchBendDown;
    Label pitchbendDownLabel;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendDownLink;
    
    // voice stealing on/off toggle
    ToggleButton voiceStealing;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> voiceStealingLink;
    
    // kill all MIDI button
    TextButton midiKill;
    
    // set quick kill ms - amount of time it takes for voices to ramp to 0 if "kill all" button is pressed
    Slider quickKillMs;
    Label quickKillmsLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> quickKillMsLink;
    
    // set concert pitch in Hz
    Slider concertPitch;
    Label concertPitchLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> concertPitchLink;
    
    // # of harmony voices
    ComboBox numberOfVoices;
    Label numVoicesLabel;
    
    // pedal pitch
    ToggleButton pedalPitchToggle;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> pedalPitchToggleLink;
    Slider pedalPitchThreshold;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> pedalPitchThreshLink;
    Label pedalPitchThreshLabel;
    ComboBox pedalPitchInterval;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pedalPitchIntervalLink;
    Label pedalPitchIntervalLabel;
    
    // descant
    ToggleButton descantToggle;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> descantToggleLink;
    Slider descantThreshold;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> descantThresholdLink;
    Label descantThreshLabel;
    ComboBox descantInterval;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> descantIntervalLink;
    Label descantIntervalLabel;
    
    void updateNumVoicesCombobox(const int newNumVoices);
    
private:
    
    void buildIntervalCombobox(ComboBox& box);
    void buildVoicesCombobox(ComboBox& box);
    
    Button::ButtonState buttonStateFromBool (const bool isOn);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiControlPanel)
};
