#pragma once

#include <JuceHeader.h>


template<typename SampleType>
class DelayBuffer
{
public:
    
    DelayBuffer (const uint32_t numChannels, const uint32_t maxDelay, const uint32_t blockLength)
    {
        lengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                             / blockLength );
        
        base.setSize (numChannels, 2 * lengthInSamples);
        
        writeIndex = lengthInSamples; // this is the MINIMUM write index!
        
        base.clear();
    };
    
    ~DelayBuffer()
    { };
    
    
    void setSize (const uint32_t numChannels, const uint32_t maxDelay, const uint32_t blockLength)
    {
        lengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                           / blockLength );
        
        base.setSize (numChannels, 2 * lengthInSamples, true, true, true);
    };
    
    
    void writeSamples (const SampleType* inputSamples, const uint32_t numSamples, const uint32_t destChannel)
    {
        jassert (numSamples <= lengthInSamples);
        
        writeIndex += numSamples; // pre-increment write_index before writing new samples
        
        if (writeIndex >= 2 * lengthInSamples)
            writeIndex = lengthInSamples;  // back up write_index to beginning
        
        uint32_t imageIndex = writeIndex - lengthInSamples;
        
        jassert (imageIndex >= 0);
        
        base.copyFrom (destChannel, writeIndex, inputSamples, numSamples);
        base.copyFrom (destChannel, imageIndex, inputSamples, numSamples);
    };
    
    
    void writeSamples (const AudioBuffer<SampleType>& inputBuffer, const uint32_t inputChannel, const uint32_t inputStartSample,
                       const uint32_t numSamples, const uint32_t destChannel)
    {
        writeSamples (inputBuffer.getReadPointer(inputChannel, inputStartSample), numSamples, destChannel);
    };
    
    
    void getDelayedSamples (SampleType* outputSamples, const uint32_t delay, const uint32_t numSamples, const uint32_t readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        jassert (numSamples <= lengthInSamples);
        
        uint32_t readIndex = writeIndex - delay;
        
        jassert (readIndex >= 0);
        
        const SampleType* reading = base.getReadPointer(readingChannel);
        
        for (int n = 0; n < numSamples; ++n)
            outputSamples[n] = reading[readIndex++];
    };
    
    
    void getDelayedSamples (AudioBuffer<SampleType> destBuffer, const uint32_t destChannel, const uint32_t destStartSample,
                            const uint32_t delay, const uint32_t numSamples, const uint32_t readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        jassert (numSamples <= lengthInSamples);
        
        uint32_t readIndex = writeIndex - delay;
        
        jassert (readIndex >= 0);
        
        destBuffer.copyFrom (destChannel, destStartSample, base, readingChannel, readIndex, numSamples);
        
        // getDelayedSamples (destBuffer.getWritePointer(destChannel, destStartSample), delay, numSamples, readingChannel);
    };
    
    
    SampleType* pointToDelayedSamples (const uint32_t delay, const uint32_t readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        return base.getReadPointer (readingChannel, writeIndex - delay);
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
