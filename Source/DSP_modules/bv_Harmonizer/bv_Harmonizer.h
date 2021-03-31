
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

#include "bv_SynthBase/bv_SynthBase.h"  // this file includes the bv_SharedCode header
#include "GrainExtractor/GrainExtractor.h"



namespace bav
{
    

template<typename SampleType>
class Harmonizer; // forward declaration...


    
/*
 HarmonizerVoice : represents a "voice" that the Harmonizer can use to generate one monophonic note. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
*/
    
template<typename SampleType>
class HarmonizerVoice  :    public dsp::SynthVoiceBase<SampleType>
{
    
    using AudioBuffer = juce::AudioBuffer<SampleType>;
    using FVO = juce::FloatVectorOperations;
    using Base = dsp::SynthVoiceBase<SampleType>;
    
    /*
        This class represents one analysis grain that is being resynthesized by the voice.
        The voice always has two active grains: one fading out, and one fading in.
    */
    class Grain
    {
    public:
        Grain() { }
        
        ~Grain() { }
        
        void storeNewGrain (const SampleType* inputSamples, const int startSample, const int numSamples, const SampleType* window,
                            const int synthesisMarker)
        {
            nextSample    = 0;
            inStartSample = startSample;
            inEndSample   = startSample + numSamples;
            size          = numSamples;
            zeroesLeft    = synthesisMarker - startSample;
            synthesisMark = synthesisMarker;
            
            jassert (size > 1);
            
            samples.clear();
            vecops::copy (inputSamples + startSample, samples.getWritePointer(0), numSamples);
            vecops::multiplyV (samples.getWritePointer(0), window, numSamples);
        }
        
        SampleType getNextSample()
        {
            jassert (size > 0);
            
            if (zeroesLeft > 0)
            {
                --zeroesLeft;
                return SampleType(0);
            }
            else if (zeroesLeft < 0)
            {
                ++zeroesLeft;
                return SampleType(0);
            }
            
            return samples.getSample (0, nextSample++);
        }
        
        void skipSamples (const int numSamples)
        {
            for (int i = 0; i < numSamples; ++i) {
                skipSample();
            }
        }
        
        void skipSample()
        {
            if (zeroesLeft > 0)
            {
                --zeroesLeft;
            }
            else
            {
                ++nextSample;
                if (nextSample >= size) nextSample = 0;
            }
        }
        
        bool isDone() const noexcept
        {
            return size == 0 || nextSample >= size;
        }
        
        void clear()
        {
            inStartSample = 0;
            inEndSample = 0;
            synthesisMark = 0;
            nextSample = 0;
            zeroesLeft = 0;
            size = 0;
            samples.clear();
        }
        
        int getLastStartIndex() const noexcept { return inStartSample; }
        
        int getLastEndIndex() const noexcept { return inEndSample; }
        
        int getLastSynthesisMark() const noexcept { return synthesisMark; }
        
    private:
        int inStartSample = 0; // the start sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        int inEndSample   = 0; // the end sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        
        int zeroesLeft = 0; // the number of zeroes the grain should output before outputting its next stored samples

        int nextSample = 0; // the next sample index to be read from the buffer
        
        int synthesisMark = 0; /// the scaled placement of the grain in the output signal
        
        int size = 0;
        
        AudioBuffer samples; // this buffer stores the input samples with the window applied
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grain)
    };
    
    
public:
    
    HarmonizerVoice (Harmonizer<SampleType>* h);
    
    // DANGER!!! FOR NON REALTIME USE ONLY!
    void increaseBufferSizes (const int newMaxBlocksize);
    
    
    void dataAnalyzed (const int period);
    
    
private:
    friend class Harmonizer<SampleType>;
    
    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int origStartSample) override;
    
    void bypassedBlockRecieved (int numSamples) override;
    
    Harmonizer<SampleType>* parent;
    
    void prepare (const int blocksize) override;
    
    void released() override;
    
    void noteCleared() override;
    
//    void sola (const SampleType* input, const int totalNumInputSamples,
//               const int origPeriod, const int newPeriod, const juce::Array<int>& indicesOfGrainOnsets,
//               const SampleType* window);
//
//    void olaFrame (const SampleType* inputAudio, const int frameStartSample, const int frameEndSample, const int frameSize,
//                   const SampleType* window, const int newPeriod);
    
    void moveUpSamples (const int numSamplesUsed);
    
    
    inline SampleType getNextSample (const SampleType* inputSamples,
                                     const SampleType* window,
                                     const int grainSize, const int origPeriodTimesScaleFactor);
    
    inline void storeNewGrain (Grain& grain, const SampleType* inputSamples, const int grainSize, const SampleType* window,
                               const int origPeriodTimesScaleFactor);
    
    AudioBuffer synthesisBuffer; // mono buffer that this voice's synthesized samples are written to
    AudioBuffer copyingBuffer;
    
    int synthesisIndex;
    
    int nextFramesPeriod = 0;
    
    Grain grain1, grain2;
    
    int lastUsedGrainInArray = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


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
    
    void samplerateChanged (double newSamplerate) override;
    
    void addNumVoices (const int voicesToAdd) override;
    
    void fillWindowBuffer (const int numSamples);
    
    
    
    dsp::PitchDetector<SampleType> pitchDetector;
    
    GrainExtractor<SampleType> grains;
    juce::Array<int> indicesOfGrainOnsets;
    
    // the arbitrary "period" imposed on the signal for analysis for unpitched frames of audio will be randomized within this range
    // NB max value should be 1 greater than the largest possible generated number 
    const juce::Range<int> unpitchedArbitraryPeriodRange { 50, 201 };
    
    AudioBuffer windowBuffer;
    int windowSize;
    
    AudioBuffer inputStorage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};


} // namespace
