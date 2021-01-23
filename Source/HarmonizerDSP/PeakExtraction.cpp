/*
  ==============================================================================

    PeakExtraction.cpp
    Created: 22 Jan 2021 8:36:07pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Harmonizer.h"


template<typename SampleType>
void Harmonizer<SampleType>::extractGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    // PART ONE - find sample indices of points of maximum energy for every pitch period
    // this function identifies a local extrema for every period of the fundamental frequency of the input audio, but these peaks are not garunteed to be spaced evenly, or exhibit any symmetry. The only garuntee is that there are as many peaks are there are full repitions of the signal's period & that each peak represents a local extrema.
    
    findPeaks (peakIndices, inputAudio, period);
    
    const int targetNumPeaks = floor (inputAudio.getNumSamples() / period);
    
    if (peakIndices.size() > targetNumPeaks)
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



template<typename SampleType>
void Harmonizer<SampleType>::findPeaks (Array<int>& targetArray,
                                        const AudioBuffer<SampleType>& inputAudio,
                                        const int period)
{
    targetArray.clearQuick();
    
    const SampleType* reading = inputAudio.getReadPointer(0);
    const int totalNumSamples = inputAudio.getNumSamples();
    
    const int outputGrain = 2 * period;
    const int halfPeriod  = ceil (period / 2.0f);
    
    int analysisIndex = halfPeriod; // marks the center of the analysis windows (which are 1 period long)
    
    while (analysisIndex < totalNumSamples)
    {
        peakCandidates.clearQuick();
        candidateDeltas.clearQuick();
        peakSearchingIndexOrder.fill (0);
        
        const int grainStart = analysisIndex - halfPeriod; // starting point of this analysis grain
        const int grainEnd = std::min (analysisIndex + halfPeriod, totalNumSamples); // ending sample
        
        while (peakCandidates.size() < 10) // 10 is arbitrary...
        {
            // identifies the local max & min within the specified analysis window & adds them to the peakCandidates array
            getPeakCandidateInRange (peakCandidates, reading, grainStart, grainEnd,
                                     analysisIndex); // predicted peak - sample index 1 period away from prev peak
            
            if (targetArray.size() > 1) // stop finding peak candidates early, we've already got a good match
            {
                int target = targetArray.getUnchecked (targetArray.size() - 2) + outputGrain;
                
                const int delta1 = abs (peakCandidates.getLast() - target);
                
                if (delta1 < 3)
                    break;
                
                const int delta2 = abs (peakCandidates.getUnchecked(peakCandidates.size() - 2) - target);
                
                if (delta2 < 3)
                    break;
            }
        }
        
        int peakIndex;
        
        if (targetArray.isEmpty())
        {
            int strongestPeakIndex = peakCandidates.getUnchecked (0);
            SampleType strongestPeak = abs(reading[strongestPeakIndex]);
            
            for (int p = 1; p < peakCandidates.size(); ++p)
            {
                const int index = peakCandidates.getUnchecked (p);
                const SampleType current = abs(reading[index]);
                
                if (current >= strongestPeak)
                {
                    strongestPeak = current;
                    strongestPeakIndex = index;
                }
            }
            
            peakIndex = strongestPeakIndex;
        }
        else
        {
            int target;
            
            if (targetArray.size() == 1)
                target = targetArray.getUnchecked (0) + period;
            else
                target = targetArray.getUnchecked (targetArray.size() - 2) + outputGrain;
            
            for (int p = 0; p < peakCandidates.size(); ++p)
            {
                const int delta = abs (peakCandidates.getUnchecked(p) - target);
                candidateDeltas.add (delta);
                
                if (delta < 2)
                    break;
            }
            
            int minDelta = candidateDeltas.getUnchecked (0);
            int indexOfMinDelta = 0;
            
            for (int d = 1; d < candidateDeltas.size(); ++d)
            {
                const int delta = candidateDeltas.getUnchecked (d);
                
                if (delta <= minDelta)
                {
                    minDelta = delta;
                    indexOfMinDelta = d;
                }
                
                if (minDelta == 0)
                    break;
            }
            
            peakIndex = peakCandidates.getUnchecked (indexOfMinDelta);
        }
        
        targetArray.add (peakIndex);
        
        // our next predicted peak is 1 period away from this peak, so that's the sample where our next analysis window will be centered:
        analysisIndex = peakIndex + period;
    }
};



template<typename SampleType>
void Harmonizer<SampleType>::getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                                      const int startSample, const int endSample, const int predictedPeak)
{
    if (startSample == endSample)
    {
        if (! candidates.contains (startSample))
            candidates.add (startSample);
        
        return;
    }
    
    const int numSamples = endSample - startSample;
    
    if (numSamples < 1)
        return;
    
    peakSearchingIndexOrder.set (0, predictedPeak);
    
    for (int s = predictedPeak + 1, i = 1;
         s <= endSample;
         ++s, i += 2)
    {
        peakSearchingIndexOrder.set (i, s);
    }
    
    for (int s = predictedPeak - 1, i = 2;
         s >= startSample;
         --s, i += 2)
    {
        peakSearchingIndexOrder.set (i, s);
    }
    
    int starting = predictedPeak;
    
    if (! candidates.isEmpty() && candidates.contains (predictedPeak))
    {
        int newStart = -1;
        
        for (int i = 0; i < numSamples; ++i)
        {
            const int s = peakSearchingIndexOrder.getUnchecked(i);
            
            if (! candidates.contains (s))
            {
                newStart = s;
                break;
            }
        }
        
        if (newStart == -1)
            return;
        
        starting = newStart;
    }
    
    SampleType localMin = input[starting];
    SampleType localMax = input[starting];
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    for (int i = 0;
         i < numSamples;
         ++i)
    {
        const int s = peakSearchingIndexOrder.getUnchecked(i);
        
        if (candidates.contains (s))
            continue;
        
        // apply a weighting function to this sample - make it less pos/neg if it's further from the center of the analysis window (ie, i > 0)
        SampleType multiplier;
        
        if (i == 0)
            multiplier = 1.0;
        else
        {
            int divisor = (i % 2 == 0) ? i - 1 : i;
            multiplier = 1.0 / divisor;
        }
        
        const SampleType currentSample = input[s] * multiplier;
        
        if (currentSample < localMin)
        {
            localMin = currentSample;
            indexOfLocalMin = s;
        }
        
        if (currentSample > localMax)
        {
            localMax = currentSample;
            indexOfLocalMax = s;
        }
    }
    
    if (indexOfLocalMax == indexOfLocalMin)
    {
        candidates.add (indexOfLocalMax);
    }
    else
    {
        const int firstPeak  = std::min (indexOfLocalMax, indexOfLocalMin);
        const int secondPeak = (firstPeak == indexOfLocalMax) ? indexOfLocalMin : indexOfLocalMax;
        
        candidates.add (firstPeak);
        candidates.add (secondPeak);
    }
};


template class Harmonizer<float>;
template class Harmonizer<double>;
