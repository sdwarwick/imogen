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
    // calculate ASDF for all lags k in range minPeriod, maxPeriod
    // in the ASDF buffer, the value stored at index 0 is the ASDF for lag minPeriod.
    // the value stored at the maximum index is the ASDF for lag maxPeriod.
    
    const int numSamples = inputAudio.getNumSamples();
    
    if (numSamples == 0)
        return -1.0f;
        
    if (numSamples < minPeriod)
        return -1.0f;
    
    const int middleIndex = floor (numSamples / 2);
    
    int highestAsdfIndex; // highest sample index written to in asdfBuffer
    
    for (int k = minPeriod; k <= maxPeriod; ++k) // the difference function
    {
        const int sampleOffset = floor ((numSamples + k) / 2);
        
        const auto* r = inputAudio.getReadPointer(0);
        
        SampleType runningSum = 0;
        
        for (int n = 0; n < numSamples - 1; ++n)
        {
            const int startingSample = n + middleIndex - sampleOffset;
            
            const SampleType difference = r[startingSample] - r[startingSample + k];
            
            runningSum += (difference * difference);
        }
        
        const int asdfBufferIndex = k - minPeriod;
        
        jassert (isPositiveAndBelow (asdfBufferIndex, asdfBuffer.getNumSamples()));
        
        asdfBuffer.setSample (0,
                              asdfBufferIndex,
                              runningSum / numSamples);
        
        highestAsdfIndex = asdfBufferIndex;
    }
    
    
    // apply a weighting function to the calculated ASDF values...
    
    
    // find correct minimum of the ASDF
    
    const SampleType* r = asdfBuffer.getReadPointer(0);
    
    SampleType asdfMinimum = r[0]; // the ASDF value corresponding to the lag value
    int minK = 0;    // the actual lag value (this is the period!)
    
    for (int n = 1; n <= highestAsdfIndex; ++n)
    {
        const SampleType currentSample = r[n];
        
        if (currentSample < asdfMinimum)
        {
            asdfMinimum = currentSample;
            minK = n;
        }
    }
    
    
    // period = minK + minPeriod
    // pitch confidence = asdfMinimum
    
    // if asdfMinimum is too high, then we have a low pitch confidence & determine the frame to be unpitched
    if (asdfMinimum > confidenceThresh)
        return -1.0f;
    
    // quadratic interpolation to find accurate float period from integer period
    SampleType realPeriod = quadraticPeakPosition (asdfBuffer.getReadPointer(0), minK, highestAsdfIndex);
    
    realPeriod += minPeriod; // account for offset in ASDF buffer - index 0 held the value for lag minPeriod
    
    return samplerate / realPeriod;
};


template<typename SampleType>
SampleType PitchDetector<SampleType>::quadraticPeakPosition (const SampleType* data, unsigned int pos, const int dataSize) noexcept
{
    const unsigned int x0 = (pos == 0)           ? pos     : pos - 1;
    const unsigned int x2 = (pos + 1 < dataSize) ? pos + 1 : pos;
    
    if (x0 == pos)
        return (data[pos] <= data[x2]) ? pos : x2;
    
    if (x2 == pos)
        return (data[pos] <= data[x0]) ? pos : x0;
    
    const auto s0 = data[x0];
    const auto s2 = data[x2];
    
    return pos + 0.5 * (s0 - s2) / (s0 - 2.0 * data[pos] + s2);
};


template class PitchDetector<float>;
template class PitchDetector<double>;
