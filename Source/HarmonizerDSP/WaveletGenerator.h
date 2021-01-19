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
              const int sampleOffset)
    {
        const SampleType* r = input.getReadPointer(0);
              SampleType* w = output.getWritePointer(0);
        
        int readingSample = 0;
        int writingSample = sampleOffset;
        
        jassert (sampleOffset + input.getNumSamples() < output.getNumSamples());
        
        while (readingSample < input.getNumSamples())
        {
            w[writingSample] += r[readingSample];
            
            ++readingSample;
            ++writingSample;
        }
    };
    
    
private:
    
    int windowIndex;
    
};
