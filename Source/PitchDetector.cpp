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
    
    if ((minHz != newMinHz) || allowRecalc)
        maxPeriod = roundToInt (1.0f / minHz * samplerate);
    
    if ((maxHz != newMaxHz) || allowRecalc)
        minPeriod = roundToInt (1.0f / maxHz * samplerate);
    
    jassert (maxPeriod > minPeriod);
    
    const int numOfLagValues = maxPeriod - minPeriod;
    
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
    
    for (int k = minPeriod; k <= maxPeriod; ++k)
    {
        const int asdfBufferIndex = k - minPeriod;
    };
    
    
    // find correct minimum peak of ASDF corresponding to the period
    
    // quadratic interpolation to find accurate float period from integer period
    
    // pitch in hz = 1 / period * samplerate
};



template class PitchDetector<float>;
template class PitchDetector<double>;
