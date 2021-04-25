

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "ImogenGuiHolder.h"
#include "OSC_parser.h"


class MainComponent  : public juce::Component,
                       public ImogenGuiHolder
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override final;
    void resized() override final;
    
    //==============================================================================
    void sendParameterChange (int paramID, float newValue) override final;
    
    void startParameterChangeGesture (int paramID) override final;
    void endParameterChangeGesture (int paramID) override final;
    
    void sendEditorPitchbend (int wheelValue) override final;
    
    void sendMidiLatch (bool shouldBeLatched) override final;
    
    void loadPreset   (const juce::String& presetName) override final;
    
    void savePreset   (const juce::String& presetName) override final;
    
    void deletePreset (const juce::String& presetName) override final;
    
    void enableAbletonLink (bool shouldBeEnabled) override final;


private:
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    
    juce::OSCReceiver oscReceiver;
    OSC_Parser oscParser;
    
    juce::OSCSender oscSender;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
