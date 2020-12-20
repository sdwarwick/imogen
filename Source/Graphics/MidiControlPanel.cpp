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
MidiControlPanel::MidiControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l): audioProcessor(p), lookAndFeel(l),
	attackLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrAttack", adsrAttack)),
	decayLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrDecay", adsrDecay)),
	sustainLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrSustain", adsrSustain)),
	releaseLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrRelease", adsrRelease)),
	adsrOnOffLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "adsrOnOff", adsrOnOff)),
	stereoWidthLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "stereoWidth", stereoWidth)),
	lowestPanLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "lowestPan", lowestPan)),
	midiVelocitySensLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens)),
	pitchBendUpLink(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendUpRange", pitchBendUp)),
	pitchBendDownLink(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendDownRange", pitchBendDown)),
	voiceStealingLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "voiceStealing", voiceStealing)),
	quickKillMsLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "quickKillMs", quickKillMs))
{
	// ADSR
	{
		// attack
		{
			adsrAttack.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrAttack.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
			addAndMakeVisible(adsrAttack);
			adsrAttack.setValue(0.035f);
			lookAndFeel.initializeLabel(attackLabel, "Attack");
			addAndMakeVisible(attackLabel);
		}
		
		// decay
		{
			adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
			addAndMakeVisible(adsrDecay);
			adsrDecay.setValue(0.06f);
			lookAndFeel.initializeLabel(decayLabel, "Decay");
			addAndMakeVisible(decayLabel);
		}
		
		// sustain
		{
			adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
			addAndMakeVisible(adsrSustain);
			adsrSustain.setValue(0.8f);
			lookAndFeel.initializeLabel(sustainLabel, "Sustain");
			addAndMakeVisible(sustainLabel);
		}
		
		// release
		{
			adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
			addAndMakeVisible(adsrRelease);
			adsrRelease.setValue(0.1f);
			lookAndFeel.initializeLabel(releaseLabel, "Release");
			addAndMakeVisible(releaseLabel);
		}
		
		// on/off toggle
		{
			adsrOnOff.setButtonText("MIDI-triggered ADSR");
			addAndMakeVisible(adsrOnOff);
			adsrOnOff.triggerClick();
		}
	}
	
	// kill all midi button
	{
		midiKill.setButtonText("Kill all MIDI");
		midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
		addAndMakeVisible(midiKill);
	}
	// quick kill ms slider
	{
		quickKillMs.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
		quickKillMs.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		addAndMakeVisible(quickKillMs);
		quickKillMs.setValue(15);
		quickKillMs.setNumDecimalPlacesToDisplay(0);
		lookAndFeel.initializeLabel(quickKillmsLabel, "Quick kill ms");
		addAndMakeVisible(quickKillmsLabel);
	}
	
	// stereo width
	{
		// stereo width dial
		{
			stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(stereoWidth);
			stereoWidth.setValue(100);
			stereoWidth.setNumDecimalPlacesToDisplay(0);
			lookAndFeel.initializeLabel(stereowidthLabel, "Stereo width");
			addAndMakeVisible(stereowidthLabel);
		}
		
		// lowest panned midiPitch
		{
			lowestPan.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
			lowestPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			addAndMakeVisible(lowestPan);
			lowestPan.setValue(0);
			lookAndFeel.initializeLabel(lowestpanLabel, "Lowest panned pitch");
			addAndMakeVisible(lowestpanLabel);
		}
	}
	
	// MIDI velocity sensitivity
	{
		midiVelocitySens.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
		midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		addAndMakeVisible(midiVelocitySens);
		midiVelocitySens.setValue(100);
		lookAndFeel.initializeLabel(midivelocitysensLabel, "MIDI velocity sensitivity");
		addAndMakeVisible(midivelocitysensLabel);
	}
	
	// pitch bend settings
	{
		{
			buildIntervalCombobox(pitchBendUp);
			pitchBendUp.setSelectedId(2);
			addAndMakeVisible(pitchBendUp);
			lookAndFeel.initializeLabel(pitchbendUpLabel, "Pitch bend range up");
			addAndMakeVisible(pitchbendUpLabel);
		}
		{
			buildIntervalCombobox(pitchBendDown);
			addAndMakeVisible(pitchBendDown);
			pitchBendDown.setSelectedId(2);
			lookAndFeel.initializeLabel(pitchbendDownLabel, "Pitch bend range down");
			addAndMakeVisible(pitchbendDownLabel);
		}
	}
	
	
	// voice stealing on/off
	{
		voiceStealing.setButtonText("Voice stealing");
		addAndMakeVisible(voiceStealing);
		voiceStealing.triggerClick();
	}

};

