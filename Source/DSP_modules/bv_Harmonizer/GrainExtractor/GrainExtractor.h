/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: GrainExtractor
 */


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{

    
using namespace juce;
    
    
template<typename SampleType>
class GrainExtractor

{
    
public:
    
    GrainExtractor();
    
    ~GrainExtractor();
    
    
    void prepare (const int maxBlocksize);
    
    void releaseResources();
    
    
    void getGrainOnsetIndices (Array<int>& targetArray,
                               const AudioBuffer<SampleType>& inputAudio,
                               const int period);
    
    
private:
    
    int lastBlocksize = 0;
    
    Array<int> peakIndices; // used by all the kinds of peak picking algorithms to store their output for transformation to grains
    
    
    // functions used for finding of PSOLA peaks
    
    void findPsolaPeaks (Array<int>& targetArray,
                         const SampleType* reading,
                         const int totalNumSamples,
                         const int period);
    
    void sortSampleIndicesForPeakSearching (Array<int>& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak,
                                  Array<int>& searchingOrder);
    
    int chooseIdealPeakCandidate (const Array<int>& candidates, const SampleType* reading,
                                  const int deltaTarget1, const int deltaTarget2);
    
    
    static constexpr int numPeaksToTest = 10; // the number of peak candidates that will be identified for each analysis window during the PSOLA peak picking process
    static constexpr int defaultFinalHandfulSize = 5; // final # of candidates with lowest delta values, which are evaluated for the strongest peak weighted using delta
    
    
    // functions used for simple zero-crossing mode
    
    void findZeroCrossings (Array<int>& targetArray,
                            const SampleType* reading,
                            const int totalNumSamples,
                            const int period);
    
    void getZeroCrossingForPeriod (Array<int>& targetArray,
                                   const SampleType* reading,
                                   const int startSample,
                                   const int endSample);
};


}; // namespace
