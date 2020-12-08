#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), midiPanel(p), ioPanel(p)
{
    setSize (1130, 700);
	
	currentPitches.ensureStorageAllocated(NUMBER_OF_VOICES);
	currentPitches.clearQuick();
	currentPitches.add(-1);
	
	addAndMakeVisible(midiPanel);
	//addAndMakeVisible(ioPanel);
	
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
	//ioPanel.setBounds(<#int x#>, <#int y#>, <#int width#>, <#int height#>);
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
