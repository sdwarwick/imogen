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
		// attack
		{
			adsrAttack.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrAttack.setRange(0.01f, 1.0f);
			adsrAttack.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
			addAndMakeVisible(adsrAttack);
			attackLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrAttack", adsrAttack);
			adsrAttack.setValue(0.035f);
			attackLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			attackLabel.setJustificationType(juce::Justification::centred);
			attackLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			attackLabel.setText("Attack", juce::dontSendNotification);
			addAndMakeVisible(attackLabel);
		}
		
		// decay
		{
			adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrDecay.setRange(0.01f, 1.0f);
			adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(adsrDecay);
			decayLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrDecay", adsrDecay);
			adsrDecay.setValue(0.06f);
			decayLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			decayLabel.setJustificationType(juce::Justification::centred);
			decayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			decayLabel.setText("Decay", juce::dontSendNotification);
			addAndMakeVisible(decayLabel);
		}
		
		// sustain
		{
			adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrSustain.setRange(0.01f, 1.0f);
			adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(adsrSustain);
			sustainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrSustain", adsrSustain);
			adsrSustain.setValue(0.8f);
			sustainLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			sustainLabel.setJustificationType(juce::Justification::centred);
			sustainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			sustainLabel.setText("Sustain", juce::dontSendNotification);
			addAndMakeVisible(sustainLabel);
		}
		
		// release
		{
			adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrRelease.setRange(0.01f, 1.0f);
			adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(adsrRelease);
			releaseLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrRelease", adsrRelease);
			adsrRelease.setValue(0.1f);
			releaseLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			releaseLabel.setJustificationType(juce::Justification::centred);
			releaseLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			releaseLabel.setText("Release", juce::dontSendNotification);
			addAndMakeVisible(releaseLabel);
		}
		
		// on/off toggle
		{
			adsrOnOff.setButtonText("MIDI-triggered ADSR");
			addAndMakeVisible(adsrOnOff);
			adsrOnOffLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "adsrOnOff", adsrOnOff);
			adsrOnOff.setToggleState(true, true);
		}
	}
	
	// kill all midi button
	{
		midiKill.setButtonText("Kill all MIDI");
		midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
		addAndMakeVisible(midiKill);
	}
	
	// stereo width
	{
		// stereo width dial
		{
			stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			stereoWidth.setRange(0, 100);
			stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(stereoWidth);
			stereoWidthLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "stereoWidth", stereoWidth);
			stereoWidth.setValue(100);
			stereowidthLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			stereowidthLabel.setJustificationType(juce::Justification::centred);
			stereowidthLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			stereowidthLabel.setText("Stereo width", juce::dontSendNotification);
			addAndMakeVisible(stereowidthLabel);
		}
		
		// lowest panned midiPitch
		{
			lowestPan.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
			lowestPan.setRange(0, 127);
			lowestPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(lowestPan);
			lowestPanLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "lowestPan", lowestPan);
			lowestPan.setValue(0);
			lowestpanLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			lowestpanLabel.setJustificationType(juce::Justification::centred);
			lowestpanLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			lowestpanLabel.setText("Lowest panned MIDI pitch", juce::dontSendNotification);
			addAndMakeVisible(lowestpanLabel);
		}
	}
	
	// MIDI velocity sensitivity
	{
		midiVelocitySens.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
		midiVelocitySens.setRange(0, 100);
		midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		addAndMakeVisible(midiVelocitySens);
		midiVelocitySensLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens);
		midiVelocitySens.setValue(100);
		midivelocitysensLabel.setFont(juce::Font(14.0f, juce::Font::bold));
		midivelocitysensLabel.setJustificationType(juce::Justification::centred);
		midivelocitysensLabel.setColour(juce::Label::textColourId, juce::Colours::white);
		midivelocitysensLabel.setText("MIDI velocity sensitivity", juce::dontSendNotification);
		addAndMakeVisible(midivelocitysensLabel);
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
		addAndMakeVisible(pitchBendUp);
		pitchBendUpLink = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendUpRange", pitchBendUp);
		pitchBendUp.setSelectedId(2);
		pitchbendUpLabel.setFont(juce::Font(14.0f, juce::Font::bold));
		pitchbendUpLabel.setJustificationType(juce::Justification::centred);
		pitchbendUpLabel.setColour(juce::Label::textColourId, juce::Colours::white);
		pitchbendUpLabel.setText("Pitch bend range up", juce::dontSendNotification);
		addAndMakeVisible(pitchbendUpLabel);
		
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
		addAndMakeVisible(pitchBendDown);
		pitchBendDownLink = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendDownRange", pitchBendDown);
		pitchBendDown.setSelectedId(2);
		pitchbendDownLabel.setFont(juce::Font(14.0f, juce::Font::bold));
		pitchbendDownLabel.setJustificationType(juce::Justification::centred);
		pitchbendDownLabel.setColour(juce::Label::textColourId, juce::Colours::white);
		pitchbendDownLabel.setText("Pitch bend range down", juce::dontSendNotification);
		addAndMakeVisible(pitchbendDownLabel);
	}
	
	// MIDI PEDAL PITCH settings
	{
		// toggle on/off
		{
			pedalPitch.setButtonText("MIDI pedal pitch");
			addAndMakeVisible(pedalPitch);
			pedalPitchLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "pedalPitchToggle", pedalPitch);
			pedalPitch.setToggleState(false, true);
		}
		// threshold
		{
			pedalPitchThresh.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
			pedalPitchThresh.setRange(0, 127);
			pedalPitchThresh.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(pedalPitchThresh);
			pedalPitchThreshLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "pedalPitchThresh", pedalPitchThresh);
			pedalPitchThresh.setValue(127);
			pedalpitchThreshLabel.setFont(juce::Font(14.0f, juce::Font::bold));
			pedalpitchThreshLabel.setJustificationType(juce::Justification::centred);
			pedalpitchThreshLabel.setColour(juce::Label::textColourId, juce::Colours::white);
			pedalpitchThreshLabel.setText("Threshold", juce::dontSendNotification);
			addAndMakeVisible(pedalpitchThreshLabel);
		}
	}
	
	// midi latch toggle
	{
		midiLatch.setButtonText("MIDI latch");
		addAndMakeVisible(midiLatch);
		midiLatchLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "midiLatch", midiLatch);
		midiLatch.setToggleState(false, true);
	}
	
	// voice stealing on/off
	{
		voiceStealing.setButtonText("Voice stealing");
		addAndMakeVisible(voiceStealing);
		voiceStealingLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "voiceStealing", voiceStealing);
		voiceStealing.setToggleState(true, true);
	}

}

