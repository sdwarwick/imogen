
#include "ImogenGUI.h"

#include "LookAndFeel/ImogenLookAndFeel.cpp"
#include "MainDialComponent/MainDialComponent.cpp"


ImogenGUI::ImogenGUI()
    : GUIInitializer (getTopLevelComponent()), tooltipWindow (this, msBeforeTooltip)
{
    setInterceptsMouseClicks (false, true);
    
    state.addAllAsInternal();

    addAndMakeVisible (mainDial);

#if JUCE_MAC
    internals.guiDarkMode->set (juce::Desktop::isOSXDarkModeActive());
#endif

    internals.guiDarkMode->onParameterChange = [this]()
    { repaint(); };

    mainDial.showPitchCorrection();

    rescanPresetsFolder();
}


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel (nullptr);
}

/*=========================================================================================================
 =========================================================================================================*/

void ImogenGUI::rescanPresetsFolder()
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

    // show spinning/progress graphic...?

    repaint();
}


void ImogenGUI::savePreset (const juce::String& presetName)
{
    auto savingTo = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName, Imogen::getPresetFileExtension()));
    
    bav::toBinary (state, savingTo);
    
    rescanPresetsFolder();
}


void ImogenGUI::loadPreset (const juce::String& presetName)
{
    auto presetToLoad = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName,
                                                                                              Imogen::getPresetFileExtension()));

    if (! presetToLoad.existsAsFile())
    {
        // display error message...
        return;
    }

    bav::fromBinary (presetToLoad, state);
    
    repaint();
}


void ImogenGUI::deletePreset (const juce::String& presetName)
{
    auto presetToDelete = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName,
                                                                                                Imogen::getPresetFileExtension()));

    if (presetToDelete.existsAsFile())
    {
        if (! presetToDelete.moveToTrash()) presetToDelete.deleteFile();

        rescanPresetsFolder();
    }
}


void ImogenGUI::renamePreset (const juce::String& previousName, const juce::String& newName)
{
    const auto presetToLoad = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (previousName,
                                                                                                    Imogen::getPresetFileExtension()));

    if (! presetToLoad.existsAsFile())
    {
        // display error message...
        return;
    }

    bav::renameFile (presetToLoad,
                     bav::addFileExtensionIfMissing (newName, Imogen::getPresetFileExtension()));

    rescanPresetsFolder();
}


/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void ImogenGUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (internals.guiDarkMode->get())
    {
    }
    else
    {
    }
}

void ImogenGUI::resized()
{
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);
}

bool ImogenGUI::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool ImogenGUI::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void ImogenGUI::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void ImogenGUI::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}


inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}
