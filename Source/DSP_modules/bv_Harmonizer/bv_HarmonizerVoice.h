

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
        
        void storeNewGrain (const int startSample, const int endSample, const int synthesisMarker);
        
        bool isActive() { return currentlyActive; }
        
        /* returns true if this sample advancement in this grain should trigger a new grain to begin */
        void getNextSampleIndex (int& origSampleIndex, SampleType& windowValue, int& samplesLeftInGrain);
        
        void skipSamples (int numSamples);
        
        void clear();
        
        int getLastStartIndex() const noexcept { return origStartSample; }
        
        int getLastEndIndex() const noexcept { return origEndSample; }
        
    private:
        /* computes a single Hann window value based on a given window size and place in the window */
        inline SampleType getWindowValue (int windowSize, int index);
        
        bool currentlyActive = false;
        
        int origStartSample = 0; // the start sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        int origEndSample   = 0; // the end sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        
        int readingIndex = 0;
        
        int zeroesLeft = 0;
        
        int grainSize = 0;
        
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
    
    void prepared (const int blocksize) override;
    
    void released() override;
    
    void noteCleared() override;
    
    inline SampleType getNextSample (const SampleType* inputSamples, const int grainSize, const int halfGrainSize,
                                     const int synthesisHopSize);
    
    inline Grain* getAvailableGrain()
    {
        for (auto* grain : grains)
            if (! grain->isActive())
                return grain;
        
        return nullptr;
    }
    
    inline bool anyGrainsAreActive()
    {
        for (auto* grain : grains)
            if (grain->isActive())
                return true;
        
        return false;
    }
    
    inline void startNewGrain (const int grainSize, const int synthesisHopSize);
    
    int nextFramesPeriod = 0;
    
    int lastUsedGrainInArray = -1;
    
    juce::OwnedArray<Grain> grains;
    
    int nextSynthesisIndex = 0;
    
    dsp::osc::Sine<SampleType> sanityTest;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


}  // namespace
