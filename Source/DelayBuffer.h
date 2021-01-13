/*
  ==============================================================================

    DelayBuffer.h
    Created: 11 Jan 2021 11:37:45pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


template<typename SampleType>
class DelayBuffer
{
public:
    
    DelayBuffer (const uint32_t maxDelay, const uint32_t blockLength)
    {
        lengthInSamples = blockLength * ( (maxDelay + (2 * blockLength) - 1)
                                           / blockLength );
        
        base.setSize (1, 2 * lengthInSamples);
        
        writeIndex = lengthInSamples; // this is the MINIMUM write index!
        
        base.clear();
    };
    
    ~DelayBuffer()
    { };
    
    
    void writeSamples (const SampleType* inputSamples, const uint32_t numSamples)
    {
        jassert (numSamples <= lengthInSamples);
        
        writeIndex += numSamples; // pre-increment write_index before writing new samples
        
        if (writeIndex >= 2 * lengthInSamples)
            writeIndex = lengthInSamples;  // back up write_index to beginning
        
        uint32_t imageIndex = writeIndex - lengthInSamples;
        
        jassert (imageIndex >= 0);
        
        base.copyFrom (0, writeIndex, inputSamples, numSamples);
        base.copyFrom (0, imageIndex, inputSamples, numSamples);
    };
    
    
    void getDelayedSamples (SampleType* outputSamples, const uint32_t delay, const uint32_t numSamples) const
    {
        jassert (delay <= lengthInSamples);
        jassert (numSamples <= lengthInSamples);
        
        uint32_t readIndex = writeIndex - delay;
        
        jassert (readIndex >= 0);
        
        const SampleType* reading = base.getReadPointer(0);
        
        for (int n = 0; n < numSamples; ++n)
            outputSamples[n] = reading[readIndex++];
    };
    
    
    SampleType* pointToDelayedSamples (const uint32_t delay) const
    {
        jassert (delay <= lengthInSamples);
        return base.getReadPointer (0, writeIndex - delay);
    };
    
    
    uint32_t numStoredSamples() const noexcept
    {
        jassert (writeIndex >= lengthInSamples);
        return (writeIndex - lengthInSamples);
    };
    
    
    
private:
    
    AudioBuffer<SampleType> base;
    
    uint32_t lengthInSamples;
    
    uint32_t writeIndex;
};
