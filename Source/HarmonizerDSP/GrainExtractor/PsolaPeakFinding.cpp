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
    // maxBlocksize = max period of input audio
    
    peakSearchingIndexOrder.ensureStorageAllocated (maxBlocksize);
    
    // the peak candidate finding function may output 2 peaks each time it's called, so have an extra slot available in case it outputs up to index newNumCandidatesToTest + 1
    peakCandidates .ensureStorageAllocated (numPeaksToTest + 1);
    candidateDeltas.ensureStorageAllocated (numPeaksToTest + 1);
    
    lastBlocksize = maxBlocksize;
};


template<typename SampleType>
void GrainExtractor<SampleType>::releasePsolaResources()
{
    peakCandidates.clear();
    candidateDeltas.clear();
    peakSearchingIndexOrder.clear();
};


template<typename SampleType>
void GrainExtractor<SampleType>::setNumPeakCandidatesToTest (const int newNumCandidatesToTest)
{
    if (newNumCandidatesToTest < 1)
        return;
    
    if (numPeaksToTest == newNumCandidatesToTest)
        return;
    
    // the peak candidate finding function may output 2 peaks each time it's called, so have an extra slot available in case it outputs up to index newNumCandidatesToTest + 1
    peakCandidates   .ensureStorageAllocated (newNumCandidatesToTest + 1);
    candidateDeltas  .ensureStorageAllocated (newNumCandidatesToTest + 1);
    
    numPeaksToTest = newNumCandidatesToTest;
};


template<typename SampleType>
void GrainExtractor<SampleType>::findPsolaPeaks (Array<int>& targetArray,
                                                 const SampleType* reading,
                                                 const int totalNumSamples,
                                                 const int period)
{
    const int outputGrain = 2 * period;
    const int halfPeriod  = ceil (period / 2.0f);
    
    int analysisIndex = 0; // marks the center of the analysis windows (which are 1 period long) [but start @ 0]
    
    while ((analysisIndex - halfPeriod) < totalNumSamples)
    {
        peakCandidates.clearQuick();
        
        // bounds of the current analysis grain. analysisIndex = the next predicted peak = the middle of this analysis grain
        const int grainStart = std::max (0, analysisIndex - halfPeriod);
        const int grainEnd   = std::min (totalNumSamples, grainStart + period);
        
        if (grainStart > grainEnd)
            return;
        
        if (grainStart == grainEnd)
        {
            peakCandidates.add (grainStart);
        }
        else
        {
            sortSampleIndicesForPeakSearching (peakSearchingIndexOrder, grainStart, grainEnd, analysisIndex);
            
            while (peakCandidates.size() < numPeaksToTest) 
                getPeakCandidateInRange (peakCandidates, reading, grainStart, grainEnd, analysisIndex);
        }
        
        // choose which peak candidate we're picking for this analysis window based on our list of candidates
        
        int peakIndex;
        
        if (peakCandidates.size() == 1)
        {
            peakIndex = peakCandidates.getUnchecked(0);
        }
        else
        {
            if (targetArray.isEmpty() || targetArray.size() == 1)
            {
                // for the first two peaks, choose the point of overall maximum energy in the analysis window, because we have no deltas to compare to for these peaks
                
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
                // for each candidate, create delta values based on how far away they are from this peak's predicated location based on both the last identified peak and the second to last peak.
                // we want the candidate that is closest to being 2 periods away from the peak before last, and is also the closest to being 1 period away from the last found peak.
                
                candidateDeltas.clearQuick();
                
                // 1. calculate delta values for each peak candidate
                // delta represents how far off this peak candidate is from the expected peak location. in a way it's a measure of jitter
                
                const int target1 = targetArray.getLast() + period;
                const int target2 = targetArray.getUnchecked(targetArray.size() - 2) + outputGrain;
                
                for (int p = 0; p < peakCandidates.size(); ++p)
                {
                    const int delta1 = abs (peakCandidates.getUnchecked(p) - target1);
                    const int delta2 = abs (peakCandidates.getUnchecked(p) - target2) * 2; // weight this delta, this value is of more consequence
                    
                    const float avgDelta = (delta1 + delta2) / 2.0f;
                    
                    candidateDeltas.add (avgDelta);
                }
            
                // 2. whittle our peak candidates down to the 4 minimum delta values
                
                const int finalHandfulSize = std::min (4, candidateDeltas.size());
                
                int finalHandful[finalHandfulSize]; // stores the indices in the candidateDeltas array of the lowest 3 delta values. These indices are also the indices in the peakCandidates array where these peaks' actual sample indices are stored.
                
                for (int i = 0; i < finalHandfulSize; ++i)
                {
                    float minDelta = candidateDeltas.getUnchecked(0);
                    int indexOfMinDelta = 0;
                    
                    for (int d = 1; d < candidateDeltas.size(); ++d)
                    {
                        const float delta = candidateDeltas.getUnchecked(d);
                        
                        if (delta <= minDelta)
                        {
                            minDelta = delta;
                            indexOfMinDelta = d;
                        }
                    }
                    
                    finalHandful[i] = indexOfMinDelta;
                    candidateDeltas.remove (indexOfMinDelta);
                }
                
                // 3. choose the strongest overall peak from these final 4 candidates
                
                int chosenPeak = peakCandidates.getUnchecked (finalHandful[0]);
                SampleType strongestPeak = abs (reading[chosenPeak]);
                
                for (int c = 1; c < finalHandfulSize; ++c)
                {
                    const int testing = peakCandidates.getUnchecked (finalHandful[c]);
                    const SampleType testingPeak = abs (reading[testing]);
                    
                    if (testingPeak > strongestPeak)
                    {
                        strongestPeak = testingPeak;
                        chosenPeak = testing;
                    }
                }
                
                peakIndex = chosenPeak;
            }
        }
        
        targetArray.add (peakIndex);
        
        // analysisIndex marks the middle of our next analysis window, so it's where our next predicted peak should be:
        if (targetArray.size() == 1)
            analysisIndex = peakIndex + period;
        else
            analysisIndex = targetArray.getUnchecked(targetArray.size() - 2) + outputGrain;
    }
};


