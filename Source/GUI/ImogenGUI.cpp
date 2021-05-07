
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


#include "bv_SharedCode/BinaryDataHelpers.h"

#include "ImogenGUI.h"


ImogenGUI::ImogenGUI (ImogenGUIUpdateSender* s)
      : parameterTree (Imogen::createParameterTree()),
        state (ValueTreeIDs::Imogen),
        treeSync (state, s),
        tooltipWindow (this, msBeforeTooltip)
{
    setBufferedToImage (true);
    
    setInterceptsMouseClicks (false, true);
    
    Imogen::buildImogenMainValueTree (state, *parameterTree);
    
    parameterPointers.reserve (numParams);
    parseParameterTreeForParameterPointers (*parameterTree);
    
    Imogen::createValueTreeParameterAttachments (state, parameterTreeAttachments,
                                                 [this](ParameterID param) { return getParameterPntr (param); },
                                                 [this](MeterID meter) { return getMeterParamPntr (meter); });
    
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


inline void ImogenGUI::parseParameterTreeForParameterPointers (const juce::AudioProcessorParameterGroup& group)
{
    for (auto* node : group)
    {
        if (auto* rawParam = node->getParameter())
        {
            if (auto* meter = dynamic_cast<bav::MeterParameter*> (rawParam))
                meterParameterPointers.push_back (meter);
            else if (auto* param = dynamic_cast<bav::Parameter*> (rawParam))
                parameterPointers.push_back (param);
            else
                jassertfalse;
        }
        else if (auto* thisGroup = node->getGroup())
        {
            parseParameterTreeForParameterPointers (*thisGroup);
        }
    }
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
