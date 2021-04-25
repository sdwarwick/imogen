/*======================================================================================================================================================
             _             _   _                _                _                 _               _
            /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
            \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
            /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
           / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
          / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/       _____  ______ __  __  ____ _______ ______
         / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /       |  __ \|  ____|  \/  |/ __ \__   __|  ____|
        / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /        | |__) | |__  | \  / | |  | | | |  | |__
    ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /         |  _  /|  __| | |\/| | |  | | | |  |  __|
   /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /          | | \ \| |____| |  | | |__| | | |  | |____
   \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/           |_|  \_\______|_|  |_|\____/  |_|  |______|
 
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 GuiHandler.h: This file defines the wrapper for Imogen's GUI that transmits messages to and from the actual processor this remote is connected to.
 
 ======================================================================================================================================================*/

#pragma once

#include "GuiHandle.h"


class RemoteGuiHandler :    public ImogenGuiHandle,
                            public ImogenParameterReciever
{
public:
    RemoteGuiHandler() = default;
    
    void sendParameterChange (int paramID, float newValue) override
    {
        juce::ignoreUnused (paramID, newValue);
    }
    
    void sendEditorPitchbend (int wheelValue) override
    {
        juce::ignoreUnused (wheelValue);
    }
    
    void sendMidiLatch (bool shouldBeLatched) override
    {
        juce::ignoreUnused (shouldBeLatched);
    }
    
    void recieveParameterChange (int paramID, float newValue) override
    {
        gui.parameterChangeRecieved (paramID, newValue);
    }
    
    void parameterDefaultsUpdated() override
    {
        gui.updateParameterDefaults();
    }
    
    void loadPreset   (const juce::String& presetName) override { juce::ignoreUnused (presetName); }
    void savePreset   (const juce::String& presetName) override { juce::ignoreUnused (presetName); }
    void deletePreset (const juce::String& presetName) override { juce::ignoreUnused (presetName); }
    
    void presetNameChange (const juce::String& newPresetName) override { juce::ignoreUnused (newPresetName); }
    
    void mts_connectionChange (bool isNowConnected) override { juce::ignoreUnused (isNowConnected); }
    void mts_scaleChange (const juce::String& newScaleName) override { juce::ignoreUnused (newScaleName); }
    
    void abletonLinkChange (bool isNowEnabled) override { juce::ignoreUnused (isNowEnabled); }
    
    
private:
    ImogenGUI gui;
};
