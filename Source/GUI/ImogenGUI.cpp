
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

#include "Holders/ImogenGuiHolder.h"


ImogenGUI::ImogenGUI (ImogenGuiHandle* h): holder(h), tooltipWindow(this, msBeforeTooltip)
{
    jassert (holder != nullptr);
           
    this->setBufferedToImage (true);
    
    makePresetMenu (selectPreset);
    selectPreset.onChange = [this] { holder->loadPreset (selectPreset.getText()); };
    
    //addAndMakeVisible(selectPreset);
    addAndMakeVisible (mainDial);
    
    setSize (940, 435);
  
#if JUCE_MAC
    const bool initializeWithDarkMode = juce::Desktop::isOSXDarkModeActive();
    juce::ignoreUnused (initializeWithDarkMode);
#endif
    
    mainDial.showPitchCorrection();
}


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
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);
}


void ImogenGUI::parameterChangeRecieved (int paramID, float newValue)
{
    juce::ignoreUnused (paramID, newValue);
    
    // getComponentForParameter (paramID) ...
}


void ImogenGUI::parameterChangeGestureStarted (int paramID)
{
    mainDial.showParameter (paramID);
}

void ImogenGUI::parameterChangeGestureEnded (int paramID)
{
    juce::ignoreUnused (paramID);
    mainDial.showPitchCorrection();
}


inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}


void ImogenGUI::presetNameChanged (const juce::String& newPresetName)
{
    if (newPresetName.isEmpty())
    {
        
    }
    else
    {
        const auto displayName = TRANS(newPresetName);
        juce::ignoreUnused (displayName);
    }
}


void ImogenGUI::mts_connectionChange (bool isNowConnected)
{
    juce::ignoreUnused (isNowConnected);
}

void ImogenGUI::mts_scaleChange (const juce::String& newScaleName)
{
    const auto displayName = TRANS(newScaleName);
    juce::ignoreUnused (displayName);
}


void ImogenGUI::updateParameterDefaults()
{
    
}


/*===========================================================================================================================
 ============================================================================================================================*/

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
