
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
 
 ImogenGuiHolder.h: This file defines a class that holds an ImogenGui object and acts as its interface with the outside world.
    Note that ImogenGuiHolder is an abstract class, because it inherits from ProcessorStateChangeSender and doesn't implement all of its pure virtual methods.
 
 ======================================================================================================================================================*/


#pragma once

#include "GUI/ImogenGUI.h"

#include "../../Common/ImogenParameters.h"

 
using namespace Imogen;


class ImogenGuiHolder
{
public:
    ImogenGuiHolder(): parameterTree(createParameterTree()), state(imogenValueTreeType())
    {
        createValueTree (state, *(parameterTree));
    }
    
    virtual ~ImogenGuiHolder() = default;
    
    /*=========================================================================================*/
    
    ImogenGUIState returnState() const { return ImogenGUIState(&p_gui); }
    
    void saveState (ImogenGUIState& guistate) { guistate.saveState (&p_gui); }
    
    void restoreState (const ImogenGUIState& guistate) { guistate.resoreState (&p_gui); }
    
    /*=========================================================================================*/
    
    const ImogenGUI* gui() const noexcept { return &p_gui; }
    
protected:
    ImogenGUI* gui() noexcept { return &p_gui; }
    
    /*=========================================================================================*/
    
private:
    ImogenGUI p_gui;
    
    std::unique_ptr<juce::AudioProcessorParameterGroup> parameterTree;
    juce::ValueTree state;
};
