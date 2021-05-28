
#include "PluginEditor.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : EditorBase (p), gui (p.state)
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
}

/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}


void ImogenAudioProcessorEditor::resized()
{
    gui.setBounds (getLocalBounds());
}
