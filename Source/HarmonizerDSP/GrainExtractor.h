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
    
    
private:
    
    void prepareForPsola (const int maxBlocksize);
    
    void releasePsolaResources();
    
    void findPsolaPeaks (Array<int>& targetArray,
                         const AudioBuffer<SampleType>& inputAudio,
                         const int period);
    
    void sortSampleIndicesForPeakSearching (Array<int>& output, const int startSample, const int endSample, const int predictedPeak);
    
    void getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                  const int startSample, const int endSample, const int predictedPeak);
    
    Array<int> peakIndices;
    Array<int> peakCandidates;
    Array<float> candidateDeltas;
    Array<int> peakSearchingIndexOrder;
    
};
