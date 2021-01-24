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
    candidateDeltas.ensureStorageAllocated (numDeltasToTest);
    
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
        
        if (windowStart > windowEnd)  //  ¯\_(ツ)_/¯
            return;
        
        if (windowStart == windowEnd)
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
                // 1. eliminate the weakest peaks from our collection of candidates
                while (peakCandidates.size() > numDeltasToTest)
                {
                    int weakestPeakIndex = peakCandidates.getUnchecked(0);
                    SampleType weakestPeak = abs(reading[weakestPeakIndex]);
                    
                    for (int c = 1; c < peakCandidates.size(); ++c)
                    {
                        const int testingIndex = peakCandidates.getUnchecked(c);
                        const SampleType testingPeak = abs(reading[testingIndex]);
                        
                        if (testingPeak < weakestPeak)
                        {
                            weakestPeak = testingPeak;
                            weakestPeakIndex = testingIndex;
                        }
                    }
                    
                    peakCandidates.remove (weakestPeakIndex);
                }
                
                candidateDeltas.clearQuick();
                
                // 2. calculate delta values for each peak candidate left
                // delta represents how far off this peak candidate is from the expected peak location - in a way it's a measure of jitter
                
                {
                    // this peak's expected location based on the last peak found (ie, the neighboring OVERLAPPING output OLA grain):
                    const int target1 = targetArray.getLast() + period;
                    // this peak's expected location based on the second to last peak found (the neighboring CONSECUTIVE OLA grain):
                    const int target2 = targetArray.getUnchecked(targetArray.size() - 2) + outputGrain;
                
                    for (int p = 0; p < peakCandidates.size(); ++p)
                    {
                        const int delta1 = abs (peakCandidates.getUnchecked(p) - target1);
                        const int delta2 = abs (peakCandidates.getUnchecked(p) - target2) * 2; // weight this delta, this value is of more consequence
                        
                        const float avgDelta = (delta1 + delta2) / 2.0f;
                        
                        candidateDeltas.add (avgDelta);
                    }
                }
            
                // 3. whittle our remaining candidates down to the candidates with the minimum delta values
                
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
                    candidateDeltas.set (indexOfMinDelta, 1000.0f); // make sure this value won't be chosen again, w/o deleting it from the candidateDeltas array
                }
                
                // 4. find the candidate in our final handful with the lowest overall delta value, and the max delta value for any candidate
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
                
                // 5. choose the strongest overall peak from these final candidates, with peaks weighted by their delta values
    
                const float deltaRange = std::max<float> (highestDelta - lowestDelta, 0.01f);
                
                int strongestPeakIndex = peakCandidates.getUnchecked (finalHandfulIndexs[lowestDeltaCandidate]);
                SampleType strongestPeak = (abs(reading[strongestPeakIndex])) * (1.0 - ((lowestDelta / deltaRange) * 0.75));
                
                for (int c = 0; c < finalHandfulSize; ++c)
                {
                    if (c == lowestDeltaCandidate)
                        continue;
                    
                    const float      testingDelta = finalHandfulDeltas[c];
                    const int        testingIndex = peakCandidates.getUnchecked (finalHandfulIndexs[c]);
                    const SampleType weight       = 1.0 - ((testingDelta / deltaRange) * 0.75); // weighting function decreases peaks with higher deltas
                    const SampleType testingPeak  = (abs(reading[testingIndex])) * weight;
                    
                    if (testingPeak < strongestPeak)
                        continue;
                    
                    strongestPeak = testingPeak;
                    strongestPeakIndex = testingIndex;
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
        
        const SampleType weighting = 1.0 - ((p / numSamples) * 0.5); // these weighting functions may need tuning...
        
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
