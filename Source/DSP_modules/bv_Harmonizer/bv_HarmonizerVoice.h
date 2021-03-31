

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
        
        void prepare (int blocksize) { samples.setSize(1, blocksize); }
        
        void storeNewGrain (const SampleType* inputSamples, const int startSample, const int numSamples, const SampleType* window,
                            const int synthesisMarker);
        
        SampleType getNextSample();
        
        void skipSamples (const int numSamples);
        
        void clear();
        
        int getLastStartIndex() const noexcept { return inStartSample; }
        
        int getLastEndIndex() const noexcept { return inEndSample; }
        
        int getLastSynthesisMark() const noexcept { return synthesisMark; }
        
    private:
        int inStartSample = 0; // the start sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        int inEndSample   = 0; // the end sample for this grain in the original input audo fed to the parent Harmonizer's analyzeInput().
        
        int zeroesLeft = 0; // the number of zeroes the grain should output before outputting its next stored samples
        
        int nextSample = 0; // the next sample index to be read from the buffer
        
        int synthesisMark = 0; /// the scaled placement of the grain in the output signal
        
        int size = 0;
        
        AudioBuffer samples; // this buffer stores the input samples with the window applied
        
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
    
    //    void sola (const SampleType* input, const int totalNumInputSamples,
    //               const int origPeriod, const int newPeriod, const juce::Array<int>& indicesOfGrainOnsets,
    //               const SampleType* window);
    //
    //    void olaFrame (const SampleType* inputAudio, const int frameStartSample, const int frameEndSample, const int frameSize,
    //                   const SampleType* window, const int newPeriod);
    
    void moveUpSamples (const int numSamplesUsed);
    
    
    inline SampleType getNextSample (const SampleType* inputSamples,
                                     const SampleType* window,
                                     const int grainSize, const int origPeriodTimesScaleFactor);
    
    inline void storeNewGrain (Grain& grain, const SampleType* inputSamples, const int grainSize, const SampleType* window,
                               const int origPeriodTimesScaleFactor);
    
    AudioBuffer synthesisBuffer; // mono buffer that this voice's synthesized samples are written to
    AudioBuffer copyingBuffer;
    
    int synthesisIndex;
    
    int nextFramesPeriod = 0;
    
    Grain grain1, grain2;
    
    int lastUsedGrainInArray = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


}  // namespace
