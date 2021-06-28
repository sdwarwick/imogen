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
 
 MainDialComponent.cpp: This file defines the large interactive dial component in the center of Imogen's GUI.
 
 ======================================================================================================================================================*/

#include "MainDialComponent.h"


namespace Imogen
{

MainDialComponent::MainDialComponent (State& stateToUse)
: state (stateToUse)
{
    setOpaque (true);
    setInterceptsMouseClicks (true, true);
}


void MainDialComponent::paint (juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState graphicsState (g);

    juce::ignoreUnused (g);
}

void MainDialComponent::resized()
{
}


bool MainDialComponent::hitTest (int x, int y)
{
    juce::ignoreUnused (x, y);
    return false;
}


bool MainDialComponent::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool MainDialComponent::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void MainDialComponent::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void MainDialComponent::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}

void MainDialComponent::showParameter (Parameter& /*param*/)
{
    
}

void MainDialComponent::showPitchCorrection()
{
    
}

}
