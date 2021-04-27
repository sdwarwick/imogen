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


MainComponent::MainComponent(): oscParser(gui())
{
    this->setBufferedToImage (true);
    
    addAndMakeVisible (gui());
    
    setSize (800, 2990);
    
    oscReceiver.addListener (&oscParser);
    
    // connect OSC sender & reciver...
    
#if JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent());
#endif
}

MainComponent::~MainComponent()
{
    oscReceiver.removeListener (&oscParser);
    
    oscSender.disconnect();
    oscReceiver.disconnect();
    
#if JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    gui()->setBounds (0, 0, getWidth(), getHeight());
}


//==============================================================================

/* This app is just a remote control for another instance of Imogen. All its outgoing events are simply rerouted to an OSC message sender... */

void MainComponent::sendParameterChange (ParameterID paramID, float newValue)
{
    oscSender.sendParameterChange (paramID, newValue);
}

void MainComponent::startParameterChangeGesture (ParameterID paramID)
{
    oscSender.startParameterChangeGesture (paramID);
}

void MainComponent::endParameterChangeGesture (ParameterID paramID)
{
    oscSender.endParameterChangeGesture (paramID);
}

void MainComponent::sendEditorPitchbend (int wheelValue) 
{
    oscSender.sendEditorPitchbend (wheelValue);
}

void MainComponent::sendMidiLatch (bool shouldBeLatched) 
{
    oscSender.sendMidiLatch (shouldBeLatched);
}

void MainComponent::loadPreset (const juce::String& presetName) 
{
    oscSender.loadPreset (presetName);
}

void MainComponent::savePreset (const juce::String& presetName) 
{
    oscSender.savePreset (presetName);
}

void MainComponent::deletePreset (const juce::String& presetName) 
{
    oscSender.deletePreset (presetName);
}

void MainComponent::enableAbletonLink (bool shouldBeEnabled)
{
    oscSender.enableAbletonLink (shouldBeEnabled);
}
