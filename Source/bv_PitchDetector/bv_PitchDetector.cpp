/*
  ==============================================================================

    PitchDetector.cpp
    Created: 18 Jan 2021 11:31:33am
    Author:  Ben Vining

  ==============================================================================
*/

#include "bv_PitchDetector.h"


template<typename SampleType>
PitchDetector<SampleType>::PitchDetector(const int minDetectableHz, const int maxDetectableHz, const double initSamplerate): confidenceThresh(0.25)
{
    minHz = minDetectableHz;
    maxHz = maxDetectableHz;
    samplerate = initSamplerate;
    
    asdfBuffer.setSize (1, 500);
    
    setHzRange (minHz, maxHz, true);
    
    lastEstimatedPeriod = minPeriod;
};


template<typename SampleType>
PitchDetector<SampleType>::~PitchDetector()
{
    
};


template<typename SampleType>
void PitchDetector<SampleType>::setHzRange (const int newMinHz, const int newMaxHz, const bool allowRecalc)
{
    jassert (newMaxHz > newMinHz);
    
    if ((! allowRecalc)
        && ((minHz == newMinHz) && (maxHz == newMaxHz)))
        return;
    
    maxPeriod = juce::roundToInt (samplerate / minHz);
    minPeriod = juce::roundToInt (samplerate / maxHz);
    
    if (! (maxPeriod > minPeriod))
        ++maxPeriod;
    
    const int numOfLagValues = maxPeriod - minPeriod + 1;
    
    if (asdfBuffer.getNumSamples() != numOfLagValues)
        asdfBuffer.setSize (1, numOfLagValues, true, true, true);
};


template<typename SampleType>
void PitchDetector<SampleType>::setSamplerate (const double newSamplerate, const bool recalcHzRange)
{
    if (samplerate == newSamplerate)
        return;
    
    if (lastFrameWasPitched)
    {
        SampleType lastHz = static_cast<SampleType> (samplerate / lastEstimatedPeriod);
        lastEstimatedPeriod = SampleType(newSamplerate) / lastHz;
    }
    
    samplerate = newSamplerate;
    
    if (recalcHzRange)
        setHzRange (minHz, maxHz, true);
};


template<typename SampleType>
float PitchDetector<SampleType>::detectPitch (const juce::AudioBuffer<SampleType>& inputAudio)
{
    // this function should return the pitch in Hz, or -1 if the frame of audio is determined to be unpitched
    
    const int numSamples = inputAudio.getNumSamples();
    
    if (numSamples < minPeriod)
        return -1.0f;
    
    jassert (asdfBuffer.getNumSamples() > (maxPeriod - minPeriod));
    
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    // the minPeriod & maxPeriod members define the overall global period range; here, the minLag & maxLag local variables are used to define the period range for this specific frame of audio, if it can be constrained more than the global range.
    
    int minLag = samplesToFirstZeroCrossing (reading, numSamples);
    int maxLag = maxPeriod;
    
    if (lastFrameWasPitched)
    {
        // pitch shouldn't halve or double between consecutive voiced frames
        minLag = std::max (minLag, juce::roundToInt (lastEstimatedPeriod / 2.0));
        maxLag = std::min (maxLag, juce::roundToInt (lastEstimatedPeriod * 2.0));
    }
    
    minLag = std::max (minLag, minPeriod);
    
    // truncation of edge cases
    if (maxLag < minLag)
        return -1.0f;
    else if (minLag == maxLag)
    {
        if (minLag > 1)
            --minLag; // minLag must be greater than or equal to 1
        else
        {
            ++maxLag;
            if (maxLag > maxPeriod)
                return -1.0f;
        }
    }
    
    const int middleIndex = juce::roundToInt(floor (numSamples / 2));
    const int halfNumSamples = juce::roundToInt(floor ((numSamples - 1) / 2));
    
    SampleType* asdfData = asdfBuffer.getWritePointer(0);
    
    // COMPUTE ASDF
    
    for (int k = minPeriod; // always write the same datasize to asdfBuffer, even if k values are being limited this frame
            k <= maxPeriod; // k = lag = period
            ++k)
    {
        const int index = k - minPeriod; // the actual asdfBuffer index for this k value's data. offset = minPeriod
        
        if (k < minLag || k > maxLag) // range compression of k is done here
        {
            asdfData[index] = 1000.0f; // still need to write a value to the asdf buffer, but make sure this would never be chosen as a possible minimum
            continue;
        }
        
        asdfData[index] = 0.0;
        
        const int sampleOffset = middleIndex - juce::roundToInt((floor (k / 2)));
        
        for (int s = sampleOffset - halfNumSamples;
                 s < sampleOffset + halfNumSamples;
               ++s)
        {
            const SampleType difference = reading[s] - reading[s + k];
            asdfData[index] += (difference * difference);
        }
        
        asdfData[index] /= numSamples; // normalize
    }
    
    const int asdfDataSize = maxPeriod - minPeriod + 1; // # of samples written to asdfBuffer
    
    const int minIndex = indexOfMinElement (asdfData, asdfDataSize);
    
    const SampleType greatestConfidence = asdfData[minIndex];
    
    if (greatestConfidence > confidenceThresh) // determine if frame is unpitched - return early
    {
        lastFrameWasPitched = false;
        return -1.0f;
    }
    
    if ((! lastFrameWasPitched) || (greatestConfidence < 0.05)) // separate confidence threshold value for this??
        return foundThePeriod (asdfData, minIndex, asdfDataSize);
    
    return chooseIdealPeriodCandidate (asdfData, asdfDataSize, minIndex);
};



