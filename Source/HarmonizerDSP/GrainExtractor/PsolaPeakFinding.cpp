/*
  ==============================================================================

    PsolaPeakFinding.cpp
    Created: 23 Jan 2021 2:56:54pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "GrainExtractor/GrainExtractor.h"

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
        peakCandidates.clearQuick();
        
        // bounds of the current analysis window. analysisIndex = the next predicted peak = the middle of this analysis window
        const int windowStart = std::max (0, analysisIndex - halfPeriod);
        const int windowEnd   = std::min (totalNumSamples, windowStart + period);
        
        if (windowStart == windowEnd) // possible edge case?
        {
            peakCandidates.add (windowStart);
        }
        else
        {
            // create a list of possible peak candidates within the current analysis window, favoring the predicted peak location
            
            sortSampleIndicesForPeakSearching (peakSearchingIndexOrder, windowStart, windowEnd, analysisIndex); // search from middle of analysis window outwards (to favor samples closest to the predicted peak location)
            
            while (peakCandidates.size() < numPeaksToTest) 
                getPeakCandidateInRange (peakCandidates, reading, windowStart, windowEnd, analysisIndex); // may add 1 or 2 peak candidates per call
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
    
    const SampleType startingWeight = (starting == predictedPeak) ? 1.0 : 1.0 - ( ((abs(starting - predictedPeak)) / numSamples) * 0.5 ); // these weighting functions may need tuning...
    SampleType localMin = input[starting] * startingWeight;
    SampleType localMax = localMin;
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    for (int i = 0; i < numSamples; ++i)
    {
        const int index = peakSearchingIndexOrder.getUnchecked(i);
        
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
    candidateDeltas.clearQuick();
    
    // 1. calculate delta values for each peak candidate
    // delta represents how far off this peak candidate is from the expected peak location - in a way it's a measure of the jitter that picking a peak candidate as this frame's peak would introduce to the overall alignment of the stream of grains based on the previous grains
    
    for (int p = 0; p < candidates.size(); ++p)
    {
        // deltaTarget1 = this peak's expected location based on the last peak found (ie, the neighboring OVERLAPPING output OLA grain)
        // deltatarget2 = this peak's expected location based on the second to last peak found (the neighboring CONSECUTIVE OLA grain)
        const int   delta1 = abs (candidates.getUnchecked(p) - deltaTarget1);
        const float delta2 = abs (candidates.getUnchecked(p) - deltaTarget2) * 1.5f; // weight this delta, this value is of more consequence
        
        candidateDeltas.add ((delta1 + delta2) / 2.0f); // average the two delta values
    }
    
    // 2. whittle our remaining candidates down to the final candidates with the minimum delta values
    
    const int finalHandfulSize = std::min (defaultFinalHandfulSize, candidateDeltas.size());
    
    int   finalHandfulIndexs[finalHandfulSize]; // candidate's index in peakCandidates array
    float finalHandfulDeltas[finalHandfulSize]; // delta value for candidate
    
    for (int i = 0; i < finalHandfulSize; ++i)
    {
        float minDelta = candidateDeltas.getUnchecked(0);
        int indexOfMinDelta = 0;
        
        for (int d = 1; d < candidateDeltas.size(); ++d)
        {
            const float delta = candidateDeltas.getUnchecked(d);
            
            if (delta < minDelta)
            {
                minDelta = delta;
                indexOfMinDelta = d;
            }
        }
        
        finalHandfulIndexs[i] = indexOfMinDelta;
        finalHandfulDeltas[i] = minDelta;
        candidateDeltas.set (indexOfMinDelta, 10000.0f); // make sure this value won't be chosen again, w/o deleting it from the candidateDeltas array
    }
    
    // 3. find the candidate in our final handful with the lowest overall delta value, as well as the max delta value for any candidate
    int lowestDeltaCandidate = 0; // index in final handful arrays of candidate w/ lowest delta value
    float lowestDelta  = finalHandfulDeltas[0]; // lowest  delta of any remaining candidate
    float highestDelta = finalHandfulDeltas[0]; // highest delta of any remaining candidate
    
    for (int c = 1; c < finalHandfulSize; ++c)
    {
        const float delta = finalHandfulDeltas[c];
        
        if (delta > highestDelta)
            highestDelta = delta;
        
        if (delta < lowestDelta)
        {
            lowestDelta = delta;
            lowestDeltaCandidate = c;
        }
    }
    
    // 4. choose the strongest overall peak from these final candidates, with peaks weighted by their delta values
    
    const float deltaRange = highestDelta - lowestDelta;
    
    if (deltaRange > 1.0f)
    {
        int strongestPeakIndex = candidates.getUnchecked (finalHandfulIndexs[lowestDeltaCandidate]);
        
        const float startingWeight = (lowestDelta == 0.0f) ? 1.0f : (1.0f - ((lowestDelta / deltaRange) * 0.75f));
        
        SampleType strongestPeak = (abs(reading[strongestPeakIndex])) * startingWeight;
    
        for (int c = 0; c < finalHandfulSize; ++c)
        {
            if (c == lowestDeltaCandidate)
                continue;
            
            const int testingIndex = candidates.getUnchecked (finalHandfulIndexs[c]);
            
            const float testingDelta = finalHandfulDeltas[c];
            const float weight = (testingDelta == 0.0f) ? 1.0f : 1.0f - ((testingDelta / deltaRange) * 0.75f); // weighting function decreases peaks with higher deltas
            
            const SampleType testingPeak = (abs(reading[testingIndex])) * weight;
            
            if (testingPeak < strongestPeak)
                continue;
            
            strongestPeak = testingPeak;
            strongestPeakIndex = testingIndex;
        }
    
        return strongestPeakIndex;
    }
    else
    {
        // there is little or no variation in candidate's delta values, so pick the strongest overall peak
        
        int strongestPeakIndex = candidates.getUnchecked(finalHandfulIndexs[0]);
        SampleType strongestPeak = abs(reading[strongestPeakIndex]);
        
        for (int c = 1; c < finalHandfulSize; ++c)
        {
            const int testingIndex = candidates.getUnchecked(finalHandfulIndexs[c]);
            const SampleType testingPeak = abs(reading[testingIndex]);
            
            if (testingPeak < strongestPeak)
                continue;
            
            strongestPeak = testingPeak;
            strongestPeakIndex = testingIndex;
        }
        
        return strongestPeakIndex;
    }
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
