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
 
 MainDialComponent.h: This file defines the large interactive dial component in the center of Imogen's GUI.
 
 ======================================================================================================================================================*/

#include <juce_gui_extra/juce_gui_extra.h>

#include "../GUI_Framework.h"


class ImogenDialComponent  :    public juce::Component
{
public:
    ImogenDialComponent(ImogenGuiHandle* h): holder(h) { jassert (holder != nullptr); }
    
    void paint (juce::Graphics& g) override final;
    
    void resized() override final;
    
    /* This function is overridden so that the component only accepts mouse clicks within the circular area of the dial. */
    bool hitTest (int x, int y) override final;
    
    bool keyPressed (const juce::KeyPress& key) override final;
    bool keyStateChanged (bool isKeyDown) override final;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override final;
    void focusLost (FocusChangeType cause) override final;
    
    //
    
    void showPitchCorrection();
    void showParameter (int paramID);
    bool isShowingPitchCorrection() const noexcept { return showingPitchCorrection.load(); }
    
    //
    
    void newIntonationData (const juce::String& noteName, int centsOffPitch);
    
private:
    ImogenGuiHandle* const holder;
    std::atomic<bool> showingPitchCorrection;
};
