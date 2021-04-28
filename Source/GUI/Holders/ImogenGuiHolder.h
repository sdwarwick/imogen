
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
 
 ImogenGuiHolder.h: This file defines a class that holds an ImogenGui object and acts as its interface with the outside world.
    Note that ImogenGuiHolder is an abstract class, because it inherits from ImogenEventSender and doesn't implement any of its pure virtual methods.
 
 ======================================================================================================================================================*/


#pragma once

#include "GUI/ImogenGUI.h"


using namespace Imogen;


class ImogenGuiHolder  :    public ImogenEventSender,
                            public ImogenEventReciever
{
public:
    ImogenGuiHolder(): p_gui(this) { }
    
    virtual ~ImogenGuiHolder() override = default;
    
    /*=========================================================================================*/
    /* ImogenEventReciever functions -- these calls are simply forwarded to the ImogenGUI object's ImogenEventReciever interface. */
    
    void recieveParameterChange (ParameterID paramID, float newValue) override final { p_gui.recieveParameterChange (paramID, newValue); }
    
    void recieveParameterChangeGestureStart (ParameterID paramID) override final { p_gui.recieveParameterChangeGestureStart (paramID); }
    void recieveParameterChangeGestureEnd   (ParameterID paramID) override final { p_gui.recieveParameterChangeGestureEnd   (paramID); }
    
    void recieveLoadPreset   (const juce::String& newPresetName) override final { p_gui.recieveLoadPreset (newPresetName); }
    void recieveSavePreset   (const juce::String& newPresetName) override final { p_gui.recieveSavePreset (newPresetName); }
    void recieveDeletePreset (const juce::String& newPresetName) override final { p_gui.recieveDeletePreset (newPresetName); }
    
    void recieveMTSconnectionChange (bool isNowConnected) override final { p_gui.recieveMTSconnectionChange (isNowConnected); }
    void recieveMTSscaleChange (const juce::String& newScaleName) override final { p_gui.recieveMTSscaleChange (newScaleName); }
    
    void recieveAbletonLinkChange (bool isNowEnabled) override final { p_gui.recieveAbletonLinkChange (isNowEnabled); }
    
    void recieveMidiLatchEvent (bool isNowLatched) override final { p_gui.recieveMidiLatchEvent (isNowLatched); }
    
    void recieveKillAllMidiEvent() override final { p_gui.recieveKillAllMidiEvent(); }
    
    void recieveEditorPitchbendEvent (int wheelValue) override final { p_gui.recieveEditorPitchbendEvent (wheelValue); }
    
    void recieveErrorCode (ErrorCode code) override final { p_gui.recieveErrorCode (code); }
    
    /*=========================================================================================*/
    
    ImogenGUI* gui() noexcept { return &p_gui; }
    
    /*=========================================================================================*/
    
    ImogenGUIState returnState() const { return ImogenGUIState(&p_gui); }
    
    void saveState (ImogenGUIState& state) { state.saveState (&p_gui); }
    
    void restoreState (const ImogenGUIState& state) { state.resoreState (&p_gui); }
    
    /*=========================================================================================*/
    
private:
    ImogenGUI p_gui;
};
