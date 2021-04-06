

namespace bav
{
    

template<typename SampleType>
class Harmonizer; // forward declaration...



/*
 HarmonizerVoice : represents a "voice" that the Harmonizer can use to generate one monophonic note. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
 */

template<typename SampleType>
class HarmonizerVoice  :    public dsp::SynthVoiceBase<SampleType>
{
    
    using AudioBuffer = juce::AudioBuffer<SampleType>;
    using Base = dsp::SynthVoiceBase<SampleType>;
    using Synthesis_Grain = SynthesisGrain<SampleType>;
    
    
public:
    
    HarmonizerVoice (Harmonizer<SampleType>* h);
    
    
private:
    friend class Harmonizer<SampleType>;
    
    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate) override;
    
    Harmonizer<SampleType>* parent;
    
    void prepared (const int blocksize) override;
    
    void released() override;
    
    void noteCleared() override;
    
    PsolaShifter<SampleType> shifter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


}  // namespace
