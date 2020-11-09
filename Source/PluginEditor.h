#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener
{
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor&);
    ~ImogenAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	void sliderValueChanged (Slider* slider) override;
	
//==============================================================================

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ImogenAudioProcessor& audioProcessor;
	
	// elements for control of harmony ADSR's
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> decayLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sustainLink;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseLink;
	Slider adsrAttack;
	Slider adsrDecay;
	Slider adsrSustain;
	Slider adsrRelease;
	
	// stereo width of harmony output
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
	Slider stereoWidth;
	
	// MIDI velocity sensitivity dial
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> midiVelocitySensLink;
	Slider midiVelocitySens;
	
	// MIDI pitch bend range up/down controls
	ComboBox pitchBendUp;
	std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendUpLink;
	ComboBox pitchBendDown;
	std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> pitchBendDownLink;
	
	// modulator input gain (gain applied before mod signal is sent into harmony algorithm
	Slider inputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> inputGainLink;
	
	// master output gain
	Slider outputGain;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> outputGainLink;
	
	// midi latch on/off toggle
	ToggleButton midiLatch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
