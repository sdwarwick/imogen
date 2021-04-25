/*================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/       _____  ______ __  __  ____ _______ ______
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /       |  __ \|  ____|  \/  |/ __ \__   __|  ____|
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /        | |__) | |__  | \  / | |  | | | |  | |__
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /         |  _  /|  __| | |\/| | |  | | | |  |  __|
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /          | | \ \| |____| |  | | |__| | | |  | |____
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/           |_|  \_\______|_|  |_|\____/  |_|  |______|
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 MainComponent.cpp :     This file defines main content component for the Imogen Remote app, which contains the Imgogen GUI and wraps it with its networking capabilities.
 
 ================================================================================================================================*/


#include "MainComponent.h"


MainComponent::MainComponent()
    : oscParser(this)
{
    this->setBufferedToImage (true);
    
    addAndMakeVisible (gui());
    
    setSize (800, 2990);
    
    oscReceiver.addListener (&oscParser);
    
#if JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent());
#endif
}

MainComponent::~MainComponent()
{
    oscReceiver.removeListener (&oscParser);
    
#if JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    gui()->setBounds (0, 0, getWidth(), getHeight());
}


//==============================================================================

void MainComponent::sendParameterChange (int paramID, float newValue) 
{
    juce::ignoreUnused (paramID, newValue);
}

void MainComponent::startParameterChangeGesture (int paramID)
{
    juce::ignoreUnused (paramID);
}

void MainComponent::endParameterChangeGesture (int paramID)
{
    juce::ignoreUnused (paramID);
}


void MainComponent::sendEditorPitchbend (int wheelValue) 
{
    juce::ignoreUnused (wheelValue);
}


void MainComponent::sendMidiLatch (bool shouldBeLatched) 
{
    juce::ignoreUnused (shouldBeLatched);
}


void MainComponent::loadPreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}

void MainComponent::savePreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}

void MainComponent::deletePreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}


void MainComponent::enableAbletonLink (bool shouldBeEnabled)
{
    juce::ignoreUnused (shouldBeEnabled);
}
