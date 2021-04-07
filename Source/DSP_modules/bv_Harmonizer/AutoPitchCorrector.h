

namespace bav
{
    
    template<typename SampleType>
    class AutoPitch
    {
        using AudioBuffer = juce::AudioBuffer<SampleType>;
        using Array = juce::Array<int>;
        
    public:
        AutoPitch(PsolaAnalyzer<SampleType>* a): analyzer(a), shifter(a) { jassert (analyzer != nullptr); }
        
        void releaseResources()
        {
            legalNotes.clear();
            shifter.releaseResources();
        }
        
        void initialize()
        {
            shifter.prepare();
            legalNotes.ensureStorageAllocated (12);
            makeAllNotesLegal();
        }
        
        void reset()
        {
            shifter.reset();
        }
        
        void makeAllNotesLegal()
        {
            legalNotes.clearQuick();
            
            for (int i = 0; i < 12; ++i)
                legalNotes.add (i);
        }
        
        void setLegalNotes (const Array& newLegalNotes)
        {
            legalNotes.clearQuick();
            
            for (int note : newLegalNotes)
            {
                note %= 12;
                legalNotes.add (note);
            }
        }
        
        void getSamples (SampleType* outputSamples, const int numSamples,
                         const float currentInputFreq, const double currentSamplerate,
                         float* correctionAmount)  // correction amount in Hz
        {
            auto currentMidiPitch = math::freqToMidi (currentInputFreq);
            
            int octaves = 0;
            
            while (currentMidiPitch >= 12)
            {
                currentMidiPitch -= 12;
                ++octaves;
            }
            
            jassert (! legalNotes.isEmpty());
            
            int closestLegalNote = legalNotes.getUnchecked(0);
            auto distance = abs(currentMidiPitch - closestLegalNote);
            
            for (auto note : legalNotes)
            {
                const auto newDist = abs(currentMidiPitch - note);
                
                if (newDist < distance)
                {
                    closestLegalNote = note;
                    distance = newDist;
                }
            }
            
            while (octaves > 0)
            {
                closestLegalNote += 12;
                --octaves;
            }
            
            const auto desiredFrequency = math::midiToFreq (closestLegalNote);
            
            if (correctionAmount != nullptr)
                *correctionAmount = desiredFrequency - currentInputFreq;
            
            shifter.getSamples (outputSamples, numSamples,
                                juce::roundToInt (currentSamplerate / desiredFrequency));  // new desired period
        }
        
        
    private:
        PsolaAnalyzer<SampleType>* analyzer;
        PsolaShifter<SampleType> shifter;
        Array legalNotes;
    };
    
    
    template class AutoPitch<float>;
    template class AutoPitch<double>;
    
} // namespace
