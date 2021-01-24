/*
  ==============================================================================

    GrainExtractor.cpp
    Created: 23 Jan 2021 2:38:27pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "GrainExtractor.h"


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
void GrainExtractor<SampleType>::getGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    targetArray.clearQuick();
    
    // PART ONE - find sample indices of points of maximum energy for every pitch period
    
    findPsolaPeaks (peakIndices, inputAudio, period);
    
    // eliminate unneeded peaks too close together ??
    
    const int totalNumSamples = inputAudio.getNumSamples();
    
    // if any periods are missing a peak, fill them in
    for (int p = 0;
         p < totalNumSamples;
         p += period)
    {
        if (! isPeriodRepresentedByAPeak (peakIndices, p, std::min(p + period, totalNumSamples)))
        {
            // fill in missing peak for this period, attempting to evenly space it with the surrounding peaks
        }
    }
    
    
    
    
    // PART TWO - create array of grain onset indices, such that grains are 2 pitch periods long, centred on points of maximum energy, w/ approx 50% overlap
    
    
    // shift from peak indices to grain onset indices
    
    for (int p = 0; p < peakIndices.size(); ++p)
    {
        const int peakIndex = peakIndices.getUnchecked (p);
        
        int grainStart = peakIndex - period; // offset the peak index by the period so that each grain will be centered on a peak
        
        if (grainStart > 0)
            targetArray.add (grainStart);
    }
    
    
    
    
   
    // if the peak finding alg missed any @ the beginning or end, fill them in here
    
    int first = (targetArray.size() > 1) ? targetArray.getUnchecked(1) : targetArray.getUnchecked(0);
    first -= period;
    
    while (first >= 0)
    {
        if (! targetArray.contains (first))
            targetArray.add (first);
        
        first -= period;
    }
    
    int last = (targetArray.size() > 1) ? targetArray.getUnchecked(targetArray.size() - 2) : targetArray.getUnchecked(0);
    last += period;
    
    while (last < inputAudio.getNumSamples())
    {
        if (! targetArray.contains (last))
            targetArray.add (last);
        
        last += period;
    }
    
    
    
};


template<typename SampleType>
bool GrainExtractor<SampleType>::isPeriodRepresentedByAPeak (const Array<int>& peaks,
                                                             const int periodMinSample, const int periodMaxSample)
{
    // return true if the peaks array contains, at any index, an element that is >= periodMinSample && < periodMaxSample
    
    
};


template class GrainExtractor<float>;
template class GrainExtractor<double>;
