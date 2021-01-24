/*
  ==============================================================================

    GrainExtractor.cpp
    Created: 23 Jan 2021 2:38:27pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "GrainExtractor/GrainExtractor.h"


template<typename SampleType>
GrainExtractor<SampleType>::GrainExtractor()
{ };


template<typename SampleType>
GrainExtractor<SampleType>::~GrainExtractor()
{ };


template<typename SampleType>
void GrainExtractor<SampleType>::prepare (const int maxBlocksize)
{
    // maxBlocksize = max period of input audio
    
    peakIndices.ensureStorageAllocated (maxBlocksize);
    
    prepareForPsola (maxBlocksize);
};


template<typename SampleType>
void GrainExtractor<SampleType>::releaseResources()
{
    peakIndices.clear();
    
    releasePsolaResources();
};


template<typename SampleType>
void GrainExtractor<SampleType>::getGrainOnsetIndicesForUnpitchedAudio (Array<int>& targetArray,
                                                                        const AudioBuffer<SampleType>& inputAudio,
                                                                        const int grainRate)
{
    targetArray.clearQuick();
    
    targetArray.add (0);
    
    for (int i = grainRate;
         i < inputAudio.getNumSamples();
         i += grainRate)
        targetArray.add (i);
};



template<typename SampleType>
void GrainExtractor<SampleType>::getGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    targetArray.clearQuick();
    
    const int totalNumSamples = inputAudio.getNumSamples();
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    // identify  peak indices for each pitch period & places them in the peakIndices array
    
    // findPsolaPeaks (peakIndices, reading, totalNumSamples, period);
    
    findZeroCrossings (peakIndices, reading, totalNumSamples, period);
    
    jassert (! peakIndices.isEmpty());
    
    
    // PART TWO - create array of grain onset indices, such that grains are 2 pitch periods long, CENTERED on points of synchronicity previously identified
    
    for (int p = 0; p < peakIndices.size(); ++p)
    {
        // offset the peak index by the period so that the peak index will be in the center of the grain (if grain is 2 periods long)
        const int grainStart = peakIndices.getUnchecked(p) - period;
        
        if (grainStart < 0)
            continue;
        
        targetArray.add (grainStart);
    }
    
    // fill in hypothetial missed grains @ start of audio
    int first = targetArray.getUnchecked(0);
    first -= period;
    
    while (first >= 0)
    {
        targetArray.insert (0, first);
        first -= period;
    }
    
    // fill in hypothetical missed grains @ end of audio
    int last = targetArray.getLast();
    last += period;
    
    while (last < inputAudio.getNumSamples())
    {
        targetArray.add (last);
        last += period;
    }
    
    
    jassert (! targetArray.isEmpty());
    
};



template class GrainExtractor<float>;
template class GrainExtractor<double>;
