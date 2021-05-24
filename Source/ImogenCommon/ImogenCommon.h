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
 dependencies:       bv_midi
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_midi/bv_midi.h"

#include "ImogenParameters.h"


namespace Imogen
{
static inline juce::File presetsFolder()
{
    return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen");
}


static inline juce::File findAppropriateTranslationFile()
{
    // things to test:
    // juce::SystemStats::getDisplayLanguage()
    // juce::SystemStats::getUserLanguage()
    // juce::SystemStats::getUserRegion()
    return {};
}


static inline juce::String getPresetFileExtension()
{
    return {".xml"};
}


/*=========================================================================================*/
/*=========================================================================================*/


namespace ValueTreeIDs /* Identifiers for the branches of Imogen's top-level ValueTree */
{
#define IMOGEN_DECLARE_VALUETREEID(name)                                                                                                             \
    static inline juce::Identifier name { "name" }

IMOGEN_DECLARE_VALUETREEID (Imogen); // the type that the top-level tree will have
IMOGEN_DECLARE_VALUETREEID (Parameters);
IMOGEN_DECLARE_VALUETREEID (Meters);
IMOGEN_DECLARE_VALUETREEID (Properties);

IMOGEN_DECLARE_VALUETREEID (SavedEditorSize);
IMOGEN_DECLARE_VALUETREEID (SavedEditorSize_X);
IMOGEN_DECLARE_VALUETREEID (SavedEditorSize_Y);

#undef IMOGEN_DECLARE_VALUETREEID
} // namespace ValueTreeIDs


/*=========================================================================================*/


} // namespace Imogen
