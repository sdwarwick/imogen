
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


ImogenGUI::ImogenGUI (ImogenGuiHandle* h): handle(h), tooltipWindow(this, msBeforeTooltip)
{
    jassert (handle != nullptr);
           
    this->setBufferedToImage (true);
    
    makePresetMenu (selectPreset);
    selectPreset.onChange = [this] { handle->loadPreset (selectPreset.getText()); };
    
    //addAndMakeVisible(selectPreset);
    
    setSize (940, 435);
}

#undef bvi_GRAPHICS_FRAMERATE_HZ


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel (nullptr);
}

void ImogenGUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}


void ImogenGUI::resized()
{
    //selectPreset.setBounds(x, y, w, h);
}


inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
//    int id = 1;
    
//    for  (juce::DirectoryEntry entry  :   juce::RangedDirectoryIterator (imgnProcessor.getPresetsFolder(), false))
//    {
//        box.addItem (entry.getFile().getFileName(), id);
//        ++id;
//    }
}


void ImogenGUI::updateParameterDefaults()
{
    
}
