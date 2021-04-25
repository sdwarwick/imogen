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


class RemoteGuiHandler :    public ImogenGuiHandle
{
public:
    RemoteGuiHandler() = default;
    
    void sendParameterChange (int paramID, float newValue) override
    {
        imgnProcessor.parameterChangeRecieved (paramID, newValue);
    }
    
    void sendEditorPitchbend (int wheelValue) override
    {
        imgnProcessor.editorPitchbend (wheelValue);
    }
    
    void sendMidiLatch (bool shouldBeLatched) override
    {
        juce::ignoreUnused (shouldBeLatched);
    }
    
    void loadPreset   (const juce::String& presetName) override { imgnProcessor.loadPreset (presetName); }
    void savePreset   (const juce::String& presetName) override { imgnProcessor.savePreset  (presetName); }
    void deletePreset (const juce::String& presetName) override { imgnProcessor.deletePreset (presetName); }
    
private:
    
};
