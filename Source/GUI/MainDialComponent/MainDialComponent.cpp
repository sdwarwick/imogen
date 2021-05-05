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


ImogenDialComponent::ImogenDialComponent()
{
    setOpaque (true);
    setInterceptsMouseClicks (true, true);
    
    showPitchCorrection();
}


void ImogenDialComponent::paint (juce::Graphics& g)
{
    juce::ignoreUnused (g);
    
    if (showingPitchCorrection.load())
    {
        
    }
    else
    {
        
    }
}

void ImogenDialComponent::resized()
{
    
}


void ImogenDialComponent::showPitchCorrection()
{
    showingPitchCorrection.store (true);
    setTooltip (TRANS ("Intonation"));
    this->repaint();
}

void ImogenDialComponent::showParameter (ParameterID paramID)
{
    juce::ignoreUnused (paramID);
    showingPitchCorrection.store (false);
    setTooltip (getParameterNameVerbose (paramID));
    this->repaint();
}


void ImogenDialComponent::newIntonationData (const juce::String& noteName, int centsOffPitch)
{
    juce::ignoreUnused (noteName, centsOffPitch);
}


bool ImogenDialComponent::hitTest (int x, int y)
{
    juce::ignoreUnused (x, y);
    return false;
}


bool ImogenDialComponent::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool ImogenDialComponent::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void ImogenDialComponent::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void ImogenDialComponent::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}
