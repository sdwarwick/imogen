/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: GrainExtractor
 */


#include "bv_Harmonizer/bv_Harmonizer.h"


template<typename SampleType>
class GrainExtractor

{
    
public:
    
    GrainExtractor();
    
    ~GrainExtractor();
    
    
    void prepare (const int maxBlocksize);
    
    void releaseResources();
    
    
    void getGrainOnsetIndices (juce::Array<int>& targetArray,
                               const juce::AudioBuffer<SampleType>& inputAudio,
                               const int period);
    
    
private:
    
    int lastBlocksize = 0;
    
    juce::Array<int> peakIndices; // used by all the kinds of peak picking algorithms to store their output for transformation to grains
    
    
    // functions used for finding of PSOLA peaks
    
    void findPsolaPeaks (juce::Array<int>& targetArray,
                         const SampleType* reading,
                         const int totalNumSamples,
                         const int period);
    
    void sortSampleIndicesForPeakSearching (juce::Array<int>& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (juce::Array<int>& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak,
                                  juce::Array<int>& searchingOrder);
    
    int chooseIdealPeakCandidate (const juce::Array<int>& candidates, const SampleType* reading,
                                  const int deltaTarget1, const int deltaTarget2);
    
    
    static constexpr int numPeaksToTest = 10; // the number of peak candidates that will be identified for each analysis window during the PSOLA peak picking process
    static constexpr int defaultFinalHandfulSize = 5; // final # of candidates with lowest delta values, which are evaluated for the strongest peak weighted using delta
    
    
    // functions used for simple zero-crossing mode
    
    void findZeroCrossings (juce::Array<int>& targetArray,
                            const SampleType* reading,
                            const int totalNumSamples,
                            const int period);
    
    void getZeroCrossingForPeriod (juce::Array<int>& targetArray,
                                   const SampleType* reading,
                                   const int startSample,
                                   const int endSample);
};
