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
	attackLink = AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrAttack", adsrAttack);
	adsrAttack.setValue(0.035f);
	
	adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrDecay.setRange(0.01f, 1.0f);
	adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrDecay.addListener(this);
	addAndMakeVisible(&adsrDecay);
	decayLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrDecay", adsrDecay);
	adsrDecay.setValue(0.06f);
	
	adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrSustain.setRange(0.01f, 1.0f);
	adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrSustain.addListener(this);
	addAndMakeVisible(&adsrSustain);
	sustainLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrSustain", adsrSustain);
	adsrSustain.setValue(0.8f);
	
	adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	adsrRelease.setRange(0.01f, 1.0f);
	adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	adsrRelease.addListener(this);
	addAndMakeVisible(&adsrRelease);
	releaseLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "adsrRelease", adsrRelease);
	adsrRelease.setValue(0.1f);
	
	stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	stereoWidth.setRange(0, 100);
	stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	stereoWidth.addListener(this);
	addAndMakeVisible(&stereoWidth);
	stereoWidthLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "stereoWidth", stereoWidth);
	stereoWidth.setValue(100);
	
	midiVelocitySens.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	midiVelocitySens.setRange(0,100);
	midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
	midiVelocitySens.addListener(this);
	addAndMakeVisible(&midiVelocitySens);
	midiVelocitySensLink = new AudioProcessorValueTreeState::SliderAttachment (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens);
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
}



void ImogenAudioProcessorEditor::sliderValueChanged(Slider* slider) {
	
}
