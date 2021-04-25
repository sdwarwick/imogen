

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "ImogenGuiHolder.h"


class MainComponent  : public juce::Component,
                       public ImogenGuiHolder
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
    
    void loadPreset   (const juce::String& presetName) override
    {
        juce::ignoreUnused (presetName);
    }
    
    void savePreset   (const juce::String& presetName) override
    {
        juce::ignoreUnused (presetName);
    }
    
    void deletePreset (const juce::String& presetName) override
    {
        juce::ignoreUnused (presetName);
    }


private:
    ImogenGUI gui;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
