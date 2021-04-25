
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
#include "../GUI/ImogenGUI.h"


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public ImogenGuiHandle,
                                    public ImogenParameterReciever
{
    using ids = ImogenAudioProcessor::parameterID;
    using event = ImogenAudioProcessor::eventID;
    
    
public:
    
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
    
    ~ImogenAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
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
    
    void recieveParameterChange (int paramID, float newValue) override
    {
        gui.parameterChangeRecieved (paramID, newValue);
    }
    
    void parameterDefaultsUpdated() override
    {
        gui.updateParameterDefaults();
    }
    
    void loadPreset   (const juce::String& presetName) override { imgnProcessor.loadPreset (presetName); }
    void savePreset   (const juce::String& presetName) override { imgnProcessor.savePreset  (presetName); }
    void deletePreset (const juce::String& presetName) override { imgnProcessor.deletePreset (presetName); }
    
    void presetNameChange (const juce::String& newPresetName) override { juce::ignoreUnused (newPresetName); }
    
    void mts_connectionChange (bool isNowConnected) override { juce::ignoreUnused (isNowConnected); }
    void mts_scaleChange (const juce::String& newScaleName) override { juce::ignoreUnused (newScaleName); }
    
    void abletonLinkChange (bool isNowEnabled) override { juce::ignoreUnused (isNowEnabled); }
    
    
    
private:
    
    ImogenGUI gui;
    
    ImogenAudioProcessor& imgnProcessor; // reference to the processor that created this editor
    
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
