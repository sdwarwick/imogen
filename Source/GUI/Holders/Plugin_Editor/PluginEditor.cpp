
#include "PluginEditor.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , imgnProcessor (p)
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);

    const auto size = imgnProcessor.getSavedEditorSize();
    const auto width = size.x, height = size.y;
    setResizable (true, true);
    getConstrainer()->setMinimumSize (width / 2, height / 2);
    getConstrainer()->setFixedAspectRatio ((float) width / (float) height);
    setSize (width, height);
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
    imgnProcessor.saveEditorSize (getWidth(), getHeight());

    gui.setBounds (0, 0, getWidth(), getHeight());
}
