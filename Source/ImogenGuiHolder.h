
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

#include "GUI/ImogenGUI.h"


class ImogenGuiHolder :     public ImogenGuiHandle
{
public:
    ImogenGuiHolder(): gui(this) { }
    
    virtual ~ImogenGuiHolder() = default;
    
    //
    
    virtual void sendParameterChange (int paramID, float newValue) = 0;
    
    virtual void sendEditorPitchbend (int wheelValue) = 0;
    
    virtual void sendMidiLatch (bool shouldBeLatched) = 0;
    
    virtual void loadPreset   (const juce::String& presetName) = 0;
    virtual void savePreset   (const juce::String& presetName) = 0;
    virtual void deletePreset (const juce::String& presetName) = 0;
    
    //
    
    void recieveParameterChange (int paramID, float newValue) { gui.parameterChangeRecieved (paramID, newValue); }
    
    void parameterDefaultsUpdated() { gui.updateParameterDefaults(); }
    
    void presetNameChange (const juce::String& newPresetName) { juce::ignoreUnused (newPresetName); }
    
    void mts_connectionChange (bool isNowConnected) { juce::ignoreUnused (isNowConnected); }
    void mts_scaleChange (const juce::String& newScaleName) { juce::ignoreUnused (newScaleName); }
    
    void abletonLinkChange (bool isNowEnabled) { juce::ignoreUnused (isNowEnabled); }
    
    //
    
private:
    ImogenGUI gui;
};
