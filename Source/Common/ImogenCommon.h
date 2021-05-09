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


#pragma once

#ifndef IMOGEN_REMOTE_APP
  #define IMOGEN_REMOTE_APP 0
#endif

#ifndef IMOGEN_HEADLESS
  #define IMOGEN_HEADLESS 0
#endif

#ifndef IMOGEN_USE_ABLETON_LINK
  #define IMOGEN_USE_ABLETON_LINK 0
#endif

#include "Shared-code/bv_SharedCode/bv_SharedCode.h"


namespace Imogen
{


enum EventID
{
    killAllMidiID,
    midiLatchID,
    pitchBendFromEditorID
};
static constexpr int numEventIDs = pitchBendFromEditorID + 1;


enum ParameterID
{
    inputSourceID,
    mainBypassID,
    leadBypassID,
    harmonyBypassID,
    dryPanID,
    dryWetID,
    adsrAttackID,
    adsrDecayID,
    adsrSustainID,
    adsrReleaseID,
    stereoWidthID,
    lowestPannedID,
    velocitySensID,
    pitchBendRangeID,
    pedalPitchIsOnID,
    pedalPitchThreshID,
    pedalPitchIntervalID,
    descantIsOnID,
    descantThreshID,
    descantIntervalID,
    voiceStealingID,
    inputGainID,
    outputGainID,
    limiterToggleID,
    noiseGateToggleID,
    noiseGateThresholdID,
    compressorToggleID,
    compressorAmountID,
    aftertouchGainToggleID,
    deEsserToggleID,
    deEsserThreshID,
    deEsserAmountID,
    reverbToggleID,
    reverbDryWetID,
    reverbDecayID,
    reverbDuckID,
    reverbLoCutID,
    reverbHiCutID,
    delayToggleID,
    delayDryWetID
};
static constexpr int numParams = delayDryWetID + 1;


enum MeterID
{
    inputLevelID,
    outputLevelID,
    gateReduxID,
    compReduxID,
    deEssGainReduxID,
    limiterGainReduxID,
    reverbLevelID,
    delayLevelID
};
static constexpr int numMeters = delayLevelID + 1;


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



namespace ValueTreeIDs  /* Identifiers for the branches of Imogen's top-level ValueTree */
{
#define IMOGEN_DECLARE_VALUETREEID(name) static inline juce::Identifier name { "name" }

    IMOGEN_DECLARE_VALUETREEID (Imogen);  // the type that the top-level tree will have
    IMOGEN_DECLARE_VALUETREEID (Parameters);
    IMOGEN_DECLARE_VALUETREEID (Meters);

#undef IMOGEN_DECLARE_VALUETREEID
}  // namespace


static inline juce::String parameterTreeID()   { return "ImogenParameters"; }
static inline juce::String parameterTreeName() { return TRANS ("Parameters"); }

static inline juce::String meterTreeID()   { return "ImogenMeters"; }
static inline juce::String meterTreeName() { return TRANS ("Meters"); }

static inline juce::String parameterTreeSeparatorString() { return { " | " }; }


static inline void buildImogenMainValueTree (juce::ValueTree& topLevelTree,
                                             const juce::AudioProcessorParameterGroup* parameterTree)
{
    // create the parameter tree
    if (auto* params = bav::findParameterSubgroup (parameterTree, parameterTreeName()))
    {
        juce::ValueTree parameters { ValueTreeIDs::Parameters };
        
        bav::createValueTreeFromParameterTree (parameters, *params);
        
        topLevelTree.addChild (parameters, 0, nullptr);
    }
    else
    {
        jassertfalse;
    }

    // create the meter parameter tree
    if (auto* mtrs = bav::findParameterSubgroup (parameterTree, meterTreeName()))
    {
        juce::ValueTree meters { ValueTreeIDs::Meters };
        
        bav::createValueTreeFromParameterTree (meters, *mtrs);
        
        topLevelTree.addChild (meters, 0, nullptr);
    }
    else
    {
        jassertfalse;
    }
}


static inline void createValueTreeParameterAttachments (juce::ValueTree& topLevelValueTree,
                                                        juce::OwnedArray<bav::ParameterAttachment>& attachments,
                                                        std::function<bav::Parameter*(ParameterID)> findParameter,
                                                        std::function<bav::Parameter*(MeterID)> findMeter)
{
    bav::createParameterValueTreeAttachments (attachments,
                                              topLevelValueTree.getChildWithName (ValueTreeIDs::Parameters),
                                              numParams,
                                              [findParameter](int param) { return findParameter (static_cast<ParameterID> (param)); });
    
    bav::createParameterValueTreeAttachments (attachments,
                                              topLevelValueTree.getChildWithName (ValueTreeIDs::Meters),
                                              numMeters,
                                              [findMeter](int meter) { return findMeter (static_cast<MeterID> (meter)); });
}


}  // namespace

