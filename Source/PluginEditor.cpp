#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), midiPanel(p)
{
    setSize (400, 300);
	
	currentPitches.ensureStorageAllocated(NUMBER_OF_VOICES);
	currentPitches.fill(0);
	
	addAndMakeVisible(&midiPanel);
	
	Timer::startTimerHz(FRAMERATE);
	
	// set up GUI elements
	{
		
		
		// dry pan
		{
			dryPan.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			dryPan.setRange(0, 127);
			dryPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&dryPan);
			dryPanLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "dryPan", dryPan);
			dryPan.setValue(64);
		}
		
		// master dry/wet
		{
			masterDryWet.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
			masterDryWet.setRange(0, 100);
			masterDryWet.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&masterDryWet);
			masterDryWetLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "masterDryWet", masterDryWet);
			masterDryWet.setValue(100);
		}
		
		// input gain
		{
			inputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
			inputGain.setRange(-60.0f, 0.0f);
			inputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&inputGain);
			inputGainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "inputGain", inputGain);
			inputGain.setValue(0.0f);
		}
		
		// output gain
		{
			outputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
			outputGain.setRange(-60.0f, 0.0f);
			outputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&outputGain);
			outputGainLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "outputGain", outputGain);
			outputGain.setValue(-4.0f);
		}
		
		// set input channel
		{
			inputChannel.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
			inputChannel.setRange(0, 99);
			inputChannel.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//	addAndMakeVisible(&inputChannel);
			inputChannelLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "inputChan", inputChannel);
			inputChannel.setValue(0);
		}
		
		// output limiter
		{
			// threshold
			{
				limiterThresh.setSliderStyle(Slider::SliderStyle::LinearVertical);
				limiterThresh.setRange(-60.0f, 0.0f);
				limiterThresh.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
		//		addAndMakeVisible(&limiterThresh);
				limiterThreshLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterThresh", limiterThresh);
				limiterThresh.setValue(-2.0f);
			}
			// release
			{
				limiterRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
				limiterRelease.setRange(1, 250);
				limiterRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
			//	addAndMakeVisible(&limiterRelease);
				limiterReleaseLink = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "limiterRelease", limiterRelease);
				limiterRelease.setValue(10);
			}
			// toggle on/off
			{
				limiterToggle.setButtonText("Limiter on/off");
		//		addAndMakeVisible(&limiterToggle);
				limiterToggleLink = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "limiterIsOn", limiterToggle);
				limiterToggle.setToggleState(true, true);
			}
		}
	}
}

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
	Timer::stopTimer();
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
	
//	dryPan.setBounds(10, 100, 75, 75);
	
//	inputGain.setBounds(170, 100, 50, 150);
//	outputGain.setBounds(250, 100, 50, 150);
	
//	inputChannel.setBounds(190, 250, 75, 35);
	
	midiPanel.setBounds(0, 0, 350, 500);
	
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