template<typename SampleType>
float PitchDetector<SampleType>::foundThePeriod (const SampleType* asdfData,
                                                 const int minIndex,
                                                 const int asdfDataSize)
{
    SampleType realPeriod = quadraticPeakPosition (asdfData, minIndex, asdfDataSize);
    realPeriod += minPeriod;
    
    jassert (realPeriod <= maxPeriod);
    
    lastEstimatedPeriod = realPeriod;
    lastFrameWasPitched = true;
    return static_cast<float> (samplerate / realPeriod); // return pitch in hz
};


template<typename SampleType>
float PitchDetector<SampleType>::chooseIdealPeriodCandidate (const SampleType* asdfData,
                                                             const int asdfDataSize,
                                                             const int minIndex) // index of minimum asdf data value
{
    const int periodCandidatesSize = std::min(periodCandidatesToTest, asdfDataSize);
    
    juce::Array<int> periodCandidates;
    periodCandidates.ensureStorageAllocated (periodCandidatesSize);
    
    periodCandidates.add (minIndex);
    
    for (int c = 1; c < periodCandidatesSize; ++c)
        getNextBestPeriodCandidate (periodCandidates, asdfData, asdfDataSize);
    
    if (periodCandidates.size() == 1)
        return foundThePeriod (asdfData, minIndex, asdfDataSize);
    
    // find the greatest & least confidences of any candidate (ie, highest & lowest asdf data values)
    // lower  asdf data --> higher confidence
    // higher asdf data --> lower  confidence
    
    const SampleType greatestConfidence = asdfData[minIndex];
    SampleType leastConfidence = greatestConfidence;
    
    for (int c = 1; c < periodCandidatesSize; ++c) // first element of periodCandidates is the index of the minimum element in the asdf
    {
        const SampleType confidence = asdfData[periodCandidates.getUnchecked(c)];
        
        if (confidence > leastConfidence)
            leastConfidence = confidence;
    }
    
    // if there is little variation in the confidences of our candidates, return the smallest k value that is a candidate
    if ((leastConfidence - greatestConfidence) < 0.35)
    {
        int smallestK = periodCandidates.getUnchecked(0);
        
        for (int candidate : periodCandidates)
            if (candidate < smallestK)
                smallestK = candidate;
        
        return foundThePeriod (asdfData, smallestK, asdfDataSize);
    }
    
    // candidate deltas: how far away each period candidate is from the last estimated period
    
    juce::Array<int> candidateDeltas;
    candidateDeltas.ensureStorageAllocated (periodCandidatesSize);
    
    for (int candidate : periodCandidates)
        candidateDeltas.add (juce::roundToInt(abs(candidate + minPeriod - lastEstimatedPeriod)));
    
    // find min & max delta val of any candidate we have
    int minDelta = candidateDeltas.getUnchecked(0);
    int maxDelta = minDelta;
    
    for (int delta : candidateDeltas)
    {
        if (delta < minDelta)
            minDelta = delta;
        
        if (delta > maxDelta)
            maxDelta = delta;
    }
    
    const int deltaRange = maxDelta - minDelta;
    
    if (deltaRange < 4) // all deltas are very close, so return the candidate with the min asdf data value
        return foundThePeriod (asdfData, minIndex, asdfDataSize);
    
    // weight the asdf data based on each candidate's delta value
    // because higher asdf values represent a lower confidence in that period candidate, we want to artificially increase the asdf data a bit for candidates with higher deltas
    
    juce::Array<SampleType> weightedCandidateConfidence;
    weightedCandidateConfidence.ensureStorageAllocated(periodCandidatesSize);
    
    // SampleType weightedCandidateConfidence[periodCandidatesSize];
    
    for (int c = 0; c < periodCandidatesSize; ++c)
    {
        const int candidate = periodCandidates.getUnchecked(c);
        const int delta = candidateDeltas.getUnchecked(c);
        
        if (delta == 0)
            weightedCandidateConfidence.add (asdfData[candidate]);
        else
        {
            const SampleType weight = SampleType(1.0) + ((delta / deltaRange) * SampleType(0.5));
            weightedCandidateConfidence.add (asdfData[candidate] * weight);
        }
    }
    
    // choose the estimated period based on the lowest weighted asdf data value
    
    int indexOfPeriod = 0;
    SampleType confidence = weightedCandidateConfidence.getUnchecked(0);
    
    for (int c = 1; c < periodCandidatesSize; ++c)
    {
        const SampleType current = weightedCandidateConfidence.getUnchecked(c);
        
        if (current < confidence)
        {
            indexOfPeriod = c;
            confidence = current;
        }
    }
    
    return foundThePeriod (asdfData, periodCandidates.getUnchecked(indexOfPeriod), asdfDataSize);
};


