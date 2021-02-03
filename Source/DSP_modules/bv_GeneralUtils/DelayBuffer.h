/*
    Part of module: bv_GeneralUtils
    Parent file: bv_GeneralUtils.h
*/


#include "bv_GeneralUtils/bv_GeneralUtils.h"


namespace bav

{
    

template<typename SampleType>
class DelayBuffer
{
public:
    
    DelayBuffer (const int numChannels, const int maxDelay, const int blockLength)
    {
        lengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                             / blockLength );
        
        base.setSize (numChannels, lengthInSamples);
        base.clear();
        
        writeIndex = 0;
    }
    
    DelayBuffer()
    {
        lengthInSamples = 0;
        writeIndex = 0;
        base.setSize (0, 0);
        base.clear();
    }
    
    ~DelayBuffer()
    { }
    
    
    void changeSize (const int newNumChannels, const int maxDelay, const int blockLength)
    {
        const int newLengthInSamples = blockLength  *  ( (maxDelay + (2 * blockLength) - 1)
                                                          / blockLength );
        
        if (lengthInSamples == newLengthInSamples && base.getNumChannels() == newNumChannels)
            return;
        
        lengthInSamples = newLengthInSamples;
        
        base.setSize (newNumChannels, lengthInSamples, true, true, true);
        
        if (writeIndex < 0)
            writeIndex = lengthInSamples + writeIndex;
        else if (writeIndex >= lengthInSamples)
            writeIndex -= lengthInSamples;
    }
    
    
    void writeSamples (const SampleType* inputSamples, const int numSamples, const int destChannel)
    {
        if (base.getNumSamples() == 0 || base.getNumChannels() == 0 || lengthInSamples == 0)
            return;
        
        jassert (numSamples <= lengthInSamples);
        
        SampleType* writing = base.getWritePointer(destChannel);
        
        int index = writeIndex;
        
        for (int s = 0;
             s < numSamples;
             ++s, ++index)
        {
            index %= lengthInSamples;
            writing[index] = inputSamples[s];
        }
        
        writeIndex = index;
    }
    
    
    void writeSamples (const juce::AudioBuffer<SampleType>& inputBuffer, const int inputChannel, const int inputStartSample,
                       const int numSamples, const int destChannel)
    {
        writeSamples (inputBuffer.getReadPointer(inputChannel, inputStartSample),
                      numSamples, destChannel);
    }
    
    
    void getDelayedSamples (SampleType* outputSamples, const int delay, const int numSamples, const int readingChannel) const
    {
        jassert (delay <= lengthInSamples);
        jassert (numSamples <= lengthInSamples);
        
        const int initRead = writeIndex - delay;
        int readIndex = initRead < 0 ? lengthInSamples + initRead : initRead;
        
        const SampleType* reading = base.getReadPointer(readingChannel);
        
        for (int n = 0;
             n < numSamples;
             ++n, ++readIndex)
        {
            readIndex %= lengthInSamples;
            outputSamples[n] = reading[readIndex];
        }
    }
    
    
    void getDelayedSamples (juce::AudioBuffer<SampleType>& destBuffer, const int destChannel, const int destStartSample,
                            const int delay, const int numSamples, const int readingChannel) const
    {
        getDelayedSamples (destBuffer.getWritePointer(destChannel, destStartSample),
                           delay, numSamples, readingChannel);
    }
    
    
    SampleType* pointToDelayedSamples (const int delay, const int readingChannel) const
    {
        const int initRead = writeIndex - delay;
        const int readIndex = initRead < 0 ? lengthInSamples + initRead : initRead;
        
        return base.getReadPointer (readingChannel, readIndex);
    }
    
    
    int numStoredSamples() const noexcept
    {
        if (writeIndex - lengthInSamples <= 0)
            return 0;
            
        return writeIndex - lengthInSamples;
    }
    
    
    
private:
    
    juce::AudioBuffer<SampleType> base;
    
    int lengthInSamples;
    
    int writeIndex;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayBuffer)
};


}; // namespace
