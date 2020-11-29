#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "PluginProcessor.h"


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
	
	// elements for control of harmony ADSR's
	Slider adsrAttack;
	Slider adsrDecay;
	Slider adsrSustain;
	Slider adsrRelease;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> decayLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sustainLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseLink;
	// on/off toggle
	ToggleButton adsrOnOff;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> adsrOnOffLink;
	
	// stereo width of harmony output
	Slider stereoWidth;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
	// sets threshold for lowest panned midiPitch. set to 0 to turn off (pan all notes); set to 127 to bypass panning entirely (pan no notes)
	Slider lowestPan;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowestPanLink;
	
	// dry vox (modulator) pan (in midiPan)
	Slider dryPan;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> dryPanLink;
	
	// master dry/wet
	Slider masterDryWet;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> masterDryWetLink;
	
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
	
	// modulator input gain (gain applied before mod signal is sent into harmony algorithm
	Slider inputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
	
	// master output gain
	Slider outputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
	
	// midi latch on/off toggle
	ToggleButton midiLatch;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> midiLatchLink;
	
	// voice stealing on/off toggle
	ToggleButton voiceStealing;
	std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> voiceStealingLink;
	
	// kill all MIDI button
	TextButton midiKill;
	
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
