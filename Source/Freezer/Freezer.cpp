/*
  ==============================================================================

    Freezer.cpp
    Created: 7 Jan 2021 11:18:38pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Freezer.h"


template<typename SampleType>
Freezer<SampleType>::Freezer(): wasFrozen(false), isCleared(true)
{
    
};



template<typename SampleType>
Freezer<SampleType>::~Freezer()
{
    
};


template<typename SampleType>
void Freezer<SampleType>::prepare (const double sampleRate, const int samplesPerBlock, const int numChannels)
{
    isCleared = false;
};


template<typename SampleType>
void Freezer<SampleType>::releaseResources()
{
    freezeBuffer.setSize(0, 0, false, false, false);
    wasFrozen = false;
    isCleared = true;
};


template<typename SampleType>
void Freezer<SampleType>::process (const AudioBuffer<SampleType>& inBuffer, AudioBuffer<SampleType>& outBuffer, const bool isFrozen)
{
    if (! isFrozen)
    {
        outBuffer.makeCopyOf (inBuffer);
        
        if (! isCleared)
            clearFrozenAudio();
        
        wasFrozen = false;
        return;
    }
    
    if (! wasFrozen)
        freezeNewAudio (inBuffer);
    
    renderFrozenAudio (outBuffer);
    
    wasFrozen = true;
};


template<typename SampleType>
void Freezer<SampleType>::freezeNewAudio (const AudioBuffer<SampleType>& input)
{
    freezeBuffer.clear();
    
    isCleared = false;
};


template<typename SampleType>
void Freezer<SampleType>::clearFrozenAudio()
{
    isCleared = true;
};


template<typename SampleType>
void Freezer<SampleType>::renderFrozenAudio (AudioBuffer<SampleType>& outBuffer)
{
    
};



template class Freezer<float>;
template class Freezer<double>;
