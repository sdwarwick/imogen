
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

    addAndMakeVisible (mainDial);

    state.setUndoManager (undoManager);

    juce::ignoreUnused (state, parameters, internals, meters);
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
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);
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


inline void GUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}

}  // namespace Imogen
