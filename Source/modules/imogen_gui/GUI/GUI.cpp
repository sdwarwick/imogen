
namespace Imogen
{

GUI::GUI (Imogen::State& stateToUse)
: GUIInitializer (*getTopLevelComponent()),
state (stateToUse)
{
    setInterceptsMouseClicks (false, true);
    
    gui::addAndMakeVisible (this, header, dial, keyboard);
}


GUI::~GUI()
{
    setLookAndFeel (nullptr);
}

void GUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    if (internals.guiDarkMode->get())
    {
    }
    else
    {
    }
}

void GUI::resized()
{
    // header, dial, keyboard
}

bool GUI::keyPressed (const juce::KeyPress&)
{
    return false;
}

}
