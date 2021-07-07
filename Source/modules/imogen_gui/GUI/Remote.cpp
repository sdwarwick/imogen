
namespace Imogen
{

Remote::Remote()
{
    this->setBufferedToImage (true);
    
    addAndMakeVisible (gui);
    
    state.addAllAsInternal();
    
    setSize (800, 2990);
}

Remote::~Remote()
{
    setLookAndFeel (nullptr);
}

void Remote::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Remote::resized()
{
    gui.setBounds (getLocalBounds());
}


}
