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
    
    findPsolaPeaks (peakIndices, reading, totalNumSamples, period);
    
    // findZeroCrossings (peakIndices, reading, totalNumSamples, period);
    
    jassert (! peakIndices.isEmpty());
    
    
    // create array of grain start indices, such that grains are 2 pitch periods long, CENTERED on points of synchronicity previously identified
    
    for (int p = 0; p < peakIndices.size(); ++p)
    {
        const int peakIndex = peakIndices.getUnchecked(p);
        
        const int grainStart = peakIndex - period; // offset the peak index by the period so that the peak index will be in the center of the grain (if grain is 2 periods long)

        if (grainStart < 0)
        {
            if (p < peakIndices.size() - 2)
                continue;
            
            // edge case for really large periods
            const int secondaryGrainStart = peakIndex - roundToInt(period / 2.0f);
            
            if (secondaryGrainStart < 0)
                targetArray.add (peakIndex);
            else
                targetArray.add (secondaryGrainStart);
        }

        targetArray.add (grainStart);
    }
    
    jassert (! targetArray.isEmpty());
};



template class GrainExtractor<float>;
template class GrainExtractor<double>;
