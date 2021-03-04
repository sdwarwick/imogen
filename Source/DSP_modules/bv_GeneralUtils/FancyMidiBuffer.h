/*
    Part of module: bv_GeneralUtils
    Parent file: bv_GeneralUtils.h
 */


namespace bav

{
    

class FancyMidiBuffer  :    public juce::MidiBuffer
{
    
public:
    
    void pushEvents (const juce::MidiBuffer& source, const int numSamples)
    {
        auto sourceStart = source.findNextSamplePosition (0);
        
        if (sourceStart == source.cend())
            return;
        
        const auto sourceEnd = source.findNextSamplePosition (numSamples);
        
        const int writingStartSample = (this->getNumEvents() == 0) ? 0 : this->getLastEventTime() + 1;
        
        std::for_each (sourceStart, sourceEnd,
                       [&] (const juce::MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(), meta.samplePosition + writingStartSample);
                       } );
    }
    
    
    void popEvents (juce::MidiBuffer& output, const int numSamples)
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
                           output.addEvent (meta.getMessage(), meta.samplePosition);
                       } );
        
        this->clear();
        
        std::for_each (output.findNextSamplePosition(0), output.findNextSamplePosition(numSamples),
                       [&] (const juce::MidiMessageMetadata& meta)
                       {
                           this->addEvent (meta.getMessage(),
                                           std::max (0, meta.samplePosition - numSamples));
                       } );
    }
    
    
private:
    
};


} // namespace
