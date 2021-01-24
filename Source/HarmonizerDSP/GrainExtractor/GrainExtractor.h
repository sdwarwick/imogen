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
    
    
    // sets the number of peak candidates to test in each analysis window for the PSOLA peak-picking algorithm. Values between 5 & 10 are recommended, and this value must not be lower than 1
    void setNumPeakCandidatesToTest (const int newNumCandidatesToTest);
    
    
private:
    
    
    bool isPeriodRepresentedByAPeak (const Array<int>& peaks, const int periodMinSample, const int periodMaxSample);
    
    
    Array<int> peakIndices; // used by all the kinds of peak picking algorithms to store their output for transformation to grains
    
    
    // functions used for finding of PSOLA peaks
    
    void prepareForPsola (const int maxBlocksize);
    
    void releasePsolaResources();
    
    void findPsolaPeaks (Array<int>& targetArray,
                         const AudioBuffer<SampleType>& inputAudio,
                         const int period);
    
    void sortSampleIndicesForPeakSearching (Array<int>& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak);
    
    // arrays used in finding of PSOLA peaks
    Array<int> peakCandidates;
    Array<float> candidateDeltas;
    Array<int> peakSearchingIndexOrder;
    
    // the number of peak candidates that will be tested for each analysis window during the PSOLA peak picking process
    int numPeaksToTest = 10;
    
};
