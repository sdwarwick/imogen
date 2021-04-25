/*======================================================================================================================================================
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
 
 ImogenGUI.h: This file defines the interface for Imogen's top-level GUI component. This class does not reference ImogenAudioProcessor, so that it can also be used to  create a GUI-only remote control application for Imogen.
 
======================================================================================================================================================*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "LookAndFeel/ImogenLookAndFeel.h"
#include "MainDialComponent/MainDialComponent.h"

#include "BinaryData.h"


/* The interface used to communicate from the GUI to the processor */
struct ImogenGuiHandle
{
    virtual ~ImogenGuiHandle() = default;
    
    virtual void sendParameterChange (int paramID, float newValue) = 0;
    virtual void startParameterChangeGesture (int paramID) = 0;
    virtual void endParameterChangeGesture (int paramID) = 0;
    
    virtual void sendEditorPitchbend (int wheelValue) = 0;
    
    virtual void sendMidiLatch (bool shouldBeLatched) = 0;
    
    virtual void loadPreset   (const juce::String& presetName) = 0;
    virtual void savePreset   (const juce::String& presetName) = 0;
    virtual void deletePreset (const juce::String& presetName) = 0;
    
    virtual void enableAbletonLink (bool shouldBeEnabled) = 0;
};


/*
*/


class ImogenGUI  :     public juce::Component
{
public:
    
    ImogenGUI (ImogenGuiHandle* h);
    
    virtual ~ImogenGUI() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void parameterChangeRecieved (int paramID, float newValue);
    void parameterChangeGestureStarted (int paramID);
    void parameterChangeGestureEnded (int paramID);
    
    void updateParameterDefaults();
    
    void presetNameChanged (const juce::String& newPresetName);
    
    void mts_connectionChange (bool isNowConnected);
    void mts_scaleChange (const juce::String& newScaleName);
    
    void abletonLinkChange (bool isNowEnabled) { juce::ignoreUnused (isNowEnabled); }
    
    //
    
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;
    void focusLost (FocusChangeType cause) override;
    
    //
    
private:
    juce::Component* getComponentForParameter (int paramID);
    
    inline void makePresetMenu (juce::ComboBox& box);
    
    //
    
    ImogenDialComponent mainDial;
    
    juce::ComboBox selectPreset;
    
    bav::ImogenLookAndFeel lookAndFeel;
    
    ImogenGuiHandle* const holder;
    
    juce::TooltipWindow tooltipWindow;
    static constexpr int msBeforeTooltip = 700;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenGUI)
};
