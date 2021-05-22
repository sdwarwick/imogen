
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
    : AudioProcessorEditor (&p)
    , imgnProcessor (p)
    , gui (this)
{
    this->setBufferedToImage (true);

    addAndMakeVisible (gui);

    const auto size = imgnProcessor.getLastEditorSize();
    const auto width = size.x, height = size.y;
    setResizable (true, true);
    getConstrainer()->setMinimumSize (width / 2, height / 2);
    getConstrainer()->setFixedAspectRatio ((float) width / (float) height);
    setSize (width, height);
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
}

/*=========================================================================================================*/

void ImogenAudioProcessorEditor::applyValueTreeStateChange (const void* encodedChangeData, size_t encodedChangeDataSize)
{
    gui.applyValueTreeStateChange (encodedChangeData, encodedChangeDataSize);
}

void ImogenAudioProcessorEditor::sendValueTreeStateChange (const void* encodedChange, size_t encodedChangeSize)
{
    imgnProcessor.applyValueTreeStateChange (encodedChange, encodedChangeSize);
}

/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}


void ImogenAudioProcessorEditor::resized()
{
    imgnProcessor.saveEditorSize (getWidth(), getHeight());

    gui.setBounds (0, 0, getWidth(), getHeight());
}
