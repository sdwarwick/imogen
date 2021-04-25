
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


struct ImogenParameterReciever
{
    ImogenParameterReciever() = default;
    
    virtual ~ImogenParameterReciever() = default;
    
    //
    
    virtual void recieveParameterChange (int paramID, float newValue) = 0;
    
    virtual void parameterDefaultsUpdated() = 0;
    
    virtual void mts_connectionChange (bool isNowConnected) = 0;
    
    virtual void mts_scaleChange (const juce::String& newScaleName) = 0;
    
    virtual void presetNameChange (const juce::String& newPresetName) = 0;
    
    virtual void abletonLinkChange (bool isNowEnabled) = 0;
};

