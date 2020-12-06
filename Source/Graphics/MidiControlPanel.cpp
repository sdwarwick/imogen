/*
  ==============================================================================

    MidiControlPanel.cpp
    Created: 29 Nov 2020 5:31:17pm
    Author:  Ben Vining

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MidiControlPanel.h"

//==============================================================================
MidiControlPanel::MidiControlPanel(ImogenAudioProcessor& p): audioProcessor(p)
{
	// ADSR
	{
		adsrAttack.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		adsrAttack.setRange(0.01f, 1.0f);
		adsrAttack.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
		adsrAttack.setNumDecimalPlacesToDisplay(5);
		addAndMakeVisible(&adsrAttack);
		attackLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrAttack", adsrAttack);
		adsrAttack.setValue(0.035f);
		
		
		adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		adsrDecay.setRange(0.01f, 1.0f);
		adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&adsrDecay);
		decayLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrDecay", adsrDecay);
		adsrDecay.setValue(0.06f);
		
		adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		adsrSustain.setRange(0.01f, 1.0f);
		adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&adsrSustain);
		sustainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrSustain", adsrSustain);
		adsrSustain.setValue(0.8f);
		
		adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		adsrRelease.setRange(0.01f, 1.0f);
		adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&adsrRelease);
		releaseLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrRelease", adsrRelease);
		adsrRelease.setValue(0.1f);
		
		adsrOnOff.setButtonText("ADSR on/off");
	//	addAndMakeVisible(&adsrOnOff);
		adsrOnOffLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "adsrOnOff", adsrOnOff);
		adsrOnOff.setToggleState(true, true);
	}
	
	// stereo width
	{
		stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		stereoWidth.setRange(0, 100);
		stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&stereoWidth);
		stereoWidthLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "stereoWidth", stereoWidth);
		stereoWidth.setValue(100);
	}
	// lowest panned midiPitch threshold
	{
		lowestPan.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
		lowestPan.setRange(0, 127);
		lowestPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&lowestPan);
		lowestPanLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "lowestPan", lowestPan);
		lowestPan.setValue(0);
	}
	// MIDI velocity sensitivity
	{
		midiVelocitySens.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
		midiVelocitySens.setRange(0,100);
		midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	//	addAndMakeVisible(&midiVelocitySens);
		midiVelocitySensLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens);
		midiVelocitySens.setValue(100);
	}
	
	// pitch bend settings
	{
		pitchBendUp.addItem("Minor Second", 1);
		pitchBendUp.addItem("Major Second", 2);
		pitchBendUp.addItem("Minor Third", 3);
		pitchBendUp.addItem("Major Third", 4);
		pitchBendUp.addItem("Perfect Fourth", 5);
		pitchBendUp.addItem("Aug Fourth/Dim Fifth", 6);
		pitchBendUp.addItem("Perfect Fifth", 7);
		pitchBendUp.addItem("Minor Sixth", 8);
		pitchBendUp.addItem("Major Sixth", 9);
		pitchBendUp.addItem("Minor Seventh", 10);
		pitchBendUp.addItem("Major Seventh", 11);
		pitchBendUp.addItem("Octave", 12);
	//	addAndMakeVisible(pitchBendUp);
		pitchBendUpLink = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendUpRange", pitchBendUp);
		pitchBendUp.setSelectedId(2);
		
		pitchBendDown.addItem("Minor Second", 1);
		pitchBendDown.addItem("Major Second", 2);
		pitchBendDown.addItem("Minor Third", 3);
		pitchBendDown.addItem("Major Third", 4);
		pitchBendDown.addItem("Perfect Fourth", 5);
		pitchBendDown.addItem("Aug Fourth/Dim Fifth", 6);
		pitchBendDown.addItem("Perfect Fifth", 7);
		pitchBendDown.addItem("Minor Sixth", 8);
		pitchBendDown.addItem("Major Sixth", 9);
		pitchBendDown.addItem("Minor Seventh", 10);
		pitchBendDown.addItem("Major Seventh", 11);
		pitchBendDown.addItem("Octave", 12);
	//	addAndMakeVisible(pitchBendDown);
		pitchBendDownLink = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendDownRange", pitchBendDown);
		pitchBendDown.setSelectedId(2);
	}
	
	// MIDI PEDAL PITCH settings
	{
		// toggle on/off
		{
			pedalPitch.setButtonText("Pedal pitch on/off");
		//	addAndMakeVisible(&pedalPitch);
			pedalPitchLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "pedalPitchToggle", pedalPitch);
			pedalPitch.setToggleState(false, true);
		}
		// threshold
		{
			pedalPitchThresh.setSliderStyle(Slider::SliderStyle::LinearVertical);
			pedalPitchThresh.setRange(0, 127);
			pedalPitchThresh.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&pedalPitchThresh);
			pedalPitchThreshLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "pedalPitchThresh", pedalPitchThresh);
			pedalPitchThresh.setValue(127);
		}
	}
	
	// midi latch toggle
	{
		midiLatch.setButtonText("MIDI latch on/off");
	//	addAndMakeVisible(&midiLatch);
		midiLatchLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "midiLatch", midiLatch);
		midiLatch.setToggleState(false, true);
	}
	
	// voice stealing on/off
	{
		voiceStealing.setButtonText("Voice stealing");
	//	addAndMakeVisible(&voiceStealing);
		voiceStealingLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "voiceStealing", voiceStealing);
		voiceStealing.setToggleState(false, true);
	}
	
	// kill all midi button
	{
		midiKill.setButtonText("Kill all MIDI");
		midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
	//	addAndMakeVisible(&midiKill);
	}

}

MidiControlPanel::~MidiControlPanel()
{
}

void MidiControlPanel::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
	g.setColour(juce::Colours::white);
}

void MidiControlPanel::resized()
{
	adsrAttack.setBounds(0, 0, 75, 75);
//	adsrDecay.setBounds(90, 10, 75, 75);
//	adsrSustain.setBounds(170, 10, 75, 75);
//	adsrRelease.setBounds(250, 10, 75, 75);
//	stereoWidth.setBounds(10, 100, 75, 75);
//	midiVelocitySens.setBounds(90, 100, 75, 75);
//	pitchBendUp.setBounds(10, 250, 350, 25);
//	pitchBendDown.setBounds(375, 250, 350, 25);
	//pedalPitch.setBounds();
	//pedalPitchThresh.setBounds();
//	midiLatch.setBounds(10, 250, 75, 35);
//	voiceStealing.setBounds(10, 375, 75, 25);
//	midiKill.setBounds(100, 250, 75, 35);
}
