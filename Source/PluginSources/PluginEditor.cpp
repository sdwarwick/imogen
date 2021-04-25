
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
#include "bv_SharedCode/BinaryDataHelpers.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p)
    : AudioProcessorEditor (&p), imgnProcessor(p)
{
    this->setBufferedToImage (true);
    
    setResizable (true, true);
    const auto size = imgnProcessor.getLastEditorSize();
    setSize (size.x, size.y);
    setResizeLimits (800, 450, 2990, 1800);
    
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

