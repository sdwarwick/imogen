
#include "MainComponent.h"

namespace Imogen
{

Remote::Remote()
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);

    state.addAllAsInternal();

    setSize (800, 2990);

    dataSync.connect ("host");
}

Remote::~Remote()
{
    dataSync.disconnect();
}

void Remote::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Remote::resized()
{
    gui.setBounds (getLocalBounds());
}

}  // namespace
