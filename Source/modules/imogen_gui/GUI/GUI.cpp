
namespace Imogen
{
BV_IMPLEMENT_PLUGIN_GUI_CONSTRUCTOR (GUI, State)
{
    setInterceptsMouseClicks (false, true);

    gui::addAndMakeVisible (this, header, dial, dryWet, keyboard);
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
    // header, dial, dryWet, keyboard
}

bool GUI::keyPressed (const juce::KeyPress&)
{
    return false;
}

}  // namespace Imogen
