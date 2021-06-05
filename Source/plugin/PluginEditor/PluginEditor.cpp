
#if ! IMOGEN_HEADLESS

#    include "PluginEditor.h"

namespace Imogen
{

Editor::Editor (Processor& p)
    : EditorBase (p), gui (p.getState())
{
    this->setBufferedToImage (true);
    addAndMakeVisible (gui);
}

Editor::~Editor()
{
}

void Editor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void Editor::resizeTriggered()
{
    gui.setBounds (getLocalBounds());
}

}  // namespace

#endif /* if ! IMOGEN_HEADLESS */
