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
#include "GuiHandle.h"
#include "LookAndFeel/ImogenLookAndFeel.h"



class ImogenGUI  :     public juce::Component
{
public:
    
    ImogenGUI (ImogenGuiHandle* h);
    
    virtual ~ImogenGUI() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void parameterChangeRecieved (int paramID, float newValue);
    
    void updateParameterDefaults();
    
    void presetNameChanged (const juce::String& newPresetName);
    
    void mts_connectionChange (bool isNowConnected);
    void mts_scaleChange (const juce::String& newScaleName);
    
    void abletonLinkChange (bool isNowEnabled) override { juce::ignoreUnused (isNowEnabled); }
    
    
private:
    
    // Called to send a parameter change update to the processor from the GUI
    void sendParameterChange (int paramID, float newValue) { handle->sendParameterChange (paramID, newValue); }
    
    void changeDialDisplay (bool displayPitchCorrection, int paramID);
    
    juce::Component* getComponentForParameter (int paramID);
    
    inline void makePresetMenu (juce::ComboBox& box);
    
    juce::ComboBox selectPreset;
    
    bav::ImogenLookAndFeel lookAndFeel;
    
    ImogenGuiHandle* const handle;
    
    juce::TooltipWindow tooltipWindow;
    static constexpr int msBeforeTooltip = 700;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenGUI)
};
