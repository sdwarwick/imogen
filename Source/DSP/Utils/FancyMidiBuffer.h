/*
  ==============================================================================

    FancyMidiBuffer.h
    Created: 21 Jan 2021 1:47:16am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class FancyMidiBuffer  :    public juce::MidiBuffer
{
    
public:
    
    void appendToEnd (const MidiBuffer& sourceBuffer, const int numSamples, const int sourceStartSample = 0)
    {
        auto sourceStart = sourceBuffer.findNextSamplePosition (sourceStartSample);
        
        if (sourceStart == sourceBuffer.cend())
            return;
        
        const auto sourceEnd = sourceBuffer.findNextSamplePosition(sourceStartSample + numSamples - 1);
        
        if (sourceStart == sourceEnd)
            return;
        
        const int writingStartSample = (this->getNumEvents() > 0) ? this->getLastEventTime() + 1 : 0;
        
        std::for_each (sourceStart, sourceEnd,
                       [&] (const MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           meta.samplePosition + writingStartSample);
                       } );
    }
    
    
    void deleteEventsAndPushUpRest (const int numSamplesUsed)
    {
        if (this->findNextSamplePosition(numSamplesUsed - 1) == this->cend())
        {
            this->clear();
            return;
        }
        
        MidiBuffer temp (*this);
        
        this->clear();
        
        auto copyingStart = temp.findNextSamplePosition (numSamplesUsed - 1);
        
        std::for_each (copyingStart, temp.cend(),
                       [&] (const MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           std::max (0,
                                                    (meta.samplePosition - numSamplesUsed)) );
                       } );
    }
    
    
    void copyFromRangeOfOtherBuffer (const MidiBuffer& sourceBuffer,
                                     const int sourceStartSample,
                                     const int destStartSample,
                                     const int numSamples)
    {
        this->clear (destStartSample, numSamples);
        
        auto midiIterator = sourceBuffer.findNextSamplePosition(sourceStartSample);
        
        if (midiIterator == sourceBuffer.cend())
            return;
        
        const auto midiEnd = sourceBuffer.findNextSamplePosition (sourceStartSample + numSamples);
        
        if (midiIterator == midiEnd)
            return;
        
        const int sampleOffset = destStartSample - sourceStartSample;
        
        std::for_each (midiIterator, midiEnd,
                       [&] (const MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           std::max (0,
                                                     meta.samplePosition + sampleOffset));
                       } );
    }
    
    
private:
    
};
