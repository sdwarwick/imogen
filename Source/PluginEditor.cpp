#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), currentSkin(ImogenLookAndFeel::Skin::design1), prevSkin(ImogenLookAndFeel::Skin::design1), midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel), staffDisplay(p), viewHelp(false)
{
    setSize (940, 475);
	
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
	midiPanel.setBounds(10, 50, 300, 415);
	ioPanel.setBounds(320, 50, 300, 415);
	staffDisplay.setBounds(630, 50, 300, 415);
	//helpScreen.setBounds(x, y, w, h);
	
	selectSkin.setBounds(10, 10, 150, 30);
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
	const int selectedid = selectSkin.getSelectedId();
	switch(selectedid)
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