template<typename SampleType>
void GrainExtractor<SampleType>::getPeakCandidateInRange (Array<int>& candidates, const SampleType* input,
                                                          const int startSample, const int endSample, const int predictedPeak)
{
    const int numSamples = endSample - startSample;
    
    int starting = predictedPeak; // sample to start analysis with, for variable initialization
    
    if (candidates.contains (predictedPeak)) // find the sample nearest to the predicted peak that's not already chosen as a peak candidate
    {
        int newStart = -1;
        
        for (int i = 1; i < numSamples; ++i)
        {
            const int s = peakSearchingIndexOrder.getUnchecked(i);
            
            if (candidates.contains (s))
                continue;
            
            newStart = s;
        }
        
        if (newStart == -1)
        {
            candidates.add (predictedPeak); // do this bc this function is called in a while loop whose condition is the size of the output array...
            return;
        }
        
        starting = newStart;
    }
    
    SampleType localMin = input[starting];
    SampleType localMax = localMin;
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    for (int i = 0, p = 0;
         i < numSamples;
         ++i)
    {
        if (i % 2 == 1)
            ++p;
        
        const int s = peakSearchingIndexOrder.getUnchecked(i);
        
        if (s == starting)
            continue;
        
        if (candidates.contains (s))
            continue;
        
        const SampleType weighting = 1.0 - ((p / numSamples) * 0.75);
        
        const SampleType currentSample = input[s] * weighting;
        
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
        return;
    }
    
    if (localMax < 0.0)
    {
        candidates.add (indexOfLocalMin);
        return;
    }
    
    if (localMin > 0.0)
    {
        candidates.add (indexOfLocalMax);
        return;
    }
    
    candidates.add (std::min (indexOfLocalMax, indexOfLocalMin));
    candidates.add (std::max (indexOfLocalMax, indexOfLocalMin));
};


template<typename SampleType>
void GrainExtractor<SampleType>::sortSampleIndicesForPeakSearching (Array<int>& output, // array to write the sorted sample indices to
                                                                    const int startSample, const int endSample,
                                                                    const int predictedPeak)
{
    output.clearQuick();
    
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
