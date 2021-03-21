/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_Harmonizer
 vendor:             Ben Vining
 version:            0.0.1
 name:               Harmonizer
 description:        base class for a polyphonic real-time pitch shifting instrument
 dependencies:       bv_PitchDetector
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_PitchDetector/bv_PitchDetector.h"  // this file includes the bv_SharedCode header
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
    
    
public:
    
    // NB. I play a bit fast and loose with private vs public functions here, because really, you should never interface directly with any non-const methods of HarmonizerVoice from outside the Harmonizer class that owns it...
    
    HarmonizerVoice (Harmonizer<SampleType>* h);
    
    void renderNextBlock (const AudioBuffer& inputAudio, AudioBuffer& outputBuffer,
                          const int origPeriod,
                          const juce::Array<int>& indicesOfGrainOnsets,
                          const AudioBuffer& windowToUse);
    
    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate) override;
    
    
    // DANGER!!! FOR NON REALTIME USE ONLY!
    void increaseBufferSizes (const int newMaxBlocksize);
    
    
    
private:
    friend class Harmonizer<SampleType>;
    
    Harmonizer<SampleType>* parent;
    
    void prepare (const int blocksize) override;
    
    void released() override;
    
    void noteCleared() override;
    
    void sola (const SampleType* input, const int totalNumInputSamples,
               const int origPeriod, const int newPeriod, const juce::Array<int>& indicesOfGrainOnsets,
               const SampleType* window);
    
    void olaFrame (const SampleType* inputAudio, const int frameStartSample, const int frameEndSample, const int frameSize, 
                   const SampleType* window, const int newPeriod);
    
    void moveUpSamples (const int numSamplesUsed);
    
    
    AudioBuffer synthesisBuffer; // mono buffer that this voice's synthesized samples are written to
    AudioBuffer copyingBuffer;
    AudioBuffer windowingBuffer; // used to apply the window to the analysis grains before OLA, so windowing only needs to be done once per analysis grain
    
    int synthesisIndex;
    
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
    using MidiMessage = juce::MidiMessage;
    
    using Voice = HarmonizerVoice<SampleType>;
    
    using ADSRParams = juce::ADSR::Parameters;
    
    using Base = dsp::SynthBase<SampleType>;
    
    using FVO = juce::FloatVectorOperations;
    
    
public:
    Harmonizer();
    
    void release() override;
    
    void analyzeInput (const AudioBuffer& inputAudio);
    
    int getLatencySamples() const noexcept { return pitchDetector.getLatencySamples(); }
    
    void updatePitchDetectionHzRange (const int minHz, const int maxHz);
    
    
    
private:
    friend class HarmonizerVoice<SampleType>;
    
    void initialized (const double initSamplerate, const int initBlocksize) override;
    
    void prepared (int blocksize) override;
    
    void samplerateChanged (double newSamplerate) override;
    
    void addNumVoices (const int voicesToAdd) override;
    
    void fillWindowBuffer (const int numSamples);
    
    
    
    PitchDetector<SampleType> pitchDetector;
    
    GrainExtractor<SampleType> grains;
    juce::Array<int> indicesOfGrainOnsets;
    
    // the arbitrary "period" imposed on the signal for analysis for unpitched frames of audio will be randomized within this range
    // NB max value should be 1 greater than the largest possible generated number 
    const juce::Range<int> unpitchedArbitraryPeriodRange { 50, 201 };
    
    AudioBuffer windowBuffer;
    int windowSize;
    
    AudioBuffer inputStorage;
    
    AudioBuffer& thisFramesInput = inputStorage;
    
    float currentInputFreq;
    
    int nextFramesPeriod = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};


} // namespace
