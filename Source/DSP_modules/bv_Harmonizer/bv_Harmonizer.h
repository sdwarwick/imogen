
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
 
 bv_Harmonizer.h: This file defines the interfaces for the Harmonizer and HarmonizerVoice classes. The Harmonizer class is essentially a synthesizer that makes sound by pitch shifting an input audio signal. The Harmonizer class owns and manages a collection of harmonizerVoice objects to play sound; a single HarmonizerVoice plays one note at a time.
 
======================================================================================================================================================*/


/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_Harmonizer
 vendor:             Ben Vining
 version:            0.0.1
 name:               Harmonizer
 description:        base class for a polyphonic real-time pitch shifting instrument
 dependencies:       bv_SynthBase
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include <climits>  // for INT_MAX

#include "bv_SynthBase/bv_SynthBase.h"  // this file includes the bv_SharedCode header
#include "GrainExtractor/GrainExtractor.h"
#include "psola_resynthesis.h"
#include "bv_HarmonizerVoice.h"



namespace bav
{
    
    
/***********************************************************************************************************************************************
***********************************************************************************************************************************************/

/*
    Harmonizer: base class for the polyphonic instrument owning & managing a collection of HarmonizerVoices
*/

template<typename SampleType>
class Harmonizer  :     public dsp::SynthBase<SampleType>
{
    using AudioBuffer = juce::AudioBuffer<SampleType>;
    using MidiBuffer  = juce::MidiBuffer;
    using Voice = HarmonizerVoice<SampleType>;
    using Base = dsp::SynthBase<SampleType>;
    using FVO = juce::FloatVectorOperations;
    using Analysis_Grain = AnalysisGrain<SampleType>;
    
    
public:
    Harmonizer();
    
    void render (const AudioBuffer& input, AudioBuffer& output, juce::MidiBuffer& midiMessages);
    
    void release() override;
    
    int getLatencySamples() const noexcept { return pitchDetector.getLatencySamples(); }
    
    void updatePitchDetectionHzRange (const int minHz, const int maxHz);
    
    int getCurrentPeriod() const noexcept { return nextFramesPeriod; }
    
    Analysis_Grain* findClosestGrain (int synthesisMarker)
    {
        Analysis_Grain* closestGrain = nullptr;
        int distance = INT_MAX;
        
        for (auto* grain : analysisGrains)
        {
            if (grain->isEmpty())
                continue;
            
            const auto newDist = abs(synthesisMarker - grain->getStartSample());
            
            if (closestGrain == nullptr || newDist < distance)
            {
                closestGrain = grain;
                distance = newDist;
            }
        }
        
        return closestGrain;
    }
    
    
    
private:
    friend class HarmonizerVoice<SampleType>;
    
    void analyzeInput (const AudioBuffer& inputAudio);
    
    void initialized (const double initSamplerate, const int initBlocksize) override;
    
    void prepared (int blocksize) override;
    
    void samplerateChanged (double newSamplerate) override;
    
    void addNumVoices (const int voicesToAdd) override;
    
    
    dsp::PitchDetector<SampleType> pitchDetector;
    
    GrainExtractor<SampleType> grains;
    juce::Array<int> indicesOfGrainOnsets;
    
    // the arbitrary "period" imposed on the signal for analysis for unpitched frames of audio will be randomized within this range
    // NB max value should be 1 greater than the largest possible generated number 
    const juce::Range<int> unpitchedArbitraryPeriodRange { 50, 201 };
    
    AudioBuffer inputStorage;
    
    juce::OwnedArray<Analysis_Grain> analysisGrains;
    
    Analysis_Grain* getEmptyGrain()
    {
        for (auto* grain : analysisGrains)
            if (grain->isEmpty())
                return grain;
        
        return nullptr;
    }
    
    int nextFramesPeriod = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};


} // namespace
