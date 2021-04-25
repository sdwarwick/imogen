

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "GuiHandle.h"
#include "ImogenGUI.h"
#include "ImogenRecieverAPI.h"


class MainComponent  : public juce::Component,
                       public ImogenGuiHandle,
                       public ImogenParameterReciever
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
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
    
    void presetNameChange (const juce::String& newPresetName) override { gui.presetNameChanged (newPresetName); }
    
    void mts_connectionChange (bool isNowConnected) override { gui.mts_connectionChange(isNowConnected); }
    void mts_scaleChange (const juce::String& newScaleName) override { gui.mts_scaleChange (newScaleName); }
    
    void abletonLinkChange (bool isNowEnabled) override { gui.abletonLinkChange (isNowEnabled); }
    

private:
    ImogenGUI gui;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
