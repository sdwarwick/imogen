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
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> attackLink;
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> decayLink;
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> sustainLink;
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> releaseLink;
	Slider adsrAttack;
	Slider adsrDecay;
	Slider adsrSustain;
	Slider adsrRelease;
	
	// stereo width of harmony output
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> stereoWidthLink;
	Slider stereoWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
