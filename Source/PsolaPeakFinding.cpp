/*
  ==============================================================================

    PsolaPeakFinding.cpp
    Created: 23 Jan 2021 2:56:54pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "GrainExtractor.h"

template<typename SampleType>
void GrainExtractor<SampleType>::prepareForPsola (const int maxBlocksize)
{
    peakIndices.ensureStorageAllocated(maxBlocksize);
    peakCandidates.ensureStorageAllocated(maxBlocksize);
    candidateDeltas.ensureStorageAllocated(maxBlocksize);
    peakSearchingIndexOrder.ensureStorageAllocated(maxBlocksize);
};


template<typename SampleType>
void GrainExtractor<SampleType>::releasePsolaResources()
{
    peakIndices.clear();
    peakCandidates.clear();
    candidateDeltas.clear();
    peakSearchingIndexOrder.clear();
};



template<typename SampleType>
void GrainExtractor<SampleType>::findPsolaPeaks (Array<int>& targetArray,
                                                 const AudioBuffer<SampleType>& inputAudio,
                                                 const int period)
{
    targetArray.clearQuick();
    
    const SampleType* reading = inputAudio.getReadPointer(0);
    const int totalNumSamples = inputAudio.getNumSamples();
    
    const int outputGrain = 2 * period;
    const int halfPeriod  = ceil (period / 2.0f);
    
    int analysisIndex = 0; // marks the center of the analysis windows (which are 1 period long) [but start @ 0]
    
    while ((analysisIndex - halfPeriod) < totalNumSamples)
    {
        peakCandidates.clearQuick();
        candidateDeltas.clearQuick();
        
        const int grainStart = std::max (analysisIndex - halfPeriod, 0); // starting point of this analysis grain
        const int grainEnd   = targetArray.isEmpty()  ?                  // ending sample of this analysis grain
        std::min (period, totalNumSamples) :
        std::min (analysisIndex + halfPeriod, totalNumSamples);
        
        if (grainStart > grainEnd)
            return;
        
        if (grainStart == grainEnd)
        {
            peakCandidates.add (grainStart);
        }
        else
        {
            sortSampleIndicesForPeakSearching (peakSearchingIndexOrder, grainStart, grainEnd, analysisIndex);
            
            while (peakCandidates.size() < 10) // 10 is arbitrary...
            {
                // identifies the local max & min within the specified analysis window & adds them to the peakCandidates array
                getPeakCandidateInRange (peakCandidates, reading, grainStart, grainEnd, analysisIndex);
                
                if (targetArray.size() > 1) // stop finding peak candidates early, we've already got a good match
                {
                    int target = targetArray.getUnchecked (targetArray.size() - 2) + outputGrain;
                    
                    if ((abs (peakCandidates.getLast() - target)) < 3)
                        break;
                    
                    if (peakCandidates.size() > 1)
                        if ((abs (peakCandidates.getUnchecked (peakCandidates.size() - 2) - target)) < 3)
                            break;
                }
            }
        }
        
        int peakIndex;
        
        if (targetArray.isEmpty() || targetArray.size() == 1)
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
            if (peakCandidates.size() == 1)
            {
                peakIndex = peakCandidates.getUnchecked(0);
            }
            else
            {
                const int target1 = targetArray.getLast() + period;
                const int target2 = targetArray.getUnchecked(targetArray.size() - 2) + outputGrain;
                
                for (int p = 0; p < peakCandidates.size(); ++p)
                {
                    const int delta1 = abs (peakCandidates.getUnchecked(p) - target1);
                    const int delta2 = abs (peakCandidates.getUnchecked(p) - target2) * 2; // weight this delta
                    
                    const float avgDelta = (delta1 + delta2) / 2.0f;
                    
                    candidateDeltas.add (avgDelta);
                    
                    if (avgDelta < 2.0f) // good enough match, we can skip the rest...
                        break;
                }
                
                float minDelta = candidateDeltas.getUnchecked(0);
                int indexOfMinDelta = 0;
                
                for (int d = 1; d < candidateDeltas.size(); ++d)
                {
                    const float delta = candidateDeltas.getUnchecked (d);
                    
                    if (delta <= minDelta)
                    {
                        minDelta = delta;
                        indexOfMinDelta = d;
                    }
                    
                    if (minDelta == 0.0f) // perfect match
                        break;
                }
                
                peakIndex = peakCandidates.getUnchecked (indexOfMinDelta);
            }
        }
        
        targetArray.add (peakIndex);
        
        // our next predicted peak is 1 period away from this peak, so that's the sample where our next analysis window will be centered:
        analysisIndex = peakIndex + period;
    }
};


template<typename SampleType>
void GrainExtractor<SampleType>::getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                                          const int startSample, const int endSample, const int predictedPeak)
{
    const int numSamples = endSample - startSample;
    
    if (numSamples < 1)
        return;
    
    int starting = predictedPeak;
    
    if (candidates.contains (predictedPeak)) // find the sample closest to the predicted peak that's not already chosen as a peak candidate
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
    SampleType localMax = localMin;
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    SampleType multiplier = 1.0; // multiplier to add to the sample values to favor peaks closer to the center of the analysis window
    const SampleType multIncrement = 1.0 / numSamples;
    
    for (int i = 0;
         i < numSamples;
         ++i)
    {
        const int s = peakSearchingIndexOrder.getUnchecked(i);
        
        if (candidates.contains (s))
            continue;
        
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
        
        if (i % 2 == 1)
        {
            multiplier -= multIncrement;
        }
    }
    
    if (indexOfLocalMax == indexOfLocalMin)
    {
        candidates.add (indexOfLocalMax);
    }
    else
    {
        candidates.add (std::min (indexOfLocalMax, indexOfLocalMin));
        candidates.add (std::max (indexOfLocalMax, indexOfLocalMin));
    }
};


template<typename SampleType>
void GrainExtractor<SampleType>::sortSampleIndicesForPeakSearching (Array<int>& output,
                                                                    const int startSample, const int endSample,
                                                                    const int predictedPeak)
{
    output.set (0, predictedPeak);
    
    int p = 1, m = -1;
    bool posLastTime = false;
    
    for (int n = 1;
         n < (endSample - startSample);
         ++n)
    {
        const int pos = predictedPeak + p;
        const int neg = predictedPeak + m;
        
        if (posLastTime)
        {
            if (neg >= startSample)
            {
                output.set (n, neg);
                posLastTime = false;
                --m;
                continue;
            }
            
            jassert (pos <= endSample);
            
            output.set (n, pos);
            posLastTime = true;
            ++p;
        }
        else
        {
            if (pos <= endSample)
            {
                output.set (n, pos);
                posLastTime = true;
                ++p;
                continue;
            }
            
            jassert (neg >= startSample);
            
            peakSearchingIndexOrder.set (n, neg);
            posLastTime = false;
            --m;
        }
    }
};


template class GrainExtractor<float>;
template class GrainExtractor<double>;
