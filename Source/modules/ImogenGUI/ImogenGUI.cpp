
#include "ImogenGUI.h"

#include "LookAndFeel/ImogenLookAndFeel.cpp"
#include "MainDialComponent/MainDialComponent.cpp"

namespace Imogen
{
GUI::GUI (Imogen::State& stateToUse)
    : GUIInitializer (*getTopLevelComponent()),
      state (stateToUse),
      parameters (state.parameters),
      internals (state.internals),
      meters (state.meters),
      darkModeSentinel (internals.guiDarkMode, *this)
{
    setInterceptsMouseClicks (false, true);

    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (mainDial);

    rescanPresetsFolder();

    state.setUndoManager (undoManager);

    juce::ignoreUnused (state, parameters, internals, meters);
}


GUI::~GUI()
{
    setLookAndFeel (nullptr);
}

/*=========================================================================================================
 =========================================================================================================*/

void GUI::rescanPresetsFolder()
{
    //    availablePresets.clearQuick();
    //    const auto xtn = getPresetFileExtension();
    //
    //    for (auto entry  :   juce::RangedDirectoryIterator (getPresetsFolder(), false))
    //    {
    //        const auto filename = entry.getFile().getFileName();
    //
    //        if (filename.endsWith (xtn))
    //            availablePresets.add (filename.dropLastCharacters (xtn.length()));
    //    }

    repaint();
}


void GUI::savePreset (const String& presetName)
{
    serializing::toBinary (state, presetNameToFilePath (presetName));
    rescanPresetsFolder();
}

void GUI::loadPreset (const String& presetName)
{
    serializing::fromBinary (presetNameToFilePath (presetName), state);
    repaint();
}

void GUI::deletePreset (const String& presetName)
{
    deleteFile (presetNameToFilePath (presetName));
    rescanPresetsFolder();
}

void GUI::renamePreset (const String& previousName, const juce::String& newName)
{
    renameFile (presetNameToFilePath (previousName),
                addFileExtensionIfMissing (newName, getPresetFileExtension()));

    rescanPresetsFolder();
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
