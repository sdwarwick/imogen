/*==========================================================================================================================
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
 
 ImogenCommon.h: This file defines some common constants that the various modules of Imogen all refer to.
 
==========================================================================================================================*/


/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenCommon
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenCommon
 description:        This Juce module declares some project-wide settings and data for Imogen as a whole.
 dependencies:       bv_SharedCode
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_SharedCode/bv_SharedCode.h"

#include "ImogenParameters.h"


namespace Imogen
{


static inline juce::File presetsFolder() { return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen"); }


static inline juce::File findAppropriateTranslationFile()
{
    // things to test:
    // juce::SystemStats::getDisplayLanguage()
    // juce::SystemStats::getUserLanguage()
    // juce::SystemStats::getUserRegion()
    return { };
}



static inline juce::String getPresetFileExtension()
{
    return { ".xml" };
}


/*=========================================================================================*/
/*=========================================================================================*/


namespace ValueTreeIDs  /* Identifiers for the branches of Imogen's top-level ValueTree */
{
#define IMOGEN_DECLARE_VALUETREEID(name) static inline juce::Identifier name { "name" }

    IMOGEN_DECLARE_VALUETREEID (Imogen);  // the type that the top-level tree will have
    IMOGEN_DECLARE_VALUETREEID (Parameters);
    IMOGEN_DECLARE_VALUETREEID (Meters);
    IMOGEN_DECLARE_VALUETREEID (Properties);

    IMOGEN_DECLARE_VALUETREEID (SavedEditorSize);
    IMOGEN_DECLARE_VALUETREEID (SavedEditorSize_X);
    IMOGEN_DECLARE_VALUETREEID (SavedEditorSize_Y);

#undef IMOGEN_DECLARE_VALUETREEID
}  // namespace


/*=========================================================================================*/


static inline void buildImogenMainValueTree (juce::ValueTree& topLevelTree,
                                             const juce::AudioProcessorParameterGroup& parameterTree,
                                             const bav::NonParamValueTreeNodeGroup& nonAutomatableTree)
{
    // create the parameter tree
    if (auto* paramGroup = bav::findParameterSubgroup (&parameterTree, parameterTreeName()))
    {
        juce::ValueTree parameters { ValueTreeIDs::Parameters };
        
        bav::createValueTreeFromParameterTree (parameters, *paramGroup);
        
        topLevelTree.addChild (parameters, -1, nullptr);
    }
    else
    {
        jassertfalse;
    }

    // create the meter parameter tree
    if (auto* meterGroup = bav::findParameterSubgroup (&parameterTree, meterTreeName()))
    {
        juce::ValueTree meters { ValueTreeIDs::Meters };
        
        bav::createValueTreeFromParameterTree (meters, *meterGroup);
        
        topLevelTree.addChild (meters, -1, nullptr);
    }
    else
    {
        jassertfalse;
    }
    
    
    /* create the rest of the ValueTree that's not bound to actual paramter objects... */
    juce::ValueTree nonParameters { ValueTreeIDs::Properties };
    
    bav::createValueTreeFromNonParamNodes (nonParameters, nonAutomatableTree);
    
    topLevelTree.addChild (nonParameters, -1, nullptr);
}


}  // namespace

