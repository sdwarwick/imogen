
#include "PluginEditor.h"

#if ! IMOGEN_HEADLESS


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : EditorBase (p), gui (p.state)
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
}


void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}


void ImogenAudioProcessorEditor::resizeTriggered()
{
    gui.setBounds (getLocalBounds());
}

#endif /* if ! IMOGEN_HEADLESS */
