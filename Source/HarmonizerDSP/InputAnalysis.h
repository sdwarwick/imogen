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
    
    SampleType getPitch(const AudioBuffer<SampleType>& inputAudio, const double samplerate);
    
    void setTolerence(const float newTolerence) noexcept { tolerence = newTolerence; }
    
    void setHzLimits(const float newMin, const float newMax) noexcept { minHz = newMin; maxHz = newMax; }
    
    void clearBuffer() { yinBuffer.clear(); };
    
    // DANGER!!! FOR NON REALTIME USE ONLY!!!
    void increaseBuffersize(const int newMaxBlocksize) { yinBuffer.setSize(1, newMaxBlocksize, true, true, true); };
    
private:
    AudioBuffer<SampleType> yinBuffer;
    SampleType prevDetectedPitch;
   
    float tolerence;
    float minHz, maxHz;
    
    SampleType simpleYin(const AudioBuffer<SampleType>& inputAudio) noexcept;
    
    unsigned int minElement(const SampleType* data, const int dataSize) noexcept;
    
    SampleType quadraticPeakPosition (const SampleType* data, unsigned int pos, const int dataSize) noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchTracker)
};



template<typename SampleType>
class EpochFinder
{
public:
    EpochFinder();
    ~EpochFinder();
    
    void extractEpochSampleIndices(const AudioBuffer<SampleType>& inputAudio, const double samplerate, Array<int>& outputArray);
    
    int averageDistanceBetweenEpochs(const Array<int>& epochIndices);
    
    // DANGER!!! FOR NON REAL TIME USE ONLY!!!
    void increaseBufferSizes(const int newMaxBlocksize);
    
private:
    CriticalSection lock;
    
    Array<SampleType> y;
    Array<SampleType> y2;
    Array<SampleType> y3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EpochFinder)
};
