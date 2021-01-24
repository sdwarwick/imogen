/*
  ==============================================================================

    GrainExtractor.h
    Created: 23 Jan 2021 2:38:27pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


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
    
    
    void getGrainOnsetIndicesForUnpitchedAudio (Array<int>& targetArray,
                                                const AudioBuffer<SampleType>& inputAudio,
                                                const int grainRate);
    
    
    // sets the number of peak candidates to test in each analysis window for the PSOLA peak-picking algorithm. Values between 5 & 10 are recommended, and this value must not be lower than 1
    void setNumPeakCandidatesToTest (const int newNumCandidatesToTest);
    
    
private:
    
    int lastBlocksize = 0;
    
    Array<int> peakIndices; // used by all the kinds of peak picking algorithms to store their output for transformation to grains
    
    
    // functions used for finding of PSOLA peaks
    
    void prepareForPsola (const int maxBlocksize);
    
    void releasePsolaResources();
    
    void findPsolaPeaks (Array<int>& targetArray,
                         const SampleType* reading,
                         const int totalNumSamples,
                         const int period);
    
    void sortSampleIndicesForPeakSearching (Array<int>& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak);
    
    
    // arrays used in finding of PSOLA peaks
    Array<int> peakCandidates;
    Array<float> candidateDeltas;
    Array<int> peakSearchingIndexOrder;
    
    int numPeaksToTest = 15; // the number of peak candidates that will be identified for each analysis window during the PSOLA peak picking process
    
    static constexpr int numDeltasToTest = 10; // final # of candidates for which delta values are calculated
    static constexpr int defaultFinalHandfulSize = 4; // final # of candidates with lowest delta values, which are evaluated for the strongest peak weighted using delta
    
    
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
