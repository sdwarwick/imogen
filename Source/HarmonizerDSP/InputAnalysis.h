/*
 ==============================================================================
 
 InputAnalysis.h
 Created: 14 Dec 2020 6:32:56pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"

template<typename SampleType>
class PitchTracker
{
public:
    PitchTracker();
    ~PitchTracker();
    
    void releaseResources();
    
    
    void prepare (const int blocksize);
    
    SampleType getPitch(const AudioBuffer<SampleType>& inputAudio, const double samplerate);
    
    
private:
    
    
    void computeASDF (const AudioBuffer<SampleType>& input); // fills the ASDF buffer with computed ASDF values for the input signal
    // "neighborhood"? what to use as sample x[n0]?
    
    
    unsigned int minElement(const SampleType* data, const int dataSize) noexcept;
    
    SampleType quadraticPeakPosition (const SampleType* data, unsigned int pos, const int dataSize) noexcept;
    
    
    AudioBuffer<SampleType> asdfBuffer;
    Array<int> asdfMinima;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchTracker)
};



template<typename SampleType>
class EpochFinder
{
public:
    EpochFinder();
    ~EpochFinder();
    
    void extractEpochSampleIndices (const AudioBuffer<SampleType>& inputAudio, const double samplerate, Array<int>& outputArray);
    
    void makeSubsetOfEpochIndicesArray (const Array<int>& epochIndices, Array<int>& outputArray,
                                        const int sampleOffset, const int numSamples);
    
    int averageDistanceBetweenEpochs(const Array<int>& epochIndices);
    
    void prepare (const int blocksize);
    
    void releaseResources();
    
private:
    CriticalSection lock;
    
    Array<SampleType> y;
    Array<SampleType> y2;
    Array<SampleType> y3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EpochFinder)
};
