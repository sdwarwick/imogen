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
    peakIndices.clearQuick();
    
    const int totalNumSamples = inputAudio.getNumSamples();
    
    // identifies peak indices for each pitch period & places them in the peakIndices array
    findPsolaPeaks (peakIndices, inputAudio.getReadPointer(0), totalNumSamples, period);
    
    if (peakIndices.isEmpty())
        return;
    
    // if any periods are missing a peak, fill them in
    for (int p = 0;
         p < totalNumSamples;
         p += period)
    {
        const int thisPeriodStart = p;
        const int thisPeriodEnd = std::min (p + period, totalNumSamples);
        
        if (! isPeriodRepresentedByAPeak (peakIndices, thisPeriodStart, thisPeriodEnd))
        {
            // fill in missing peak for this period, attempting to evenly space it with the surrounding peaks
        }
    }
    
    // if any periods got more than one peak, filter them out
    for (int p = 0;
         p < totalNumSamples;
         p += period)
    {
        const int thisPeriodStart = p;
        const int thisPeriodEnd = std::min (p + period, totalNumSamples);
        
        // find index of first peak within this period
        int thisPeak = 0;
        for (int i = 0; i < peakIndices.size(); ++i)
        {
            if (peakIndices.getUnchecked(i) >= thisPeriodStart)
            {
                thisPeak = peakIndices.getUnchecked(i);
                break;
            }
        }
        
        if (thisPeak < peakIndices.size() - 2)
        {
            if (peakIndices.getUnchecked(thisPeak + 1) > thisPeriodEnd)
            {
                // we've got an extra peak this period
            }
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
    for (int s = periodMinSample; s < periodMaxSample; ++s)
        if (peaks.contains(s))
            return true;
    
    return false;
};


template class GrainExtractor<float>;
template class GrainExtractor<double>;
