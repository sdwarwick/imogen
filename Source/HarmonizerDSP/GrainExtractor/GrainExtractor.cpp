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
    prepareForPsola (maxBlocksize);
};


template<typename SampleType>
void GrainExtractor<SampleType>::releaseResources()
{
    releasePsolaResources();
};


template<typename SampleType>
void GrainExtractor<SampleType>::getGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    // PART ONE - find sample indices of points of maximum energy for every pitch period
    // this function identifies a local extrema for every period of the fundamental frequency of the input audio, but these peaks are not garunteed to be spaced evenly, or exhibit any symmetry. The only garuntee is that there are as many peaks are there are full repitions of the signal's period & that each peak represents a local extrema.
    
    findPsolaPeaks (peakIndices, inputAudio, period);
    
    const int targetNumPeaks = floor (inputAudio.getNumSamples() / period);
    
    if (peakIndices.size() > targetNumPeaks)
    {
        
    }
    else if (peakIndices.size() < targetNumPeaks)
    {
        
    }
    
    
    // PART TWO - create array of grain onset indices, such that grains are 2 pitch periods long, centred on points of maximum energy, w/ approx 50% overlap
    
    targetArray.clearQuick();
    
    
    // step 1 - shift from peak indices to grain onset indices
    
    for (int p = 0; p < peakIndices.size(); ++p)
    {
        const int peakIndex = peakIndices.getUnchecked (p);
        
        int grainStart = peakIndex - period; // offset the peak index by the period so that each grain will be centered on a peak
        
        if (grainStart > 0)
            targetArray.add (grainStart);
    }
    
    int last = targetArray.getLast() + period;
    while (last < inputAudio.getNumSamples())
    {
        targetArray.add (last);
        last += period;
    }
    
    
    /*
     step 2 - attempt to preserve/create symmetry such that for all grains grainStart[p]:
     
     continuous grains in the same "stream":
     grainStart[p - 2] = grainStart[p] - (2 * period)
     grainStart[p + 2] = grainStart[p] + (2 * period)
     
     neighboring overlapping grains:
     grainStart[p - 1] = grainStart[p] - period
     grainStart[p + 1] = grainStart[p] + period
     */
    
    const int periodDoubled = 2 * period;
    const int numSamples = inputAudio.getNumSamples();
    
    for (int p = 0; p < targetArray.size(); ++p)
    {
        int g = targetArray.getUnchecked (p);
        
        if ((p - 1) >= 0)
        {
            if ((p - 2) >= 0)
            {
                /*
                 this is the neighboring grain in the SAME CONTINUOUS STREAM of grains! The symmetry here is most important
                 shift the CURRENT grain (grainStart[p]) to make this equation true:
                 grainStart[p - 2] = grainStart[p] - 2 * period
                 or
                 grainStart[p] = grainStart[p - 2] + 2 * period
                 */
                
                const int g00 = targetArray.getUnchecked (p - 2);
                
                const int targetG = g00 + periodDoubled;
                
                if (targetG < numSamples)
                {
                    targetArray.set (p, targetG);
                    g = targetG;
                }
            }
            
            // this is the neighboring OVERLAPPING grain - the overlap need not be *exactly* 50%, but keeping it as synchronous as possible is desirable - so we're only correcting this index if the OLA % of the grain's current position is lower than 25% or higher than 75%
            
            const int target_g0 = g - period;
            const int actual_g0 = targetArray.getUnchecked (p - 1);
            
            if (target_g0 != actual_g0 && target_g0 > 0)
            {
                const float olaPcnt = abs(g - actual_g0) / periodDoubled;
                
                if (olaPcnt > 0.75f || olaPcnt < 0.25f)
                    targetArray.set (p - 1, target_g0);
            }
        }
        
        if ((p + 1) < targetArray.size())
        {
            if ((p + 2) < targetArray.size())
            {
                // this is the neighboring grain in the SAME CONTINUOUS STREAM of grains! The symmetry here is most important
                // shift the FUTURE grain (grainStart[p + 2]) to make this equation true: grainStart[p + 2] = grainStart[p] + 2 * period
                
                const int target_g2 = g + periodDoubled;
                
                if (target_g2 < numSamples)
                    targetArray.set (p + 2, target_g2);
            }
            
            // this is the neighboring OVERLAPPING grain - the overlap need not be *exactly* 50%, but keeping it as synchronous as possible is desirable
            
            const int target_g1 = g + period;
            const int actual_g1 = targetArray.getUnchecked (p + 1);
            
            if (target_g1 != actual_g1 && target_g1 < numSamples)
            {
                const float olaPcnt = abs(g - actual_g1) / periodDoubled;
                
                if (olaPcnt > 0.75f || olaPcnt < 0.25f)
                    targetArray.set (p + 1, target_g1);
            }
        }
    }
};



template class GrainExtractor<float>;
template class GrainExtractor<double>;
