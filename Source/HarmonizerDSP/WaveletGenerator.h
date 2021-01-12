/*
  ==============================================================================

    WaveletGenerator.h
    Created: 11 Jan 2021 7:10:17pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


template<typename SampleType>
class WaveletGenerator
{
public:
    
    WaveletGenerator() { };
    
    ~WaveletGenerator() { };
    
    
    void ola (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
              const float playbackSpeed, const int sampleOffset,
              const AudioBuffer<SampleType>& windowToUse)
    {
        const int numSamples = floor (input.getNumSamples() * playbackSpeed);
        
        SampleType* w = output.getWritePointer(0);
        
        for (int n = sampleOffset; n < sampleOffset + numSamples; ++n)
        {
         //   w[n] += (this wavelet)
        }
    };
    
    
private:
    
};
