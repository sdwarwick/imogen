
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
 
 ImogenGUI.cpp: This file defines implementation details for Imogen's top-level GUI component.

===================================================================================================================================================*/


#include "bv_SharedCode/BinaryDataHelpers.h"


#define bvi_GRAPHICS_FRAMERATE_HZ 60


ImogenGUI::ImogenGUI()
{
    this->setBufferedToImage (true);
    
    makePresetMenu (selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    //addAndMakeVisible(selectPreset);
    
    setSize (940, 435);
    
    Timer::startTimerHz (bvi_GRAPHICS_FRAMERATE_HZ);
}

#undef bvi_GRAPHICS_FRAMERATE_HZ


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel(nullptr);
    Timer::stopTimer();
