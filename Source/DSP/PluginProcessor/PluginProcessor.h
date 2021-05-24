
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
 
 PluginProcessor.h: This file defines the core interface for Imogen's AudioProcessor as a whole. The class ImogenAudioProcessor is the top-level object that represents an instance of Imogen.
 
======================================================================================================================================================*/


#pragma once

#include "ImogenEngine/ImogenEngine.h"

#include "ImogenCommon/ImogenCommon.h"

#ifndef IMOGEN_HEADLESS
#define IMOGEN_HEADLESS 0
#endif

#if !IMOGEN_HEADLESS
#include <../../third-party/ableton-link/include/ableton/Link.hpp>
#endif


#if !IMOGEN_HEADLESS
class ImogenAudioProcessorEditor;
#endif


struct ImogenGUIUpdateReciever
{
    virtual void applyValueTreeStateChange (const void* encodedChangeData, size_t encodedChangeDataSize) = 0;
};


using namespace Imogen;


/*
*/

class ImogenAudioProcessor : public juce::AudioProcessor,
                             private juce::Timer,
                             private bav::SystemInitializer
{
    using Parameter = bav::Parameter;
    using RAP = juce::RangedAudioParameter;

    using ParameterID               = Imogen::ParameterID;
    using MeterID                   = Imogen::MeterID;
    using NonAutomatableParameterID = Imogen::NonAutomatableParameterID;

    using ChangeDetails = juce::AudioProcessorListener::ChangeDetails;


public:
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;

    /*=========================================================================================*/

    void timerCallback() override final;

    /*=========================================================================================*/
    /* juce::AudioProcessor functions */

    void prepareToPlay (double sampleRate, int samplesPerBlock) override final;

    void releaseResources() override final;

    void reset() override final;

    void processBlock (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages) override final;
    void processBlock (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages) override final;

    void processBlockBypassed (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages) override final;
    void processBlockBypassed (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages) override final;

    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override final;

    double getTailLengthSeconds() const override final;

    void getStateInformation (juce::MemoryBlock& destData) override final;
    void setStateInformation (const void* data, int sizeInBytes) override final;

    juce::AudioProcessorParameter* getBypassParameter() const override final { return mainBypassPntr; }

    int                getNumPrograms() override final { return 1; }
    int                getCurrentProgram() override final { return 0; }
    void               setCurrentProgram (int) override final { }
    const juce::String getProgramName (int) override final { return {}; }
    void               changeProgramName (int, const juce::String&) override final { }

    bool acceptsMidi() const override final { return true; }
    bool producesMidi() const override final { return true; }
    bool supportsMPE() const override final { return false; }
    bool isMidiEffect() const override final { return false; }

    const juce::String getName() const override final { return JucePlugin_Name; }
    juce::StringArray  getAlternateDisplayNames() const override final { return {"Imgn"}; }

    bool hasEditor() const override final;

    juce::AudioProcessorEditor* createEditor() override final;

    bool supportsDoublePrecisionProcessing() const override final { return true; }

    /*=========================================================================================*/

    inline bool isMidiLatched() const;

    bool isAbletonLinkEnabled() const;

    int getNumAbletonLinkSessionPeers() const;

    bool isConnectedToMtsEsp() const noexcept;

    juce::String getScaleName() const;

    /*=========================================================================================*/

    juce::Point< int > getLastEditorSize() const { return savedEditorSize; }

    void saveEditorSize (int width, int height);

    /*=========================================================================================*/

private:
    /*=========================================================================================*/
    /* Initialization functions */

    template < typename SampleType >
    void initialize (bav::ImogenEngine< SampleType >& activeEngine);

    BusesProperties makeBusProperties() const;

    template < typename SampleType >
    void initializeParameterFunctionPointers (bav::ImogenEngine< SampleType >& engine);

    /*=========================================================================================*/

    template < typename SampleType1, typename SampleType2 >
    void prepareToPlayWrapped (const double sampleRate, bav::ImogenEngine< SampleType1 >& activeEngine, bav::ImogenEngine< SampleType2 >& idleEngine);

    template < typename SampleType >
    inline void processBlockWrapped (juce::AudioBuffer< SampleType >& buffer,
                                     juce::MidiBuffer&                midiMessages,
                                     bav::ImogenEngine< SampleType >& engine,
                                     const bool                       isBypassedThisCallback);

    /*=========================================================================================*/

    void changeMidiLatchState (bool isNowLatched);

    /*=========================================================================================*/

    void updateMeters (ImogenMeterData meterData);

    /*=========================================================================================*/

    ImogenGUIUpdateReciever* getActiveGuiEventReciever() const;

    void saveEditorSizeToValueTree();
    void updateEditorSizeFromValueTree();

    /*=========================================================================================*/

    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine< float >  floatEngine;
    bav::ImogenEngine< double > doubleEngine;

    /*=========================================================================================*/

    RAP* mainBypassPntr; // this one gets referenced specifically...

    juce::Point< int > savedEditorSize;
    
    Imogen::Parameters parameters;
    
    Imogen::Meters meters;

#if !IMOGEN_HEADLESS
    ableton::Link abletonLink; // this object represents the plugin as a participant in an Ableton Link session
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
