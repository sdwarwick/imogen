/*
    Part of module: bv_GeneralUtils
    Parent file: bv_GeneralUtils.h
 */


#include "bv_GeneralUtils/bv_GeneralUtils.h"


namespace bav

{
    

class FancyMidiBuffer  :    public juce::MidiBuffer
{
    
public:
    
    void pushEvents (const juce::MidiBuffer& source, const int numSamples, const int sourceStartSample = 0, const int sampleOffset = 0)
    {
        auto sourceStart = source.findNextSamplePosition (sourceStartSample);
        
        if (sourceStart == source.cend())
            return;
        
        const auto sourceEnd = source.findNextSamplePosition(sourceStartSample + numSamples);
        
        const int writingStartSample = (this->getNumEvents() == 0) ? 0 : this->getLastEventTime() + 1;
        
        std::for_each (sourceStart, sourceEnd,
                       [&] (const juce::MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           std::max(0, meta.samplePosition + sampleOffset + writingStartSample));
                       } );
    }
    
    
    void popEvents (juce::MidiBuffer& output, const int numSamples, const int outputSampleOffset = 0)
    {
        output.clear();
        
        auto readStart = this->findNextSamplePosition (0);
        
        if (readStart == this->cend())
            return;
        
        auto readEnd = this->findNextSamplePosition (numSamples);
        
        if (readStart == readEnd)
            return;
        
        std::for_each (readStart, readEnd,
                       [&] (const juce::MidiMessageMetadata& meta)
                       {
                           output.addEvent (meta.getMessage(),
                                            std::max(0, meta.samplePosition + outputSampleOffset));
                       } );
        
        this->clear();
        
        std::for_each (output.findNextSamplePosition(0), output.findNextSamplePosition(numSamples),
                       [&] (const juce::MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           std::max (0, meta.samplePosition - outputSampleOffset - numSamples));
                       } );
    }
    
    
private:
    
};


} // namespace
