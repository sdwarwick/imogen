/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_PitchDetector
 vendor:             Ben Vining
 version:            0.0.1
 name:               Pitch detector
 description:        ASDF-based pitch detector
 dependencies:       juce_audio_utils
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include <juce_audio_utils/juce_audio_utils.h>


namespace bav

{
    
    
using namespace juce;


template<typename SampleType>
class PitchDetector
{
public:
    
    PitchDetector (const int minDetectableHz, const int maxDetectableHz, const double initSamplerate);
    
    ~PitchDetector();
    
    void setHzRange (const int newMinHz, const int newMaxHz, const bool allowRecalc = false);
    
    void setConfidenceThresh (const SampleType newUpperThresh,
                              const SampleType newLowerThresh)
            { upperConfidenceThresh = newUpperThresh;
              lowerConfidenceThresh = newLowerThresh; }
    
    void setSamplerate (const double newSamplerate, const bool recalcHzRange = true);
    
    float detectPitch (const AudioBuffer<SampleType>& inputAudio);
    
    
    
private:
    
    int minHz, maxHz;
    int minPeriod, maxPeriod;
    
    SampleType lastEstimatedPeriod;
    bool lastFrameWasPitched = false;
    
    double samplerate;
    
    SampleType upperConfidenceThresh;  // if the lowest asdf data value is above this thresh, the frame of audio is determined to be unpitched
    SampleType lowerConfidenceThresh;  // if the lowest asdf data value is below this thresh, that k is returned as the period, without the more sophisticated period candidate selection process
    
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


}; // namespace
