#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), midiPanel(p), ioPanel(p), staffDisplay(p), viewHelp(false)
{
    setSize (940, 435);
	
	initializeLookAndFeel(lookAndFeel);
	
	addAndMakeVisible(midiPanel);
	midiPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(ioPanel);
	ioPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(staffDisplay);
	staffDisplay.setLookAndFeel(&lookAndFeel);
	addChildComponent(helpScreen);
	helpScreen.setLookAndFeel(&lookAndFeel);
	
	Timer::startTimerHz(FRAMERATE);
	
};

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
	setLookAndFeel(nullptr);
	Timer::stopTimer();
};

//==============================================================================
void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll (juce::Colours::dimgrey);
};

void ImogenAudioProcessorEditor::resized()
{
	midiPanel.setBounds(10, 10, 300, 415);
	ioPanel.setBounds(320, 10, 300, 415);
	staffDisplay.setBounds(630, 10, 300, 415);
	//helpScreen.setBounds(x, y, w, h);
};



void ImogenAudioProcessorEditor::timerCallback()
{
	staffDisplay.repaint();
	if(viewHelp) {
		if(! helpScreen.isVisible() ) { helpScreen.setVisible(true); };
		helpScreen.repaint();
	} else {
		helpScreen.setVisible(false);
	}
};


void ImogenAudioProcessorEditor::initializeLookAndFeel(ImogenLookAndFeel& lookAndFeel)
{
	// rotary sliders
	lookAndFeel.setColour(Slider::ColourIds::rotarySliderFillColourId, juce::Colours::royalblue);
	lookAndFeel.setColour(Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::black);
	lookAndFeel.setColour(Slider::ColourIds::thumbColourId, juce::Colours::black);
	
	// labels
	lookAndFeel.setColour(Label::ColourIds::textColourId, juce::Colours::black);
	
	// buttons
	lookAndFeel.setColour(TextButton::buttonColourId, juce::Colours::black);
};
