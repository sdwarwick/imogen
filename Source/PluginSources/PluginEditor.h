
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
 
 PluginEditor.h: This file defines the binding between Imogen's top-level GUI component and an ImogenAudioProcessor.
 
======================================================================================================================================================*/


#pragma once

#include "PluginProcessor.h"


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public ImogenGuiHolder
{
    using ids = ImogenAudioProcessor::parameterID;
    using event = ImogenAudioProcessor::eventID;
    
    
public:
    
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
    
    ~ImogenAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override final;
    void resized() override final;
    
    void sendParameterChange (int paramID, float newValue) override final;
    
    void startParameterChangeGesture (int paramID) override final;
    void endParameterChangeGesture   (int paramID) override final;
    
    void sendEditorPitchbend (int wheelValue) override final;
    
    void sendMidiLatch (bool shouldBeLatched) override final;
    
    void loadPreset   (const juce::String& presetName) override final;
    void savePreset   (const juce::String& presetName) override final;
    void deletePreset (const juce::String& presetName) override final;
    
    void enableAbletonLink (bool shouldBeEnabled) override final;
    
    
private:
    ImogenAudioProcessor& imgnProcessor; // reference to the processor that created this editor
    
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
