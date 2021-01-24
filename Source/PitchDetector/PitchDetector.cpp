/*
  ==============================================================================

    PitchDetector.cpp
    Created: 18 Jan 2021 11:31:33am
    Author:  Ben Vining

  ==============================================================================
*/

#include "PitchDetector.h"


template<typename SampleType>
PitchDetector<SampleType>::PitchDetector(const int minHz, const int maxHz, const double samplerate)
{
    this->minHz = minHz;
    this->maxHz = maxHz;
    this->samplerate = samplerate;
    
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
    
    maxPeriod = roundToInt (samplerate / minHz);
    minPeriod = roundToInt (samplerate / maxHz);
    
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
        SampleType lastHz = samplerate / lastEstimatedPeriod;
        lastEstimatedPeriod = newSamplerate / lastHz;
    }
    
    samplerate = newSamplerate;
    
    if (recalcHzRange)
        setHzRange (minHz, maxHz, true);
};


template<typename SampleType>
float PitchDetector<SampleType>::detectPitch (const AudioBuffer<SampleType>& inputAudio)
{
    // this function should return the pitch in Hz, or -1 if the frame of audio is determined to be unpitched
    
    const int numSamples = inputAudio.getNumSamples();
    
    if (numSamples < minPeriod)
        return -1.0f;
    
    jassert (asdfBuffer.getNumSamples() > (maxPeriod - minPeriod));
    
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    int minLag = samplesToFirstZeroCrossing (reading, numSamples); // little trick to avoid picking too small a period
    int maxLag = maxPeriod;
    
    if (lastFrameWasPitched) // pitch shouldn't halve or double between consecutive voiced frames...
    {
        minLag = std::max (minLag, roundToInt (lastEstimatedPeriod / 2.0));
        maxLag = std::min (maxLag, roundToInt (lastEstimatedPeriod * 2.0));
    }
    
    minLag = std::max (minLag, minPeriod);
    
    if (maxLag < minLag)
        maxLag = minLag + 1;
    else if (minLag == maxLag)
        ++maxLag;
    
    const int middleIndex = floor (numSamples / 2.0f);
    
    SampleType* asdfData = asdfBuffer.getWritePointer(0);
    
    // in the ASDF buffer, the value stored at index 0 is the ASDF for lag minPeriod.
    // the value stored at the maximum index is the ASDF for lag maxPeriod.
    // always write the same datasize to the ASDF buffer (with regard to this member variables), even if the k range is being limited this frame by the minLag & maxLag local variables.
    
    for (int k = minPeriod; // always write the same datasize to asdfBuffer, even if k values are being limited this frame
            k <= maxPeriod;
            ++k)
    {
        const int index = k - minPeriod; // the actual asdfBuffer index for this k value's data
        
        if (k < minLag || k > maxLag) // range compression of k is done here
        {
            asdfData[index] = 2.0;
            continue;
        }
        
        asdfData[index] = 0.0;
        
        const int sampleOffset = floor ((numSamples + k) / 2.0f);
        
        for (int n = 0; n < numSamples; ++n)
        {
            const int startingSample = n + middleIndex - sampleOffset;
            
            const SampleType difference = reading[startingSample] - reading[startingSample + k];
            
            asdfData[index] += (difference * difference);
        }
        
        asdfData[index] /= numSamples; // normalize
        
        if (index > 3 && k - 2 > minLag) // test to see if we've found a good enough match already, so we can stop computing ASDF values...
        {
            const SampleType confidence = asdfData[index - 2];
            
            if (confidence < confidenceThresh
                && confidence < asdfData[index]
                && confidence < asdfData[index - 1]
                && confidence < asdfData[index - 3]
                && confidence < asdfData[index - 4])
                
                return foundThePeriod (asdfData, index - 2, index + 1);
        }
    }
    
    const int asdfDataSize = maxPeriod - minPeriod + 1; // # of samples written to asdfBuffer
    
    const int minIndex = indexOfMinElement (asdfData, asdfDataSize);
    const SampleType confidence = asdfData[minIndex];
    
    if (confidence > confidenceThresh)
    {
        lastFrameWasPitched = false;
        return -1.0f;
    }
    
    return foundThePeriod (asdfData, minIndex, asdfDataSize);
};


template<typename SampleType>
float PitchDetector<SampleType>::foundThePeriod (const SampleType* asdfData,
                                                 const int minIndex,
                                                 const int asdfDataSize)
{
    SampleType realPeriod = quadraticPeakPosition (asdfData, minIndex, asdfDataSize);
    realPeriod += minPeriod;
    lastEstimatedPeriod = realPeriod;
    lastFrameWasPitched = true;
    return (float) (samplerate / realPeriod); // return pitch in hz
};


template<typename SampleType>
unsigned int PitchDetector<SampleType>::samplesToFirstZeroCrossing (const SampleType* inputAudio, const int numInputSamples)
{
    int analysisStart = 0;
    
    // find first sample not 0
    if (inputAudio[0] == 0)
    {
        int startingSample = 1;
        
        while (startingSample < numInputSamples)
        {
            if (inputAudio[startingSample] != 0)
            {
                analysisStart = startingSample;
                break;
            }
            ++startingSample;
        }
        
        if (startingSample == numInputSamples - 1)
            return 0;
    }
    
    const bool startedPositive = inputAudio[analysisStart] > 0.0;
    
    for (int s = analysisStart + 1; s < floor (numInputSamples / 2.0f); ++s)
    {
        const auto currentSample = inputAudio[s];
        
        if (currentSample == 0.0)
            continue;
        
        const bool isNowPositive = currentSample > 0.0;
        
        if (startedPositive != isNowPositive)
            return s + 1;
    }
    
    return 0;
};


template<typename SampleType>
int PitchDetector<SampleType>::indexOfMinElement (const SampleType* data, const int dataSize)
{
    // find minimum of ASDF output data - indicating that index (lag value) to be the period
    
    SampleType min = data[0];
    
    if (min == 0)
        return 0;
    
    int minIndex = 0;
    
    for (int n = 1; n < dataSize; ++n)
    {
        const SampleType current = data[n];
        
        if (current == 0)
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
SampleType PitchDetector<SampleType>::quadraticPeakPosition (const SampleType* data, unsigned int pos, const int dataSize) noexcept
{
    if ((pos == 0) || ((pos + 1) >= dataSize)) // edge of data, can't interpolate
        return pos;
    
    const auto posData = data[pos];
    
    if (posData == 0)
        return pos;
    
    const auto s0 = data[pos - 1];
    const auto s2 = data[pos + 1];
    
    return pos + 0.5 * (s0 - s2) / (s0 - 2.0 * posData + s2);
};


template class PitchDetector<float>;
template class PitchDetector<double>;
