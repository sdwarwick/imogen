/*
  ==============================================================================

    MidiControlPanel.h
    Created: 29 Nov 2020 5:31:17pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class MidiControlPanel  : public juce::Component
{
public:
    MidiControlPanel(ImogenAudioProcessor& p);
    ~MidiControlPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	// elements for control of harmony ADSR's
	Slider adsrAttack;
	Slider adsrDecay;
	Slider adsrSustain;
	Slider adsrRelease;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> decayLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sustainLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseLink;
	// adsr on/off toggle
	ToggleButton adsrOnOff;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> adsrOnOffLink;
	
	// stereo width of harmony output
	Slider stereoWidth;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
	// sets threshold for lowest panned midiPitch. set to 0 to turn off (pan all notes); set to 127 to bypass panning entirely (pan no notes)
	Slider lowestPan;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowestPanLink;
	
	// MIDI velocity sensitivity dial
	Slider midiVelocitySens;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midiVelocitySensLink;
	
	// MIDI pitch bend range up/down controls
	ComboBox pitchBendUp;
	std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendUpLink;
	ComboBox pitchBendDown;
	std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendDownLink;
	
	// MIDI PEDAL PITCH: doubles the lowest played pitch an octave lower, if lowest active pitch is below a certain threshold
	// toggle on/off
	ToggleButton pedalPitch;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> pedalPitchLink;
	// set the highest pitch that will be doubled @ 8vb
	Slider pedalPitchThresh;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> pedalPitchThreshLink;
	
	// midi latch on/off toggle
	ToggleButton midiLatch;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> midiLatchLink;
	
	// voice stealing on/off toggle
	ToggleButton voiceStealing;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> voiceStealingLink;
	
	// kill all MIDI button
	TextButton midiKill;

private:
	ImogenAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiControlPanel)
};
