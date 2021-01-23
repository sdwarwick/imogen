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
    
    maxPeriod = roundToInt (1.0f / minHz * samplerate);
    minPeriod = roundToInt (1.0f / maxHz * samplerate);
    
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
    
    calculateASDF (inputAudio.getReadPointer(0), numSamples, asdfBuffer.getWritePointer(0));
    
    const int asdfDataSize = maxPeriod - minPeriod + 1; // # of samples written to asdfBuffer
    
    const unsigned int minIndex = indexOfMinElement (asdfBuffer.getReadPointer(0), asdfDataSize);
    
    if (asdfBuffer.getSample (0, minIndex) > confidenceThresh)
        return -1.0f;
    
    // quadratic interpolation to find accurate float period from integer period
    SampleType realPeriod = quadraticPeakPosition (asdfBuffer.getReadPointer(0), minIndex, asdfDataSize);
    
    realPeriod += minPeriod; // account for offset in ASDF buffer - index 0 held the value for lag minPeriod
    
    return (float) (samplerate / realPeriod);
};



template<typename SampleType>
void PitchDetector<SampleType>::calculateASDF (const SampleType* inputAudio, const int numSamples, SampleType* outputData)
{
    // in the ASDF buffer, the value stored at index 0 is the ASDF for lag minPeriod.
    // the value stored at the maximum index is the ASDF for lag maxPeriod.
    
    const int middleIndex = floor (numSamples / 2.0f);
    
    const int minLag = samplesToFirstZeroCrossing (inputAudio, numSamples);
    
    for (int k = minPeriod; k <= maxPeriod; ++k)
    {
        const int index = k - minPeriod; // the actual asdfBuffer index
        
        if (k <= minLag)
        {
            outputData[index] = 1.0;
            continue;
        }
        
        const int sampleOffset = floor ((numSamples + k) / 2.0f);
        
        outputData[index] = 0.0;
        
        for (int n = 0; n < numSamples; ++n)
        {
            const int startingSample = n + middleIndex - sampleOffset;
            
            const SampleType difference = inputAudio[startingSample] - inputAudio[startingSample + k];
            
            outputData[index] += (difference * difference);
        }
        
        outputData[index] /= numSamples; // normalize
    }
};


template<typename SampleType>
unsigned int PitchDetector<SampleType>::samplesToFirstZeroCrossing (const SampleType* inputAudio, const int numSamples)
{
    int analysisStart = 0;
    
    // find first sample not 0
    if (inputAudio[0] == 0)
    {
        int startingSample = 1;
        
        while (startingSample < numSamples)
        {
            if (inputAudio[startingSample] != 0)
            {
                analysisStart = startingSample;
                break;
            }
            ++startingSample;
        }
        
        if (startingSample == numSamples - 1)
            return 0;
    }
    
    int numSamps = 0;
    
    const bool startedPositive = inputAudio[analysisStart] > 0.0;
    
    for (int s = analysisStart + 1; s < numSamples; ++s)
    {
        const auto currentSample = inputAudio[s];
        
        if (currentSample == 0.0)
            continue;
        
        bool isNowPositive = currentSample > 0.0;
        
        if (startedPositive != isNowPositive)
        {
            numSamps = s + 1;
            break;
        }
    }
    
    if (numSamps == numSamples)
        return 0;
    
    return numSamps;
};


template<typename SampleType>
unsigned int PitchDetector<SampleType>::indexOfMinElement (const SampleType* data, const int dataSize)
{
    // find minimum of ASDF output data - indicating that index (lag value) to be the period
    
    SampleType min = data[0];
    
    if (min == 0)
        return 0;
    
    unsigned int minIndex = 0;
    
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
