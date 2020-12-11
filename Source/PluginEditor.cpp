#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), currentSkin(ImogenLookAndFeel::Skin::design1), prevSkin(ImogenLookAndFeel::Skin::design1), midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel), staffDisplay(p, lookAndFeel), viewHelp(false)
{
    setSize (940, 435);
	
	lookAndFeel.changeSkin(currentSkin);
	
	addAndMakeVisible(midiPanel);
	midiPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(ioPanel);
	ioPanel.setLookAndFeel(&lookAndFeel);
	addAndMakeVisible(staffDisplay);
	staffDisplay.setLookAndFeel(&lookAndFeel);
	addChildComponent(helpScreen);
	helpScreen.setLookAndFeel(&lookAndFeel);
	
	Timer::startTimerHz(FRAMERATE);
	
	selectSkin.addItem("design1", 1);
	selectSkin.addItem("design2", 2);
	selectSkin.addItem("design3", 3);
	selectSkin.setSelectedId(1);
	addAndMakeVisible(selectSkin);
	selectSkin.onChange = [this] { skinSelectorChanged(); };
	
	helpButton.setButtonText("Help");
	helpButton.onClick = [this] { helpButtonClicked(); };
	addAndMakeVisible(helpButton);
};

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
	setLookAndFeel(nullptr);
	Timer::stopTimer();
};

//==============================================================================
void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::blankCanvasColourId));
};

void ImogenAudioProcessorEditor::resized()
{
	midiPanel.setBounds(10, 10, 300, 415);
	ioPanel.setBounds(320, 10, 300, 415);
	staffDisplay.setBounds(630, 10, 300, 350);
	
	selectSkin.setBounds(780, 385, 150, 30);
	helpButton.setBounds(690, 385, 75, 30);
	
	helpScreen.setBounds(158, 80, 625, 315);
	
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


void ImogenAudioProcessorEditor::skinSelectorChanged()
{
	switch(selectSkin.getSelectedId())
	{
		case(1):
			currentSkin = ImogenLookAndFeel::Skin::design1;
			break;
		case(2):
			currentSkin = ImogenLookAndFeel::Skin::design2;
			break;
		case(3):
			currentSkin = ImogenLookAndFeel::Skin::design3;
			break;
	}
	
	if(currentSkin != prevSkin)
	{
		lookAndFeel.changeSkin(currentSkin);
		prevSkin = currentSkin;
		this->repaint();
	}
};


void ImogenAudioProcessorEditor::helpButtonClicked()
{
	if(viewHelp) { viewHelp = false; }
	else { viewHelp = true; }
	
	this->repaint();
};
