#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
	
	adsrAttack.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrAttack.setRange(0.01f, 1.0f);
	adsrAttack.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrAttack.addListener(this);
	addAndMakeVisible(&adsrAttack);
	AudioProcessorValueTreeState::SliderAttachment* attackLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrAttack", adsrAttack);
	adsrAttack.setValue(0.035f);
	
	adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrDecay.setRange(0.01f, 1.0f);
	adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrDecay.addListener(this);
	addAndMakeVisible(&adsrDecay);
	AudioProcessorValueTreeState::SliderAttachment* decayLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrDecay", adsrDecay);
	adsrDecay.setValue(0.06f);
	
	adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrSustain.setRange(0.01f, 1.0f);
	adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrSustain.addListener(this);
	addAndMakeVisible(&adsrSustain);
	AudioProcessorValueTreeState::SliderAttachment* sustainLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrSustain", adsrSustain);
	adsrSustain.setValue(0.8f);
	
	adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrRelease.setRange(0.01f, 1.0f);
	adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrRelease.addListener(this);
	addAndMakeVisible(&adsrRelease);
	AudioProcessorValueTreeState::SliderAttachment* releaseLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrRelease", adsrRelease);
	adsrRelease.setValue(0.1f);
	
	stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	stereoWidth.setRange(0, 100);
	stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	stereoWidth.addListener(this);
	addAndMakeVisible(&stereoWidth);
	AudioProcessorValueTreeState::SliderAttachment* stereoWidthLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "stereoWidth", stereoWidth);
	stereoWidth.setValue(100);
	
	midiVelocitySens.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	midiVelocitySens.setRange(0,100);
	midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	midiVelocitySens.addListener(this);
	addAndMakeVisible(&midiVelocitySens);
	AudioProcessorValueTreeState::SliderAttachment* midiVelocitySensLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens);
	midiVelocitySens.setValue(100);
	
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
	AudioProcessorValueTreeState::ComboBoxAttachment* pitchBendUpLink = new AudioProcessorValueTreeState::ComboBoxAttachment (audioProcessor.tree, "PitchBendUpRange", pitchBendUp);
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
	addAndMakeVisible(pitchBendDown);
	AudioProcessorValueTreeState::ComboBoxAttachment* pitchBendDownLink = new AudioProcessorValueTreeState::ComboBoxAttachment (audioProcessor.tree, "PitchBendDownRange", pitchBendDown);
	pitchBendDown.setSelectedId(2);
	
	inputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
	inputGain.setRange(-60.0f, 0.0f);
	inputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	inputGain.addListener(this);
	addAndMakeVisible(&inputGain);
	AudioProcessorValueTreeState::SliderAttachment* inputGainLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "inputGain", inputGain);
	inputGain.setValue(0.0f);
	
	outputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
	outputGain.setRange(-60.0f, 0.0f);
	outputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	outputGain.addListener(this);
	addAndMakeVisible(&outputGain);
	AudioProcessorValueTreeState::SliderAttachment* outputGainLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "outputGain", outputGain);
	outputGain.setValue(-4.0f);
	
	midiLatch.setButtonText("MIDI latch");
	midiLatch.addListener(this);
	addAndMakeVisible(&midiLatch);
	AudioProcessorValueTreeState::ButtonAttachment* midiLatchLink = new AudioProcessorValueTreeState::ButtonAttachment (audioProcessor.tree, "midiLatch", midiLatch);
	midiLatch.setToggleState(false, true);
}

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {

}

//==============================================================================
void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ImogenAudioProcessorEditor::resized()
{
	adsrAttack.setBounds(10, 10, 75, 75);
	adsrDecay.setBounds(90, 10, 75, 75);
	adsrSustain.setBounds(170, 10, 75, 75);
	adsrRelease.setBounds(250, 10, 75, 75);
	
	stereoWidth.setBounds(10, 100, 75, 75);
	midiVelocitySens.setBounds(90, 100, 75, 75);
	
	pitchBendUp.setBounds(10, 250, 350, 25);
	pitchBendDown.setBounds(375, 250, 350, 25);
	
	inputGain.setBounds(170, 100, 50, 150);
	outputGain.setBounds(250, 100, 50, 150);
	
	midiLatch.setBounds(10, 250, 75, 35);
}



void ImogenAudioProcessorEditor::sliderValueChanged(Slider* slider) {
	
}


void ImogenAudioProcessorEditor::buttonClicked(Button* button) {
	
}


void ImogenAudioProcessorEditor::buttonStateChanged(Button* button) {
	
}