MidiControlPanel::~MidiControlPanel()
{
}

void MidiControlPanel::paint (juce::Graphics& g)
{
	g.fillAll (juce::Colours::burlywood);
	
	g.setColour(juce::Colours::steelblue);
	
	juce::Rectangle<int> adsrPanel (5, 5, 260, 125);
	g.fillRect(adsrPanel);
	
	juce::Rectangle<int> stereoWidthPanel (5, 135, 190, 100);
	g.fillRect(stereoWidthPanel);
	
	juce::Rectangle<int> midiVelocitysensPanel (200, 135, 95, 100);
	g.fillRect(midiVelocitysensPanel);
	
	juce::Rectangle<int> pitchbendPanel (5, 240, 275, 65);
	g.fillRect(pitchbendPanel);
	
	juce::Rectangle<int> pedalpitchPanel (5, 310, 140, 65);
	g.fillRect(pedalpitchPanel);
}

void MidiControlPanel::resized()
{
	// adsr
	{
		attackLabel.setBounds(0, 25, 75, 35);
		adsrAttack.setBounds(0, 50, 75, 75);
		
		decayLabel.setBounds(65, 25, 75, 35);
		adsrDecay.setBounds(65, 50, 75, 75);
		
		sustainLabel.setBounds(130, 25, 75, 35);
		adsrSustain.setBounds(130, 50, 75, 75);
		
		releaseLabel.setBounds(195, 25, 75, 35);
		adsrRelease.setBounds(195, 50, 75, 75);
		
		adsrOnOff.setBounds(50, 0, 175, 35);
	}
	
	// stereo width
	{
		stereowidthLabel.setBounds(10, 130, 85, 35);
		stereoWidth.setBounds(15, 155, 75, 75);
		
		lowestpanLabel.setBounds(100, 130, 90, 50);
		lowestPan.setBounds(128, 180, 35, 35);
	}
	
	// midi velocity sensitivity
	{
		midivelocitysensLabel.setBounds(205, 140, 85, 35);
		midiVelocitySens.setBounds(230, 180, 35, 35);
	}
	
	// pitch bend
	{
		pitchbendUpLabel.setBounds(10, 235, 125, 35);
		pitchBendUp.setBounds(10, 265, 125, 30);
		
		pitchbendDownLabel.setBounds(145, 235, 125, 35);
		pitchBendDown.setBounds(145, 265, 125, 30);
	}
	
	// pedal pitch
	{
		pedalPitch.setBounds(15, 305, 125, 35);
		
		pedalpitchThreshLabel.setBounds(25, 335, 75, 35);
		pedalPitchThresh.setBounds(100, 335, 35, 35);
	}
	
	midiKill.setBounds(280, 10, 100, 35);
	
	voiceStealing.setBounds(270, 50, 125, 35);
	
	midiLatch.setBounds(270, 80, 125, 35);

}
