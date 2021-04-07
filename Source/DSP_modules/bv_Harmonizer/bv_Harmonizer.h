
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
 
 bv_Harmonizer.h: This file defines the interfaces for the Harmonizer class. The Harmonizer class is essentially a synthesizer that makes sound by pitch shifting an input audio signal. The Harmonizer class owns and manages a collection of harmonizerVoice objects to play sound; a single HarmonizerVoice plays one note at a time.
 
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

#include "bv_SynthBase/bv_SynthBase.h"  // this file includes the bv_SharedCode header
#include "PSOLA/granular_resynthesis.h"
#include "PSOLA/psola_analyzer.h"
#include "PSOLA/psola_shifter.h"
#include "bv_HarmonizerVoice.h"
#include "AutoPitchCorrector.h"



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
    using Analysis_Grain = AnalysisGrain<SampleType>;
    
    
public:
    Harmonizer();
    
    void render (const AudioBuffer& input, AudioBuffer& output, juce::MidiBuffer& midiMessages);
    
    void release() override;
    
    int getLatencySamples() const noexcept { return pitchDetector.getLatencySamples(); }
    
    void updatePitchDetectionHzRange (const int minHz, const int maxHz);
    
    
private:
    friend class HarmonizerVoice<SampleType>;
    
    void analyzeInput (const AudioBuffer& inputAudio);
    
    void initialized (const double initSamplerate, const int initBlocksize) override;
    
    void prepared (int blocksize) override;
    
    void resetTriggered() override;
    
    void samplerateChanged (double newSamplerate) override;
    
    void addNumVoices (const int voicesToAdd) override;
    
    
    dsp::PitchDetector<SampleType> pitchDetector;
    
    AudioBuffer inputStorage;
    
    PsolaAnalyzer<SampleType> analyzer;
    
    AutoPitch<SampleType> autoPitch;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};


} // namespace
