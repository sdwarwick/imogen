

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
    
    //==============================================================================
    void sendParameterChange (int paramID, float newValue) override;
    
    void sendEditorPitchbend (int wheelValue) override;
    
    void sendMidiLatch (bool shouldBeLatched) override;
    
    void loadPreset   (const juce::String& presetName) override;
    
    void savePreset   (const juce::String& presetName) override;
    
    void deletePreset (const juce::String& presetName) override;


private:
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
