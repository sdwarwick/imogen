
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
 
 ImogenRecieverAPI.h: This file defines the API used for communicating change events from an ImogenAudioProcessor object to an ImogenGUI object. The abstraction of this API allows the same GUI code to be used both for the plugin build and the remote app build.
 
 ======================================================================================================================================================*/


#pragma once

#include "ImogenGUI.h"


class ImogenGuiHolder  :    public ImogenGuiHandle
{
public:
    ImogenGuiHolder(): p_gui(this) { }
    
    virtual ~ImogenGuiHolder() = default;
    
    //
    
    void recieveParameterChange (int paramID, float newValue) { p_gui.parameterChangeRecieved (paramID, newValue); }
    
    void parameterDefaultsUpdated() { p_gui.updateParameterDefaults(); }
    
    void presetNameChange (const juce::String& newPresetName) { p_gui.presetNameChanged (newPresetName); }
    
    void mts_connectionChange (bool isNowConnected) { p_gui.mts_connectionChange (isNowConnected); }
    void mts_scaleChange (const juce::String& newScaleName) { p_gui.mts_scaleChange (newScaleName); }
    
    void abletonLinkChange (bool isNowEnabled) { p_gui.abletonLinkChange (isNowEnabled); }
    
    //
    
    ImogenGUI* gui() noexcept { return &p_gui; }
    
    //
    
private:
    ImogenGUI p_gui;
};
