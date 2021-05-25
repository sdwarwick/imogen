
/*===================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 ImogenGUI.cpp: This file defines implementation details for Imogen's top-level GUI component.

===================================================================================================================================================*/


#include "ImogenGUI.h"

#include "LookAndFeel/ImogenLookAndFeel.cpp"
#include "MainDialComponent/MainDialComponent.cpp"


ImogenGUI::ImogenGUI (ImogenGUIUpdateSender* s)
    : GUIInitializer (getTopLevelComponent())
    , tooltipWindow (this, msBeforeTooltip)
{
    setInterceptsMouseClicks (false, true);
    
    parameters.addAllParametersAsInternal();
    meters.addAllParametersAsInternal();

    addAndMakeVisible (mainDial);
    
#if JUCE_MAC
    parameters.guiDarkMode->set (juce::Desktop::isOSXDarkModeActive());
#endif
    
    parameters.guiDarkMode->onParameterChange = [this](){ repaint(); };

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
    const auto filename = bav::addFileExtensionIfMissing (presetName, Imogen::getPresetFileExtension());

    juce::FileOutputStream stream (filename);
    
    state.serialize().writeToStream (stream);

    rescanPresetsFolder();
}


void ImogenGUI::loadPreset (const juce::String& presetName)
{
    const auto presetToLoad = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName,
                                                                                                    Imogen::getPresetFileExtension()));

    if (!presetToLoad.existsAsFile())
    {
        // display error message...
        return;
    }

    juce::MemoryBlock data;

    juce::FileInputStream stream (presetToLoad);

    stream.readIntoMemoryBlock (data);

    state.deserialize (data.getData(), data.getSize());

    repaint();
}


void ImogenGUI::deletePreset (const juce::String& presetName)
{
    auto presetToDelete = Imogen::presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName,
                                                                                                Imogen::getPresetFileExtension()));

    if (presetToDelete.existsAsFile())
    {
        if (!presetToDelete.moveToTrash()) presetToDelete.deleteFile();

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

    if (parameters.guiDarkMode->get()) { }
    else
    {
    }
}

void ImogenGUI::resized()
{
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);

    auto r = getLocalBounds(); // this rectangle represents the entire area of our GUI

    juce::ignoreUnused (r);
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
