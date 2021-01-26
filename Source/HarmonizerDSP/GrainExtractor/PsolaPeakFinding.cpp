/*
  ==============================================================================

    PsolaPeakFinding.cpp
    Created: 23 Jan 2021 2:56:54pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "GrainExtractor/GrainExtractor.h"


template<typename SampleType>
void GrainExtractor<SampleType>::findPsolaPeaks (Array<int>& targetArray,
                                                 const SampleType* reading,
                                                 const int totalNumSamples,
                                                 const int period)
{
    targetArray.clearQuick();
    
    const int outputGrain = 2 * period;
    const int halfPeriod  = ceil (period / 2.0f);
    
    int analysisIndex = 0; // marks the center of the analysis windows (which are 1 period long) -- but start @ 0
    
    while ((analysisIndex - halfPeriod) < totalNumSamples)
    {
        Array<int> peakCandidates;
        peakCandidates.ensureStorageAllocated (numPeaksToTest + 1);
        
        // bounds of the current analysis window. analysisIndex = the next predicted peak = the middle of this analysis window
        const int windowStart = std::max (0, analysisIndex - halfPeriod);
        const int windowEnd   = std::min (totalNumSamples, windowStart + period);
        
        if (windowStart == windowEnd) // possible edge case?
        {
            if (windowEnd == totalNumSamples) // another possible edge case...
                return;
                
            peakCandidates.add (windowStart);
        }
        else
        {
            Array<int> peakSearchingOrder;
            peakSearchingOrder.ensureStorageAllocated (windowEnd - windowStart);
            
            sortSampleIndicesForPeakSearching (peakSearchingOrder, windowStart, windowEnd, analysisIndex);
            
            while (peakCandidates.size() < numPeaksToTest) 
                getPeakCandidateInRange (peakCandidates, reading, windowStart, windowEnd, analysisIndex, peakSearchingOrder);
        }
        
        // identify the most ideal peak for this analysis window out of our list of candidates
        
        int peakIndex;
        
        if (peakCandidates.size() == 1)
        {
            peakIndex = peakCandidates.getUnchecked(0);
        }
        else
        {
            if (targetArray.size() > 1)
            {
                peakIndex = chooseIdealPeakCandidate (peakCandidates, reading,
                                                      targetArray.getLast() + period,
                                                      targetArray.getUnchecked(targetArray.size() - 2) + outputGrain);
            }
            else
            {
                // for the first two peaks, choose the point of overall maximum energy in the analysis window, because we have no deltas to compare to for these peaks
                
                int strongestPeakIndex = peakCandidates.getUnchecked (0);
                SampleType strongestPeak = abs(reading[strongestPeakIndex]);
                
                for (int candidate : peakCandidates)
                {
                    const SampleType current = abs(reading[candidate]);
                    
                    if (current <= strongestPeak)
                        continue;
                
                    strongestPeak = current;
                    strongestPeakIndex = candidate;
                }
                
                peakIndex = strongestPeakIndex;
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
                                                          const int startSample, const int endSample, const int predictedPeak,
                                                          Array<int>& searchingOrder)
{
    jassert (! searchingOrder.isEmpty());
    
    const int numSamples = endSample - startSample;
    
    int starting = predictedPeak; // sample to start analysis with, for variable initialization
    
    if (candidates.contains (predictedPeak)) // find the sample nearest to the predicted peak that's not already chosen as a peak candidate
    {
        int newStart = -1;
        
        for (int i = 1; i < numSamples; ++i)
        {
            const int s = searchingOrder.getUnchecked(i);
            
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
    
    const SampleType startingWeight = (starting == predictedPeak) ? 1.0 : 1.0 - ( ((abs(starting - predictedPeak)) / numSamples) * 0.5 );
    SampleType localMin = input[starting] * startingWeight;
    SampleType localMax = localMin;
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    for (int i = 0; i < numSamples; ++i)
    {
        const int index = searchingOrder.getUnchecked(i);
        
        if (index == starting)
            continue;
        
        if (candidates.contains (index))
            continue;
        
        const SampleType weighting = 1.0 - ((abs(index - predictedPeak) / numSamples) * 0.5); // these weighting functions may need tuning...
        
        const SampleType currentSample = input[index] * weighting;
        
        if (currentSample < localMin)
        {
            localMin = currentSample;
            indexOfLocalMin = index;
        }
        
        if (currentSample > localMax)
        {
            localMax = currentSample;
            indexOfLocalMax = index;
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
int GrainExtractor<SampleType>::chooseIdealPeakCandidate (const Array<int>& candidates, const SampleType* reading,
                                                          const int deltaTarget1, const int deltaTarget2)
{
    Array<int> candidateDeltas;
    candidateDeltas.ensureStorageAllocated (candidates.size());
    
    // 1. calculate delta values for each peak candidate
    // delta represents how far off this peak candidate is from the expected peak location - in a way it's a measure of the jitter that picking a peak candidate as this frame's peak would introduce to the overall alignment of the stream of grains based on the previous grains
    
    for (int candidate : candidates)
    {
        // deltaTarget1 = this peak's expected location based on the last peak found (ie, the neighboring OVERLAPPING output OLA grain)
        // deltatarget2 = this peak's expected location based on the second to last peak found (the neighboring CONSECUTIVE OLA grain)
        const int   delta1 = abs (candidate - deltaTarget1);
        const float delta2 = abs (candidate - deltaTarget2) * 1.5f; // weight this delta, this value is of more consequence
        
        candidateDeltas.add ((delta1 + delta2) / 2.0f); // average the two delta values
    }
    
    // find the highest delta value of any candidate
    float maxDelta = 0.0f;
    for (float delta : candidateDeltas)
        if (delta > maxDelta)
            maxDelta = delta;
    
    // 2. whittle our remaining candidates down to the final candidates with the minimum delta values
    
    const int finalHandfulSize = std::min (defaultFinalHandfulSize, candidateDeltas.size());
    
    Array<int>   finalHandful;       // copy sample indices of candidates to here from input "candidates" array
    Array<float> finalHandfulDeltas; // delta values for candidates
    
    finalHandful.ensureStorageAllocated (finalHandfulSize);
    finalHandfulDeltas.ensureStorageAllocated (finalHandfulSize);
    
    for (int i = 0; i < finalHandfulSize; ++i)
    {
        float minDelta = maxDelta;
        
        for (int delta : candidateDeltas)
            if (delta < minDelta)
                minDelta = delta;
        
        const int indexOfMinDelta = candidateDeltas.indexOf (minDelta);
        
        finalHandfulDeltas.add (minDelta);
        finalHandful.add (candidates.getUnchecked (indexOfMinDelta));
        
        candidateDeltas.set (indexOfMinDelta, minDelta + maxDelta); // make sure this value won't be chosen again, w/o deleting it from the candidateDeltas array
    }
    
    // 3. identify the highest & lowest delta values for the final handful candidates

    float lowestDelta  = finalHandfulDeltas.getUnchecked (0); // lowest  delta of any remaining candidate
    float highestDelta = lowestDelta;                         // highest delta of any remaining candidate
    
    for (float delta : finalHandfulDeltas)
    {
        if (delta > highestDelta)
            highestDelta = delta;
        
        if (delta < lowestDelta)
            lowestDelta = delta;
    }
    
    // 4. choose the strongest overall peak from these final candidates, with peaks weighted by their delta values
    
    const float deltaRange = highestDelta - lowestDelta;

    int chosenPeak;
    SampleType strongestPeak;
    
    if (deltaRange < 1.0f)
    {
        chosenPeak = finalHandful.getUnchecked (0);
        strongestPeak = (abs(reading[chosenPeak]));
    }
    else
    {
        chosenPeak = finalHandful.getUnchecked (finalHandfulDeltas.indexOf (lowestDelta));
        const float startingWeight = (lowestDelta == 0.0f) ? 1.0f : (1.0f - ((lowestDelta / deltaRange) * 0.75f));
        strongestPeak = (abs(reading[chosenPeak])) * startingWeight;
    }
    
    for (int candidate : finalHandful)
    {
        if (candidate == chosenPeak)
            continue;
        
        SampleType testingPeak;
        
        if (deltaRange < 1.0f)
            testingPeak = (abs(reading[candidate]));
        else
        {
            const float testingDelta = finalHandfulDeltas.getUnchecked (finalHandful.indexOf (candidate));
            const float weight = (testingDelta == 0.0f) ? 1.0f : 1.0f - ((testingDelta / deltaRange) * 0.75f); // weighting function decreases peaks with higher deltas
            testingPeak = (abs(reading[candidate])) * weight;
        }
        
        if (testingPeak < strongestPeak)
            continue;
        
        strongestPeak = testingPeak;
        chosenPeak = candidate;
    }
    
    return chosenPeak;
};



template<typename SampleType>
void GrainExtractor<SampleType>::sortSampleIndicesForPeakSearching (Array<int>& output, // array to write the sorted sample indices to
                                                                    const int startSample, const int endSample,
                                                                    const int predictedPeak)
{
    output.clearQuick();
    
    output.set (0, predictedPeak);
    
    int p = 1, m = -1;
    
    for (int n = 1;
         n < (endSample - startSample);
         ++n)
    {
        const int pos = predictedPeak + p;
        const int neg = predictedPeak + m;
        
        if (n % 2 == 0) // n is even
        {
            if (neg >= startSample)
            {
                output.set (n, neg);
                --m;
                continue;
            }
            
            jassert (pos <= endSample);
            
            output.set (n, pos);
            ++p;
        }
        else
        {
            if (pos <= endSample)
            {
                output.set (n, pos);
                ++p;
                continue;
            }
            
            jassert (neg >= startSample);
            
            output.set (n, neg);
            --m;
        }
    }
};



template class GrainExtractor<float>;
template class GrainExtractor<double>;
