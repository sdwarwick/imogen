
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

#include "PluginProcessor/PluginProcessor.h"


using namespace Imogen;


class ImogenAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public ImogenGuiHolder
{
    
public:
    
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);
    
    ~ImogenAudioProcessorEditor() override;
    
    /*=========================================================================================*/
    /* ProcessorStateChangeSender functions */
    
    void sendParameterChange  (ParameterID param, float newValue) override final;
    void sendParameterGesture (ParameterID param, bool gestureStart) override final;
    
    /*=========================================================================================*/
    /* juce::Component functions */
    
    void paint (juce::Graphics&) override final;
    void resized() override final;
    
    /*=========================================================================================*/
    
private:
    ImogenAudioProcessor& imgnProcessor; // reference to the processor that created this editor
    
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
