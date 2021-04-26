
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
    Note that ImogenGuiHolder is an abstract class, because it inherits from ImogenGuiHandle and doesn't implement any of its pure virtual methods.
 
 ======================================================================================================================================================*/


#pragma once

#include "GUI/ImogenGUI.h"


using namespace Imogen;


class ImogenGuiHolder  :    public ImogenGuiHandle
{
public:
    ImogenGuiHolder(): p_gui(this) { }
    
    virtual ~ImogenGuiHolder() = default;
    
    //
    
    void recieveParameterChange (parameterID paramID, float newValue) { p_gui.parameterChangeRecieved (paramID, newValue); }
    
    void recieveParameterChangeGestureStart (parameterID paramID) { p_gui.parameterChangeGestureStarted (paramID); }
    
    void recieveParameterChangeGestureEnd (parameterID paramID) { p_gui.parameterChangeGestureEnded (paramID); }
    
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
