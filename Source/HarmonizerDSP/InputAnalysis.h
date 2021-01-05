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


class PitchTracker
{
public:
    PitchTracker();
    ~PitchTracker();
    
    //	float findPitch(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate);
    
    float getPitch(const AudioBuffer<float>& inputAudio, const double samplerate);
    
    void setTolerence(const float newTolerence) noexcept { tolerence = newTolerence; }
    
    void setHzLimits(const float newMin, const float newMax) noexcept { minHz = newMin; maxHz = newMax; }
    
    void clearBuffer() { yinBuffer.clear(); };
    
    // DANGER!!! FOR NON REALTIME USE ONLY!!!
    void increaseBuffersize(const int newMaxBlocksize) { yinBuffer.setSize(1, newMaxBlocksize, true, true, true); };
    
private:
    AudioBuffer<float> yinBuffer;
    float prevDetectedPitch;
    //	Array<float> powerTerms;
    //	int yinBufferSize;
    
    float tolerence;
    float minHz, maxHz;
    
    float simpleYin(const AudioBuffer<float>& inputAudio) noexcept;
    
    unsigned int minElement(const float* data, const int dataSize) noexcept;
    
    float quadraticPeakPosition (const float* data, unsigned int pos, const int dataSize) noexcept;
    
    //	void difference(AudioBuffer<float>& inputBuffer, const int inputChan, const int inputBufferLength);
    //
    //	void fastDifference(AudioBuffer<float>& inputAudio, const int inputChan);
    //
    //	void cumulativeMeanNormalizedDifference() const;
    //
    //	int absoluteThreshold();
    //
    //	float parabolicInterpolation(int tauEstimate) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchTracker)
};




class EpochFinder
{
public:
    EpochFinder();
    ~EpochFinder();
    
    void extractEpochSampleIndices(const AudioBuffer<float>& inputAudio, const double samplerate, Array<int>& outputArray);
    
    int averageDistanceBetweenEpochs(const Array<int>& epochIndices);
    
    // DANGER!!! FOR NON REAL TIME USE ONLY!!!
    void increaseBufferSizes(const int newMaxBlocksize);
    
private:
    CriticalSection lock;
    
    Array<float> y;
    Array<float> y2;
    Array<float> y3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EpochFinder)
};
