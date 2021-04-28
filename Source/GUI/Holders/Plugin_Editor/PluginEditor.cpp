
/*===================================================================================================================================================
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
 
 PluginEditor.cpp: This file defines implementation details for Imogen's top-level GUI component

===================================================================================================================================================*/


#include "PluginEditor.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), imgnProcessor(p)
{
    this->setBufferedToImage (true);
    
    addAndMakeVisible (gui());
    
    setResizable (true, true);
    setResizeLimits (800, 450, 2990, 1800);
    const auto size = imgnProcessor.getLastEditorSize();
    setSize (size.x, size.y);
    
#if JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent());
#endif
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
#if JUCE_OPENGL
    openGLContext.detach();
#endif
}


void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}


void ImogenAudioProcessorEditor::resized()
{
    imgnProcessor.saveEditorSize ({ getWidth(), getHeight() });
    
    gui()->setBounds (0, 0, getWidth(), getHeight());
}


/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessorEditor::sendParameterChange (ParameterID paramID, float newValue)
{
    imgnProcessor.recieveParameterChange (paramID, newValue);
}

void ImogenAudioProcessorEditor::sendParameterChangeGestureStart (ParameterID paramID)
{
    imgnProcessor.recieveParameterChangeGestureStart (paramID);
}

void ImogenAudioProcessorEditor::sendParameterChangeGestureEnd   (ParameterID paramID)
{
    imgnProcessor.recieveParameterChangeGestureEnd (paramID);
}

void ImogenAudioProcessorEditor::sendEditorPitchbend (int wheelValue)
{
    imgnProcessor.recieveEditorPitchbendEvent (wheelValue);
}

void ImogenAudioProcessorEditor::sendMidiLatch (bool shouldBeLatched)
{
    juce::ignoreUnused (shouldBeLatched);
}

void ImogenAudioProcessorEditor::sendLoadPreset (const juce::String& presetName)
{
    imgnProcessor.loadPreset (presetName);
}

void ImogenAudioProcessorEditor::sendSavePreset (const juce::String& presetName)
{
    imgnProcessor.savePreset  (presetName);
}

void ImogenAudioProcessorEditor::sendDeletePreset (const juce::String& presetName)
{
    imgnProcessor.deletePreset (presetName);
}

void ImogenAudioProcessorEditor::sendEnableAbletonLink (bool shouldBeEnabled)
{
    imgnProcessor.recieveAbletonLinkChange (shouldBeEnabled);
}
