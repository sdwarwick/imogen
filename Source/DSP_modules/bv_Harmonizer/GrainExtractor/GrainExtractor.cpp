
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 GrainExtractor.cpp: This file defines implementation details for the GrainExtractor class.
 
======================================================================================================================================================*/




#include "GrainExtractor.h"

#define bvhge_NUM_PEAKS_TO_TEST 10
#define bvhge_DEFAULT_FINAL_HANDFUL_SIZE 5


namespace bav
{
    
    
template<typename SampleType>
GrainExtractor<SampleType>::GrainExtractor()
{ }


template<typename SampleType>
GrainExtractor<SampleType>::~GrainExtractor()
{ }
    
    
template<typename SampleType>
void GrainExtractor<SampleType>::releaseResources()
{
    peakIndices.clear();
    peakCandidates.clear();
    peakSearchingOrder.clear();
    candidateDeltas.clear();
    finalHandful.clear();
    finalHandfulDeltas.clear();
}
    
    
template<typename SampleType>
void GrainExtractor<SampleType>::prepare (const int maxBlocksize)
{
    // maxBlocksize = max period of input audio
    
    jassert (maxBlocksize > 0);
    
    peakIndices.ensureStorageAllocated (maxBlocksize);
    
    peakCandidates.ensureStorageAllocated (bvhge_NUM_PEAKS_TO_TEST + 1);
    peakCandidates.clearQuick();
    peakSearchingOrder.ensureStorageAllocated(maxBlocksize);
    peakSearchingOrder.clearQuick();
    candidateDeltas.ensureStorageAllocated (bvhge_NUM_PEAKS_TO_TEST);
    candidateDeltas.clearQuick();
    finalHandful.ensureStorageAllocated (bvhge_NUM_PEAKS_TO_TEST);
    finalHandful.clearQuick();
    finalHandfulDeltas.ensureStorageAllocated (bvhge_NUM_PEAKS_TO_TEST);
    finalHandfulDeltas.clearQuick();
}


template<typename SampleType>
void GrainExtractor<SampleType>::getGrainOnsetIndices (IArray& targetArray,
                                                       const juce::AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    targetArray.clearQuick();
    
    const int totalNumSamples = inputAudio.getNumSamples();
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    // identify  peak indices for each pitch period & places them in the peakIndices array
    
    findPsolaPeaks (peakIndices, reading, totalNumSamples, period);
    
    jassert (! peakIndices.isEmpty());
    
    const int grainLength = period * 2;
    const int numSamples = inputAudio.getNumSamples();
    const int halfPeriod = juce::roundToInt (period * 0.5f);
    
    // create array of grain start indices, such that grains are 2 pitch periods long, CENTERED on points of synchronicity previously identified
    
    for (int i = 0; i < peakIndices.size(); ++i)
    {
        const int peakIndex = peakIndices.getUnchecked(i);
        
        int grainStart = peakIndex - period; // offset the peak index by the period so that the peak index will be in the center of the grain (if grain is 2 periods long)
        
        if (grainStart < 0)
        {
            if (i < peakIndices.size() - 2 || targetArray.size() > 1)
                continue;
            
            while (grainStart < 0)
                grainStart += halfPeriod;
        }
        
        if (grainStart + grainLength > numSamples)
        {
            if (i < peakIndices.size() - 2 || targetArray.size() > 1)
                continue;
            
            const int quarterPeriod = juce::roundToInt (halfPeriod * 0.5f);
            
            while (grainStart + grainLength > numSamples)
                grainStart -= quarterPeriod;
            
            if (grainStart < 0)
                grainStart = 0;
        }
        
        targetArray.add (grainStart);
    }
    
    jassert (! targetArray.isEmpty());
    
#if JUCE_DEBUG
    for (int i = 2; i < targetArray.size(); ++i)
        jassert (targetArray.getUnchecked(i) > targetArray.getUnchecked(i - 2));
#endif
}
    
    
template<typename SampleType>
inline void GrainExtractor<SampleType>::findPsolaPeaks (IArray& targetArray,
                                                        const SampleType* reading,
                                                        const int totalNumSamples,
                                                        const int period)
{
    targetArray.clearQuick();
    
    const int grainSize = 2 * period; // output grains are 2 periods long w/ 50% overlap
    const int halfPeriod = juce::roundToInt (period * 0.5f);
    
    jassert (totalNumSamples >= grainSize);
    
    int analysisIndex = halfPeriod; // marks the center of the analysis windows, which are 1 period long
    
    do {
        const int frameStart = analysisIndex - halfPeriod;
        const int frameEnd = std::min (totalNumSamples, frameStart + period); // analysis grains are 1 period long
        
        jassert (frameStart >= 0 && frameEnd <= totalNumSamples);
        
        targetArray.add (findNextPeak (frameStart, frameEnd,
                                       std::min (analysisIndex, frameEnd), // predicted peak location for this frame
                                       reading, targetArray, period, grainSize));
        
        jassert (! targetArray.isEmpty());
        
        const int prevAnalysisIndex = analysisIndex;
        const int targetArraySize = targetArray.size();
        
        // analysisIndex marks the middle of our next analysis window, so it's where our next predicted peak should be:
        if (targetArraySize == 1)
            analysisIndex = targetArray.getUnchecked(0) + period;
        else
            analysisIndex = targetArray.getUnchecked(targetArraySize - 2) + grainSize;
        
        if (analysisIndex == prevAnalysisIndex)
            analysisIndex = prevAnalysisIndex + period;
        else
            jassert (analysisIndex > prevAnalysisIndex);
    }
    while (analysisIndex - halfPeriod < totalNumSamples);
}
    

template<typename SampleType>
inline int GrainExtractor<SampleType>::findNextPeak (const int frameStart, const int frameEnd, int predictedPeak,
                                                     const SampleType* reading,
                                                     const IArray& targetArray,
                                                     const int period, const int grainSize)
{
    jassert (frameEnd > frameStart);
    jassert (predictedPeak >= frameStart && predictedPeak <= frameEnd);
    
    peakSearchingOrder.clearQuick();
    sortSampleIndicesForPeakSearching (peakSearchingOrder, frameStart, frameEnd, predictedPeak);
    
    jassert (peakSearchingOrder.size() == frameEnd - frameStart);
    
    peakCandidates.clearQuick();
    
    for (int i = 0; i < bvhge_NUM_PEAKS_TO_TEST; ++i)
        getPeakCandidateInRange (peakCandidates, reading, frameStart, frameEnd, predictedPeak, peakSearchingOrder);
    
    jassert (! peakCandidates.isEmpty());
    
#undef bvhge_NUM_PEAKS_TO_TEST
    
    switch (peakCandidates.size())
    {
        case 1:
            return peakCandidates.getUnchecked(0);
            
        case 2:
            return choosePeakWithGreatestPower (peakCandidates, reading);
            
        default:
        {
            if (targetArray.size() <= 1)
                return choosePeakWithGreatestPower (peakCandidates, reading);
            
            return chooseIdealPeakCandidate (peakCandidates, reading,
                                             targetArray.getLast() + period,
                                             targetArray.getUnchecked(targetArray.size() - 2) + grainSize);
        }
    }
}
    

template<typename SampleType>
inline void GrainExtractor<SampleType>::getPeakCandidateInRange (IArray& candidates, const SampleType* input,
                                                                 const int startSample, const int endSample, const int predictedPeak,
                                                                 const IArray& searchingOrder)
{
    jassert (! searchingOrder.isEmpty());
    
    int starting = -1;
    
    for (int poss : searchingOrder)
    {
        if (! candidates.contains (poss))
        {
            starting = poss;
            break;
        }
    }
    
    if (starting == -1)
        return;
    
#define bvhge_WEIGHT(index, predicted, numSamples) SampleType(1.0 - ((abs(index - predicted) / numSamples) * 0.5))
    
    jassert (starting >= startSample && starting <= endSample);
    
    const int numSamples = endSample - startSample;
    
    SampleType localMin = input[starting] * bvhge_WEIGHT(starting, predictedPeak, numSamples);
    SampleType localMax = localMin;
    int indexOfLocalMin = starting;
    int indexOfLocalMax = starting;
    
    for (int index : searchingOrder)
    {
        if (index == starting || candidates.contains (index))
            continue;
        
        jassert (index >= startSample && index <= endSample);
        
        const SampleType currentSample = input[index] * bvhge_WEIGHT(index, predictedPeak, numSamples);
        
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
    
#undef bvhge_WEIGHT
    
    if (indexOfLocalMax == indexOfLocalMin)
    {
        candidates.add (indexOfLocalMax);
    }
    else if (localMax < SampleType(0.0))
    {
        candidates.add (indexOfLocalMin);
    }
    else if (localMin > SampleType(0.0))
    {
        candidates.add (indexOfLocalMax);
    }
    else
    {
        candidates.add (std::min (indexOfLocalMax, indexOfLocalMin));
        candidates.add (std::max (indexOfLocalMax, indexOfLocalMin));
    }
}


template<typename SampleType>
inline int GrainExtractor<SampleType>::chooseIdealPeakCandidate (const IArray& candidates, const SampleType* reading,
                                                                 const int deltaTarget1, const int deltaTarget2)
{
    candidateDeltas.clearQuick();
    finalHandful.clearQuick();
    finalHandfulDeltas.clearQuick();
    
    // 1. calculate delta values for each peak candidate
    // delta represents how far off this peak candidate is from the expected peak location - in a way it's a measure of the jitter that picking a peak candidate as this frame's peak would introduce to the overall alignment of the stream of grains based on the previous grains
    
    for (int candidate : candidates)
    {
        candidateDeltas.add ((abs (candidate - deltaTarget1)
                           + (abs (candidate - deltaTarget2) * 1.5f))
                             * 0.5f);
    }
    
    // 2. whittle our remaining candidates down to the final candidates with the minimum delta values
    
    const int finalHandfulSize = std::min (bvhge_DEFAULT_FINAL_HANDFUL_SIZE, candidateDeltas.size());
#undef bvhge_DEFAULT_FINAL_HANDFUL_SIZE
    
    float minimum = 0.0f;
    int minimumIndex = 0;
    const int dataSize = candidateDeltas.size();
    
    for (int i = 0; i < finalHandfulSize; ++i)
    {
        bav::vecops::findMinAndMinIndex (candidateDeltas.getRawDataPointer(), dataSize, minimum, minimumIndex);
        
        finalHandfulDeltas.add (minimum);
        finalHandful.add (candidates.getUnchecked (minimumIndex));
        
        candidateDeltas.set (minimumIndex, 1000.0f); // make sure this value won't be chosen again, w/o deleting it from the candidateDeltas array
    }
    
    jassert (finalHandful.size() == finalHandfulSize && finalHandfulDeltas.size() == finalHandfulSize);
    
    // 3. choose the strongest overall peak from these final candidates, with peaks weighted by their delta values
    
    const float deltaRange = bav::vecops::findRangeOfExtrema (finalHandfulDeltas.getRawDataPointer(), finalHandfulDeltas.size());
    
    if (deltaRange < 0.05f)  // prevent dividing by 0 in the next step...
        return finalHandful.getUnchecked(0);
    
#define bvhge_DELTA_WEIGHT(delta, deltaRange) 1.0f - ((delta / deltaRange) * 0.75f)
    
    int chosenPeak = finalHandful.getUnchecked (0);
    SampleType strongestPeak = abs(reading[chosenPeak]) * bvhge_DELTA_WEIGHT(finalHandfulDeltas.getUnchecked(0), deltaRange);
    
    for (int i = 1; i < finalHandfulSize; ++i)
    {
        const int candidate = finalHandful.getUnchecked(i);
        
        if (candidate == chosenPeak)
            continue;
        
        SampleType testingPeak = abs(reading[candidate]) * bvhge_DELTA_WEIGHT(finalHandfulDeltas.getUnchecked(i), deltaRange);
        
        if (testingPeak > strongestPeak)
        {
            strongestPeak = testingPeak;
            chosenPeak = candidate;
        }
    }
    
#undef bvhge_DELTA_WEIGHT
    
    return chosenPeak;
}


template<typename SampleType>
inline int GrainExtractor<SampleType>::choosePeakWithGreatestPower (const IArray& candidates, const SampleType* reading)
{
    int strongestPeakIndex = candidates.getUnchecked (0);
    SampleType strongestPeak = abs(reading[strongestPeakIndex]);
    
    for (int candidate : candidates)
    {
        const SampleType current = abs(reading[candidate]);
        
        if (current > strongestPeak)
        {
            strongestPeak = current;
            strongestPeakIndex = candidate;
        }
    }
    
    return strongestPeakIndex;
}


template<typename SampleType>
inline void GrainExtractor<SampleType>::sortSampleIndicesForPeakSearching (IArray& output, // array to write the sorted sample indices to
                                                                           const int startSample, const int endSample,
                                                                           const int predictedPeak)
{
    jassert (predictedPeak >= startSample && predictedPeak <= endSample);
    
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
            }
            else
            {
                jassert (pos <= endSample);
                output.set (n, pos);
                ++p;
            }
        }
        else
        {
            if (pos <= endSample)
            {
                output.set (n, pos);
                ++p;
            }
            else
            {
                jassert (neg >= startSample);
                output.set (n, neg);
                --m;
            }
        }
    }
}



template class GrainExtractor<float>;
template class GrainExtractor<double>;


} // namespace
