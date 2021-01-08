/*
  ==============================================================================

    Freezer.h
    Created: 7 Jan 2021 11:18:38pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"


template<typename SampleType>
class Freezer
{
public:
    
    Freezer();
    ~Freezer();
    
    
    void prepare (const double sampleRate, const int samplesPerBlock, const int numChannels);
    
    void releaseResources();
    
    
    /*
        To achieve an easy interface with the rest of the signal flow, this function should be able to always be called, and should handle the appropriate freezeNewAudio() or renderFrozennAudio() calls internally.
        If the parameter "isFrozen" is false, this function should be a simple pipe copying the input to the output.
     */
    void process (const AudioBuffer<SampleType>& inBuffer, AudioBuffer<SampleType>& outBuffer, const bool isFrozen);
    
    
    
private:
    
    AudioBuffer<SampleType> freezeBuffer;
    
    bool wasFrozen;
    bool isCleared;
    
    void freezeNewAudio (const AudioBuffer<SampleType>& input);
    
    void clearFrozenAudio();
    
    void renderFrozenAudio (AudioBuffer<SampleType>& outBuffer);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Freezer)
};
