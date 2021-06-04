
#include "MainComponent.h"


MainComponent::MainComponent()
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);

    state.addAllAsInternal();

    setSize (800, 2990);

    //dataSync.connect (<#const juce::String &targetHostName#>);
}


MainComponent::~MainComponent()
{
    dataSync.disconnect();
}


/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    gui.setBounds (getLocalBounds());
}
