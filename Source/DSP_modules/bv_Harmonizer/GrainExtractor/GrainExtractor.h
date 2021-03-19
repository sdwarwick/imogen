/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: GrainExtractor
 */

#pragma once


namespace bav
{

    
template<typename SampleType>
class GrainExtractor
{
    
    using IArray = juce::Array<int>;
    using FArray = juce::Array<float>;
    
    
public:
    
    GrainExtractor();
    
    ~GrainExtractor();
    
    
    void prepare (const int maxBlocksize);
    
    void releaseResources();
    
    
    void getGrainOnsetIndices (IArray& targetArray,
                               const juce::AudioBuffer<SampleType>& inputAudio,
                               const int period);
    
    
private:
    
    IArray peakIndices; // used by all the kinds of peak picking algorithms to store their output for transformation to grains
    
    IArray peakCandidates;
    IArray peakSearchingOrder;
    
    FArray candidateDeltas;
    IArray   finalHandful;
    FArray finalHandfulDeltas;
    
    // functions used for finding of PSOLA peaks
    
    void findPsolaPeaks (IArray& targetArray,
                         const SampleType* reading,
                         const int totalNumSamples,
                         const int period);
    
    int findNextPeak (const int frameStart, const int frameEnd, int predictedPeak, const SampleType* reading, const IArray& targetArray,
                      const int period, const int grainSize);
    
    void sortSampleIndicesForPeakSearching (IArray& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (IArray& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak,
                                  const IArray& searchingOrder);
    
    int chooseIdealPeakCandidate (const IArray& candidates, const SampleType* reading,
                                  const int deltaTarget1, const int deltaTarget2);
    
    int choosePeakWithGreatestPower (const IArray& candidates, const SampleType* reading);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainExtractor)
};


} // namespace
