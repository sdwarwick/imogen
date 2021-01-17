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
    
    WaveletGenerator(): windowIndex(0)
    { };
    
    ~WaveletGenerator() { };
    
    
    void ola (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
              const float playbackSpeed, const int sampleOffset,
              const AudioBuffer<SampleType>& windowToUse)
    {
        const int numSamples = floor (input.getNumSamples() * playbackSpeed);
        
        const SampleType* r = input.getReadPointer(0);
              SampleType* w = output.getWritePointer(0);
        const SampleType* win = windowToUse.getReadPointer(0);
        
        int readingSample = 0;
        int writingSample = sampleOffset;
        
        while (readingSample <= numSamples)
        {
            w[writingSample] += (r[readingSample] * win[windowIndex]);
            
            ++readingSample;
            ++writingSample;
            ++windowIndex;
            
            if (windowIndex >= windowToUse.getNumSamples())
                windowIndex = 0;
        }
    };
    
    
private:
    
    int windowIndex;
    
};
