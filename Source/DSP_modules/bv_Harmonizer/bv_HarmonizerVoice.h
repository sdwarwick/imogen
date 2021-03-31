

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
    using FVO = juce::FloatVectorOperations;
    using Base = dsp::SynthVoiceBase<SampleType>;
    
    /*
     This class represents one analysis grain that is being resynthesized by the voice.
     The voice always has two active grains: one fading out, and one fading in.
     */
    class Grain
    {
    public:
        Grain() { }
        
        ~Grain() { }
        
        void storeNewGrain (const int startSample, const int endSample);
        
        bool isActive() { return currentlyActive; }
        
        bool getNextSampleIndex (int& origSampleIndex, int& windowIndex);
        
        void skipSamples (const int numSamples);
        
        void clear();
        
        int getLastStartIndex() const noexcept { return origStartSample; }
        
        int getLastEndIndex() const noexcept { return origEndSample; }
        
    private:
        bool currentlyActive = false;
        
        int origStartSample = 0; // the start sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        int origEndSample   = 0; // the end sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        int halfwayIndex    = 0;
        
        int readingIndex = 0;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grain)
    };
    
    /*
    */
    
public:
    
    HarmonizerVoice (Harmonizer<SampleType>* h);
    
    void dataAnalyzed (const int period);
    
    
private:
    friend class Harmonizer<SampleType>;
    
    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int origStartSample) override;
    
    void bypassedBlockRecieved (int numSamples) override;
    
    Harmonizer<SampleType>* parent;
    
    void prepare (const int blocksize) override;
    
    void released() override;
    
    void noteCleared() override;
    
    inline SampleType getNextSample (const SampleType* inputSamples,
                                     const SampleType* window,
                                     const int grainSize, const int origPeriodTimesScaleFactor);
    
    inline Grain* getAvailableGrain()
    {
        for (auto* grain : grains)
            if (! grain->isActive())
                return grain;
        
        return nullptr;
    }
    
    int nextFramesPeriod = 0;
    
    int lastUsedGrainInArray = -1;
    
    juce::OwnedArray<Grain> grains;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


}  // namespace
