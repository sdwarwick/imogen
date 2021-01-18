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
    
    PitchDetector(const int minHz, const int maxHz, const double samplerate);
    
    ~PitchDetector();
    
    int getMinHz() const noexcept { return minHz; };
    
    int getMaxHz() const noexcept { return maxHz; };
    
    void setHzRange (const int newMinHz, const int newMaxHz, const bool allowRecalc = false);
    
    double getSamplerate() const noexcept { return samplerate; };
    
    void setSamplerate (const double newSamplerate, const bool recalcHzRange = true);
    
    float detectPitch (const AudioBuffer<SampleType>& inputAudio);
    
    
private:
    
    int minHz, maxHz;
    int minPeriod, maxPeriod;
    
    double samplerate;
    
    AudioBuffer<SampleType> asdfBuffer; // calculated ASDF values will be placed in this buffer
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchDetector)
};
