/*
  ==============================================================================

    PitchDetector.h
    Created: 18 Jan 2021 11:31:33am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


template<typename SampleType>
class PitchDetector
{
public:
    
    PitchDetector(const int minDetectableHz, const int maxDetectableHz, const double initSamplerate);
    
    ~PitchDetector();
    
    int getMinHz() const noexcept { return minHz; }
    
    int getMaxHz() const noexcept { return maxHz; }
    
    int getMaxPeriod() const noexcept { return maxPeriod; }
    
    void setHzRange (const int newMinHz, const int newMaxHz, const bool allowRecalc = false);
    
    SampleType getCurrentConfidenceThresh() const noexcept { return confidenceThresh; }
    
    void setConfidenceThresh (const SampleType newThresh) { confidenceThresh = newThresh; }
    
    double getSamplerate() const noexcept { return samplerate; }
    
    void setSamplerate (const double newSamplerate, const bool recalcHzRange = true);
    
    float detectPitch (const AudioBuffer<SampleType>& inputAudio);
    
    
    
private:
    
    int minHz, maxHz;
    int minPeriod, maxPeriod;
    
    SampleType lastEstimatedPeriod;
    bool lastFrameWasPitched = false;
    
    double samplerate;
    
    SampleType confidenceThresh;
    
    AudioBuffer<SampleType> asdfBuffer; // calculated ASDF values will be placed in this buffer
    
    
    float chooseIdealPeriodCandidate (const SampleType* asdfData, const int asdfDataSize, const int minIndex);
    
    int samplesToFirstZeroCrossing (const SampleType* inputAudio, const int numSamples);
    
    int indexOfMinElement (const SampleType* data, const int dataSize);
    
    float foundThePeriod (const SampleType* asdfData, const int minIndex, const int asdfDataSize);
    
    SampleType quadraticPeakPosition (const SampleType* data, const int pos, const int dataSize) noexcept;
    
    
    static constexpr int periodCandidatesToTest = 15;
    
    void getNextBestPeriodCandidate (Array<int>& candidates, const SampleType* asdfData, const int dataSize);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchDetector)
};