MidiControlPanel::~MidiControlPanel()
{
	setLookAndFeel(nullptr);
};

void MidiControlPanel::paint (juce::Graphics& g)
{
	g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::backgroundPanelColourId));
	
	g.setColour(lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::insetPanelColourId));
	
	juce::Rectangle<int> adsrPanel (5, 110, 290, 125);
	g.fillRect(adsrPanel);
	
	juce::Rectangle<int> stereoWidthPanel (150, 310, 145, 100);
	g.fillRect(stereoWidthPanel);
	
	juce::Rectangle<int> midiVelocitysensPanel (5, 5, 85, 100);
	g.fillRect(midiVelocitysensPanel);
	
	juce::Rectangle<int> pitchbendPanel (5, 240, 290, 65);
	g.fillRect(pitchbendPanel);
	
};

void MidiControlPanel::resized()
{
	// adsr
	{
		attackLabel.setBounds	(5, 130, 75, 35);
		adsrAttack.setBounds	(5, 152, 75, 75);
		
		decayLabel.setBounds	(78, 130, 75, 35);
		adsrDecay.setBounds		(78, 152, 75, 75);
		
		sustainLabel.setBounds	(148, 130, 75, 35);
		adsrSustain.setBounds	(148, 152, 75, 75);
		
		releaseLabel.setBounds	(220, 130, 75, 35);
		adsrRelease.setBounds	(220, 152, 75, 75);
		
		adsrOnOff.setBounds		(70, 110, 175, 35);
	}
	
	// stereo width
	{
		stereowidthLabel.setBounds	(165, 300, 50, 50);
		stereoWidth.setBounds		(153, 333, 75, 75);
		
		lowestpanLabel.setBounds	(240, 310, 50, 50);
		lowestPan.setBounds			(248, 365, 35, 35);
	}
	
	// midi velocity sensitivity
	{
		midivelocitysensLabel.setBounds(5, 10, 85, 35);
		midiVelocitySens.setBounds(25, 50, 45, 45);
	}
	
	// pitch bend
	{
		pitchbendUpLabel.setBounds	(15, 235, 130, 35);
		pitchBendUp.setBounds		(15, 265, 130, 30);
		
		pitchbendDownLabel.setBounds(150, 235, 140, 35);
		pitchBendDown.setBounds		(155, 265, 130, 30);
	}
	
	midiKill.setBounds(145, 5, 100, 35);
	quickKillmsLabel.setBounds(125, 51, 85, 35);
	quickKillMs.setBounds(215, 48, 45, 45);
	
	voiceStealing.setBounds(15, 310, 125, 35);

};


void MidiControlPanel::buildIntervalCombobox(ComboBox& box)
{
	box.addItem("Minor Second", 1);
	box.addItem("Major Second", 2);
	box.addItem("Minor Third", 3);
	box.addItem("Major Third", 4);
	box.addItem("Perfect Fourth", 5);
	box.addItem("Aug Fourth/Dim Fifth", 6);
	box.addItem("Perfect Fifth", 7);
	box.addItem("Minor Sixth", 8);
	box.addItem("Major Sixth", 9);
	box.addItem("Minor Seventh", 10);
	box.addItem("Major Seventh", 11);
	box.addItem("Octave", 12);
};
