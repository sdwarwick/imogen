#pragma once

#include <JuceHeader.h>


template<typename SampleType>
class DelayBuffer
{
public:
    
    DelayBuffer (const int numChannels, const int maxDelay, const int blockLength)
    {
        lengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                             / blockLength );
        
        base.setSize (numChannels, 2 * lengthInSamples);
        
        writeIndex = lengthInSamples; // this is the MINIMUM write index!
        
        base.clear();
    };
    
    
    DelayBuffer()
    {
        lengthInSamples = 0;
        writeIndex = 0;
        base.setSize (0, 0);
    };
    
    
    ~DelayBuffer()
    { };
    
    
    void changeSize (const int newNumChannels, const int maxDelay, const int blockLength)
    {
        const int newLengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                                          / blockLength );
        
        if (lengthInSamples == newLengthInSamples && base.getNumChannels() == newNumChannels)
            return;
        
        lengthInSamples = newLengthInSamples;
        
        base.setSize (newNumChannels, 2 * lengthInSamples, true, true, true);
        
        if ((writeIndex < lengthInSamples) || (writeIndex >= 2 * lengthInSamples))
            writeIndex = lengthInSamples;
    };
    
    
    void writeSamples (const SampleType* inputSamples, const int numSamples, const int destChannel)
    {
        if (base.getNumSamples() == 0 || base.getNumChannels() == 0 || lengthInSamples == 0)
            return;
        
        jassert (numSamples < lengthInSamples);
        
        if ((writeIndex + numSamples) >= base.getNumSamples())
            writeIndex = lengthInSamples;
        
        base.copyFrom (destChannel, writeIndex,                   inputSamples, numSamples);
        base.copyFrom (destChannel, writeIndex - lengthInSamples, inputSamples, numSamples);
        
        writeIndex += numSamples;
    };
    
    
    void writeSamples (const AudioBuffer<SampleType>& inputBuffer, const int inputChannel, const int inputStartSample,
                       const int numSamples, const int destChannel)
    {
        writeSamples (inputBuffer.getReadPointer(inputChannel, inputStartSample),
                      numSamples, destChannel);
    };
    
    
    void getDelayedSamples (SampleType* outputSamples, const int delay, const int numSamples, const int readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        jassert (numSamples <= lengthInSamples);
        
        int readIndex = writeIndex - delay;
        
        jassert (readIndex >= 0);
        
        const SampleType* reading = base.getReadPointer(readingChannel);
        
        for (int n = 0; n < numSamples; ++n)
            outputSamples[n] = reading[readIndex++];
    };
    
    
    void getDelayedSamples (AudioBuffer<SampleType>& destBuffer, const int destChannel, const int destStartSample,
                            const int delay, const int numSamples, const int readingChannel) const
    {
        getDelayedSamples (destBuffer.getWritePointer(destChannel, destStartSample),
                           delay, numSamples, readingChannel);
    };
    
    
    SampleType* pointToDelayedSamples (const int delay, const int readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        return base.getReadPointer (readingChannel, writeIndex - delay);
    };
    
    
    int numStoredSamples() const noexcept
    {
        jassert (writeIndex >= lengthInSamples);
        return (writeIndex - lengthInSamples);
    };
    
    
    
private:
    
    AudioBuffer<SampleType> base;
    
    int lengthInSamples;
    
    int writeIndex;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayBuffer)
};
