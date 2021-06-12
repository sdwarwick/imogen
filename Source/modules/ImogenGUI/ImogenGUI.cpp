
#include "ImogenGUI.h"

#include "LookAndFeel/ImogenLookAndFeel.cpp"
#include "MainDialComponent/MainDialComponent.cpp"

namespace Imogen
{
GUI::GUI (Imogen::State& stateToUse)
    : GUIInitializer (*getTopLevelComponent()),
      state (stateToUse)
{
    setInterceptsMouseClicks (false, true);

    setLookAndFeel (&lookAndFeel);
}


GUI::~GUI()
{
    setLookAndFeel (nullptr);
}

/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

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
    
}

bool GUI::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool GUI::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void GUI::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void GUI::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}


}  // namespace Imogen
