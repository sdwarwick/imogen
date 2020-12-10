#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), midiPanel(p), ioPanel(p)
{
    setSize (940, 435);
	
	currentPitches.ensureStorageAllocated(NUMBER_OF_VOICES);
	currentPitches.clearQuick();
	currentPitches.add(-1);
	
	lookAndFeel.setColour(Slider::ColourIds::rotarySliderFillColourId, juce::Colours::mediumslateblue);
	lookAndFeel.setColour(Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::black);
	lookAndFeel.setColour(Slider::ColourIds::thumbColourId, juce::Colours::darkgrey);
	
	addAndMakeVisible(midiPanel);
	midiPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(ioPanel);
	ioPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(staffDisplay);
	staffDisplay.setLookAndFeel(&lookAndFeel);
	
	Timer::startTimerHz(FRAMERATE);
	
}

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
	Timer::stopTimer();
}

//==============================================================================
void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void ImogenAudioProcessorEditor::resized()
{
	midiPanel.setBounds(10, 10, 300, 415);
	ioPanel.setBounds(320, 10, 300, 415);
	staffDisplay.setBounds(630, 10, 300, 415);
}



void ImogenAudioProcessorEditor::timerCallback() {
//	this->repaint();
	
	Array<int> returnedpitches = audioProcessor.returnActivePitches();
	
	if(returnedpitches.getUnchecked(0) == -1) {
		// no pitches are currently active
		currentPitches.clearQuick();
		currentPitches.add(-1);
	} else {
		currentPitches.clearQuick();
		for(int i = 0; i < returnedpitches.size(); ++i) {
			currentPitches.add(returnedpitches.getUnchecked(i));
		}
	}
	
}
