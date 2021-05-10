
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


ImogenGUI::ImogenGUI (ImogenGUIUpdateSender* s)
      : state (ValueTreeIDs::Imogen),
        treeSync (state, s),
        tooltipWindow (this, msBeforeTooltip)
{
#if BV_USE_NE10
    ne10_init();
#endif
    
#if IMOGEN_REMOTE_APP
    bav::initializeTranslations (findAppropriateTranslationFile());
#endif
    
    setInterceptsMouseClicks (false, true);
    
    Imogen::buildImogenMainValueTree (state, processor.getParameterTree());
    
    Imogen::initializeParameterPointers (parameterPointers, meterParameterPointers, processor.getParameterTree());
    
    bav::createTwoWayParameterValueTreeAttachments (parameterTreeAttachments,
                                                    state.getChildWithName (ValueTreeIDs::Parameters), numParams,
                                                    [this](int param) { return getParameterPntr (static_cast<ParameterID>(param)); });
    
    bav::createReadOnlyParameterValueTreeAttachments (meterTreeAttachments,
                                                      state.getChildWithName (ValueTreeIDs::Meters), numMeters,
                                                      [this](int param) { return getMeterParamPntr (static_cast<MeterID>(param)); });
    
    makePresetMenu (selectPreset);
    // selectPreset.onChange = [this] { holder->sendLoadPreset (selectPreset.getText()); };
    
    //addAndMakeVisible(selectPreset);
    addAndMakeVisible (mainDial);
    
    setSize (940, 435);
  
#if JUCE_MAC
    darkMode.store (juce::Desktop::isOSXDarkModeActive());
#else
    darkMode.store (true);
#endif
    
    mainDial.showPitchCorrection();
    
    rescanPresetsFolder();
}


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel (nullptr);
}


inline bav::Parameter* ImogenGUI::getParameterPntr (const ParameterID paramID) const
{
    for (auto* pntr : parameterPointers)
        if (static_cast<ParameterID>(pntr->key()) == paramID)
            return pntr;
    
    return nullptr;
}


inline bav::Parameter* ImogenGUI::getMeterParamPntr (const MeterID meter) const
{
    for (auto* pntr : meterParameterPointers)
        if (static_cast<MeterID>(pntr->key()) == meter)
            return pntr;
    
    return nullptr;
}


/*=========================================================================================================
 =========================================================================================================*/


void ImogenGUI::applyValueTreeStateChange (const void* encodedChangeData, size_t encodedChangeDataSize)
{
    juce::ValueTreeSynchroniser::applyChange (state, encodedChangeData, encodedChangeDataSize, nullptr);
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
    const auto filename = bav::addFileExtensionIfMissing (presetName, getPresetFileExtension());
    
    juce::FileOutputStream stream (filename);
    
    auto tree = state.createCopy();
    
    tree.setProperty ("PresetName",
                      bav::removeFileExtensionIfThere (presetName, getPresetFileExtension()),
                      nullptr);
    
    tree.writeToStream (stream);

    rescanPresetsFolder();
}


void ImogenGUI::loadPreset (const juce::String& presetName)
{
    if (presetName.isEmpty())
    {
        // display error message...
        return;
    }

    rescanPresetsFolder();

    const auto filename = bav::addFileExtensionIfMissing (presetName, getPresetFileExtension());
    const auto presetToLoad = presetsFolder().getChildFile (filename);

    if (! presetToLoad.existsAsFile())
    {
        // display error message...
        return;
    }

    juce::MemoryBlock data;
    
    juce::FileInputStream stream (presetToLoad);
    
    stream.readIntoMemoryBlock (data);
    
    auto newTree = juce::ValueTree::readFromData (data.getData(), data.getSize());
    
    if (! newTree.isValid() || ! newTree.hasType (state.getType()))
        return;
    
    state.copyPropertiesAndChildrenFrom (newTree, nullptr);
    
    treeSync.sendFullSyncCallback();
    
    repaint();
}


void ImogenGUI::deletePreset (const juce::String& presetName)
{
    rescanPresetsFolder();
    
    auto presetToDelete = presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName, getPresetFileExtension()));

    if (presetToDelete.existsAsFile())
    {
        if (! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();

        rescanPresetsFolder();
    }
}


void ImogenGUI::renamePreset (const juce::String& previousName, const juce::String& newName)
{
    if (previousName.isEmpty() || newName.isEmpty())
    {
        // display error message...
        return;
    }
    
    rescanPresetsFolder();
    
    const auto filename = bav::addFileExtensionIfMissing (previousName, getPresetFileExtension());
    const auto presetToLoad = presetsFolder().getChildFile (filename);
    
    if (! presetToLoad.existsAsFile())
    {
        // display error message...
        return;
    }
    
    // load the old preset into memory
    
    juce::MemoryBlock data;
    
    juce::FileInputStream inStream (presetToLoad);
    
    inStream.readIntoMemoryBlock (data);
    
    auto newTree = juce::ValueTree::readFromData (data.getData(), data.getSize());
    
    if (! newTree.isValid() || ! newTree.hasType (state.getType()))
        return;
    
    // delete the old preset file
    if (! presetToLoad.moveToTrash())
        presetToLoad.deleteFile();
    
    // edit the saved preset name
    newTree.setProperty ("PresetName",
                         bav::removeFileExtensionIfThere (newName, getPresetFileExtension()),
                         nullptr);
    
    // write to a new file
    const auto newFilename = bav::addFileExtensionIfMissing (newName, getPresetFileExtension());
    
    juce::FileOutputStream outStream (newFilename);
    
    newTree.writeToStream (outStream);
    
    rescanPresetsFolder();
}


/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void ImogenGUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    if (darkMode.load())
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
    
    auto r = getLocalBounds();  // this rectangle represents the entire area of our GUI
    
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


/*=========================================================================================================
 =========================================================================================================*/

void ImogenGUI::setDarkMode (bool shouldUseDarkMode)
{
    darkMode.store (shouldUseDarkMode);
    // inform all child components of the change...
    this->repaint();
}

inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}