template<typename SampleType>
void PitchDetector<SampleType>::getNextBestPeriodCandidate (juce::Array<int>& candidates,
                                                            const SampleType* asdfData,
                                                            const int dataSize)
{
    int initIndex;
    
    if (! candidates.contains(0))
        initIndex = 0;
    else
    {
        initIndex = -1;
        
        for (int i = 1; i < dataSize; ++i)
        {
            if (! candidates.contains(i))
            {
                initIndex = i;
                break;
            }
        }
        
        if (initIndex == -1)
            return;
    }
    
    SampleType min = asdfData[initIndex];
    int minIndex = initIndex;
    
    for (int i = 0; i < dataSize; ++i)
    {
        if (i == initIndex)
            continue;
        
        if (candidates.contains(i))
            continue;
        
        const SampleType current = asdfData[i];
        
        if (current == 0.0)
        {
            candidates.add (i);
            return;
        }
        
        if (current < min)
        {
            min = current;
            minIndex = i;
        }
    }
    
    candidates.add (minIndex);
};


template<typename SampleType>
int PitchDetector<SampleType>::samplesToFirstZeroCrossing (const SampleType* inputAudio, const int numInputSamples)
{
    if (inputAudio[0] == 0.0)
        return 0;
    
    const bool startedPositive = inputAudio[0] > 0.0;
    
    for (int s = 1; s < floor (numInputSamples / 2.0f); ++s)
    {
        const auto currentSample = inputAudio[s];
        
        if (currentSample == 0.0)
            return s;
        
        const bool isNowPositive = currentSample > 0.0;
        
        if (startedPositive != isNowPositive)
            return s;
    }
    
    return 0;
};



template<typename SampleType>
int PitchDetector<SampleType>::indexOfMinElement (const SampleType* data, const int dataSize)
{
    // find minimum of ASDF output data - indicating that index (lag value) to be the period
    
    SampleType min = data[0];
    
    if (min == 0.0)
        return 0;
    
    int minIndex = 0;
    
    for (int n = 1; n < dataSize; ++n)
    {
        const SampleType current = data[n];
        
        if (current == 0.0)
            return n;
        
        if (current < min)
        {
            min = current;
            minIndex = n;
        }
    }
    
    return minIndex;
};


template<typename SampleType>
SampleType PitchDetector<SampleType>::quadraticPeakPosition (const SampleType* data, const int pos, const int dataSize) noexcept
{
    if ((pos == 0) || ((pos + 1) >= dataSize)) // edge of data, can't interpolate
        return static_cast<SampleType> (pos);
    
    const auto posData = data[pos];
    
    if (posData == 0)
        return static_cast<SampleType> (pos);
    
    const auto s0 = data[pos - 1];
    const auto s2 = data[pos + 1];
    
    return pos + SampleType(0.5) * (s2 - s0) / (SampleType(2.0) * posData - s2 - s0);
};


template class PitchDetector<float>;
template class PitchDetector<double>;
