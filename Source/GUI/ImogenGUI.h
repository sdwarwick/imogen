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


using namespace Imogen;


class ImogenGUI  :     public juce::Component,
                       public ImogenEventReciever
{
public:
    
    ImogenGUI (ImogenEventSender* h);
    
    virtual ~ImogenGUI() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void recieveParameterChange             (ParameterID paramID, float newValue) override final;
    void recieveParameterChangeGestureStart (ParameterID paramID) override final;
    void recieveParameterChangeGestureEnd   (ParameterID paramID) override final;
    
    void recieveLoadPreset   (const juce::String& newPresetName) override final;
    void recieveSavePreset   (const juce::String&) override final { }
    void recieveDeletePreset (const juce::String&) override final { }
    
    void recieveAbletonLinkChange (bool isNowEnabled) override final { juce::ignoreUnused (isNowEnabled); }
    
    void recieveMTSconnectionChange (bool isNowConnected) override final;
    void recieveMTSscaleChange (const juce::String& newScaleName) override final;
    
    void recieveMidiLatchEvent (bool isNowLatched) override final;
    void recieveKillAllMidiEvent() override final;
    
    void recieveEditorPitchbendEvent (int) override final { }
    
    //
    
    void recieveErrorCode (ErrorCode code) override final;
    
    //
    
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;
    void focusLost (FocusChangeType cause) override;
    
    //
    
    ImogenGUIParameter* getParameter (ParameterID paramID) const;
    
    //
    
    void setDarkMode (bool shouldUseDarkMode);
    bool isUsingDarkMode() const noexcept { return darkMode.load(); }
    
    //
    
private:
    inline void makePresetMenu (juce::ComboBox& box);
    
    //
    
    ImogenDialComponent mainDial;
    
    juce::ComboBox selectPreset;
    
    bav::ImogenLookAndFeel lookAndFeel;
    
    ImogenEventSender* const holder;
    
    juce::TooltipWindow tooltipWindow;
    static constexpr int msBeforeTooltip = 700;
    
    void createParameters();
    
    std::vector<std::unique_ptr<ImogenGUIParameter>> parameters;
    
    std::atomic<bool> darkMode;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenGUI)
};



/* Simple data container class used for saving & loading the current UI state */
class ImogenGUIState
{
public:
    // creates a default ImogenGUIState object
    ImogenGUIState()
    {
        usingDarkMode = true;
        // guiSize.setX ();
        // guiSize.setY ();
    }
    
    // creates an ImogenGUIState object from a passed ImogenGUI object
    ImogenGUIState (const ImogenGUI* gui)
    {
        usingDarkMode = gui->isUsingDarkMode();
        guiSize.setX (gui->getWidth());
        guiSize.setY (gui->getHeight());
    }
    
    virtual ~ImogenGUIState() = default;
    
    // updates the state object to reflect the passed GUI object
    void saveState (const ImogenGUI* gui)
    {
        usingDarkMode = gui->isUsingDarkMode();
        guiSize.setX (gui->getWidth());
        guiSize.setY (gui->getHeight());
    }
    
    void resoreState (ImogenGUI* gui) const
    {
        restoreState (*this, gui);
    }
    
    // restores a GUI's state to the state described by the passed ImogenGUIState object
    static void restoreState (const ImogenGUIState& state, ImogenGUI* gui)
    {
        gui->setDarkMode (state.usingDarkMode);
        gui->setSize (state.guiSize.x, state.guiSize.y);
        
        gui->repaint();
    }
    
    /* these are public so they can be easily edited & retrieved from outside the class */
    bool usingDarkMode;
    juce::Point<int> guiSize;
};
