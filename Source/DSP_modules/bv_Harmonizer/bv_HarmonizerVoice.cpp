
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
 
 bv_HarmonizerVoice.cpp: This file defines implementation details for the HarmonizerVoice class. HarmonizerVoice essentially represents a single synthesizer voice that generates sound by pitch shifting an audio input. The voice's parent Harmonizer object performs the analysis of the input signal, so that analysis only needs to be done once no matter how many voices the Harmonizer has.
 
======================================================================================================================================================*/


#include "bv_Harmonizer.h"


#define bvh_NUM_SYNTHESIS_GRAINS 32  // these are cheap, no reason not to have a lot


// multiplicative smoothing cannot ever actually reach 0
#define bvhv_MIN_SMOOTHED_GAIN 0.0000001
#define _SMOOTHING_ZERO_CHECK(inputGain) std::max(SampleType(bvhv_MIN_SMOOTHED_GAIN), SampleType (inputGain))


namespace bav
{
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::storeNewGrain (const int startSample, const int endSample, const int synthesisMarker)
{
    currentlyActive = true;
    origStartSample = startSample;
    origEndSample = endSample;
    readingIndex = startSample;
    
    halfwayIndex = juce::roundToInt (floor (origStartSample + ((origEndSample - origStartSample) * 0.5f)));
    
    zeroesLeft = synthesisMarker - startSample;
}
    
    
/* returns true if this sample is the halfway-point for this grain, and should trigger another grain being activated. */
template<typename SampleType>
bool HarmonizerVoice<SampleType>::Grain::getNextSampleIndex (int& origSampleIndex, int& windowIndex)
{
    if (zeroesLeft > 0)
    {
        origSampleIndex = -1;
        windowIndex = -1;
        --zeroesLeft;
        return false;
    }
    
    origSampleIndex = readingIndex;
    windowIndex = readingIndex - origStartSample;
    
    const bool triggerNewGrain = readingIndex == halfwayIndex;
    
    ++readingIndex;
    
    if (readingIndex >= origEndSample)
    {
        clear();
        return false;
    }
    else
    {
        return triggerNewGrain;
    }
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::skipSamples (int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        skipSample();
}

    
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::skipSample()
{
    int o, w;
    getNextSampleIndex (o, w);
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::clear()
{
    currentlyActive = false;
    origStartSample = 0;
    origEndSample = 0;
    readingIndex = 0;
    zeroesLeft = 0;
}
    
    

/*
*/


template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), parent(h)
{
    
}


template<typename SampleType>
void HarmonizerVoice<SampleType>::prepared (const int blocksize)
{
    jassert (blocksize > 0);
    
    while (grains.size() < bvh_NUM_SYNTHESIS_GRAINS)
        grains.add (new Grain());
    
    nextSynthesisIndex = 0;
    lastUsedGrainInArray = 0;
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    for (auto* grain : grains)
        grain->clear();
}
    

template<typename SampleType>
void HarmonizerVoice<SampleType>::dataAnalyzed (const int period)
{
    jassert (period > 0);
    nextFramesPeriod = period;
    
    lastUsedGrainInArray = 0;
    nextSynthesisIndex = 0;
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::bypassedBlockRecieved (int numSamples)
{
    for (auto* grain : grains)
        grain->skipSamples (numSamples);
}
        

template<typename SampleType>
void HarmonizerVoice<SampleType>::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int origStartSample)
{
    juce::ignoreUnused(origStartSample);
    
    const auto* reading = parent->inputStorage.getReadPointer(0);
    auto* writing = output.getWritePointer(0);
    const auto* window = parent->windowBuffer.getReadPointer(0);
    
    jassert (desiredFrequency > 0 && currentSamplerate > 0);
    const int newPeriod = juce::roundToInt (currentSamplerate / desiredFrequency);
    const auto scaleFactor = float (newPeriod / nextFramesPeriod);
    const int synthesisHopSize = juce::roundToInt (scaleFactor * nextFramesPeriod);
    
    const int grainSize = nextFramesPeriod * 2;
    
    for (int s = 0; s < output.getNumSamples(); ++s)
    {
        writing[s] = getNextSample (reading, window, grainSize, synthesisHopSize);
    }
}
    

template<typename SampleType>
inline SampleType HarmonizerVoice<SampleType>::getNextSample (const SampleType* inputSamples,
                                                              const SampleType* window,
                                                              const int grainSize,
                                                              const int synthesisHopSize)  // synthesisHopSize = scale factor * orig period
{
    if (! anyGrainsAreActive())
        startNewGrain (grainSize, synthesisHopSize);
    
    SampleType sample = 0;
    
    for (auto* grain : grains)
    {
        if (! grain->isActive())
            continue;
        
        int sIdx = -1, wIdx = -1;
        
        bool shouldStartNewGrain = grain->getNextSampleIndex (sIdx, wIdx);
        
        if (sIdx == -1 || wIdx == -1)
            continue;
        
        jassert (sIdx >= 0 && wIdx >= 0 && wIdx <= grainSize);
        
        sample += inputSamples[sIdx] * window[wIdx];
        
        if (shouldStartNewGrain)
            startNewGrain (grainSize, synthesisHopSize);
    }
    
    return sample;
}
    
    
template<typename SampleType>
inline void HarmonizerVoice<SampleType>::startNewGrain (const int grainSize, const int synthesisHopSize) // synthesisHopSize = scale factor * orig period
{
    if (auto* newGrain = getAvailableGrain())
    {
        const int grainStart = parent->indicesOfGrainOnsets.getUnchecked (lastUsedGrainInArray);
        
        ++lastUsedGrainInArray;
        
        if (lastUsedGrainInArray >= parent->indicesOfGrainOnsets.size())
            lastUsedGrainInArray = 0;
        
        newGrain->storeNewGrain (grainStart, grainStart + grainSize, nextSynthesisIndex);
        
        nextSynthesisIndex += synthesisHopSize;
    }
}
    

// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
template<typename SampleType>
void HarmonizerVoice<SampleType>::noteCleared()
{
    lastUsedGrainInArray = 0;
    nextSynthesisIndex = 0;
    
    for (auto* grain : grains)
        grain->clear();
}

    
#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
#undef bvh_NUM_SYNTHESIS_GRAINS
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
