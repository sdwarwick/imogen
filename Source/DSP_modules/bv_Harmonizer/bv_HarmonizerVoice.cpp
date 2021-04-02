
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
    
    grainSize = endSample - startSample;
    
    halfwayIndex = juce::roundToInt (floor (startSample + (grainSize * 0.5f)));
    
    zeroesLeft = synthesisMarker - startSample;  // grains can only be respaced by being "placed" into the future...
    
    jassert (grainSize > 0);
}
    
    
/* returns true if this sample is the halfway-point for this grain, and should trigger another grain being activated. */
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::getNextSampleIndex (int& origSampleIndex, SampleType& windowValue, int& samplesLeftInGrain)
{
    if (zeroesLeft > 0 || grainSize < 1)
    {
        origSampleIndex = -1;
        windowValue = 0;
        --zeroesLeft;
        samplesLeftInGrain = grainSize + std::max(zeroesLeft, 0);
        if (grainSize < 1) currentlyActive = false;
    }
    
    jassert (readingIndex >= origStartSample && readingIndex <= origEndSample);
    
    origSampleIndex = readingIndex;
    
    const auto windowIndex = readingIndex - origStartSample;
    jassert (windowIndex >= 0 && windowIndex < grainSize);
    windowValue = getWindowValue (grainSize, windowIndex);
    
    ++readingIndex;
    
    samplesLeftInGrain = origEndSample - readingIndex;
    
    if (samplesLeftInGrain <= 0)
    {
        clear();
        samplesLeftInGrain = 0;
    }
}
    
    
template<typename SampleType>
inline SampleType HarmonizerVoice<SampleType>::Grain::getWindowValue (int windowSize, int index)
{
    auto cos2 = std::cos (static_cast<SampleType> (2 * index)
                * juce::MathConstants<SampleType>::pi / static_cast<SampleType> (windowSize - 1));
    
    return static_cast<SampleType> (0.5 - 0.5 * cos2);
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
    int o, l;
    SampleType w;
    getNextSampleIndex (o, w, l);
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::Grain::clear()
{
    currentlyActive = false;
    origStartSample = 0;
    origEndSample = 0;
    readingIndex = 0;
    zeroesLeft = 0;
    grainSize = 0;
}
    
    

/*=====================================================================================================================================================
======================================================================================================================================================*/


template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), parent(h)
{
    nextFramesPeriod = 0;
}


template<typename SampleType>
void HarmonizerVoice<SampleType>::prepared (const int blocksize)
{
    jassert (blocksize > 0);
    
    while (grains.size() < bvh_NUM_SYNTHESIS_GRAINS)
        grains.add (new Grain());
    
    nextSynthesisIndex = 0;
    lastUsedGrainInArray = 0;
    nextFramesPeriod = 0;
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    for (auto* grain : grains)
        grain->clear();
    
    nextSynthesisIndex = 0;
    lastUsedGrainInArray = 0;
    nextFramesPeriod = 0;
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
    juce::ignoreUnused (origStartSample);
    jassert (desiredFrequency > 0 && currentSamplerate > 0);
    
    const auto origPeriod = nextFramesPeriod;
    jassert (origPeriod > 0);
    
    const auto grainSize = origPeriod * 2;
    
    const auto* reading = parent->inputStorage.getReadPointer(0);
    auto* writing = output.getWritePointer(0);
    
    const auto newPeriod = juce::roundToInt (currentSamplerate / desiredFrequency);
    jassert (newPeriod > 0);
    const auto scaleFactor = (float(newPeriod) / float(origPeriod));
    const auto synthesisHopSize = juce::roundToInt (scaleFactor * origPeriod);
    
    for (int s = 0; s < output.getNumSamples(); ++s)
    {
        writing[s] = getNextSample (reading, grainSize, origPeriod, synthesisHopSize);
    }
}
    

template<typename SampleType>
    inline SampleType HarmonizerVoice<SampleType>::getNextSample (const SampleType* inputSamples,
                                                                  const int grainSize, const int halfGrainSize,
                                                                  const int synthesisHopSize)  // synthesisHopSize = scale factor * orig period
{
    if (! anyGrainsAreActive())
        startNewGrain (grainSize, synthesisHopSize);
    
    jassert (grainSize > 0 && halfGrainSize > 0 && synthesisHopSize > 0);
    
    SampleType sample = 0;
    
    for (auto* grain : grains)
    {
        if (! grain->isActive())
            continue;
        
        int sIdx = -1;
        SampleType window = 0;
        int sampsLeft = 0;
        
        grain->getNextSampleIndex (sIdx, window, sampsLeft);
        
        if (sIdx == -1)
            continue;
        
        jassert (window >= SampleType(-1) && window <= SampleType(1));
        
        sample += (inputSamples[sIdx] * window);
        
        if (sampsLeft == halfGrainSize)
            startNewGrain (grainSize, synthesisHopSize);
    }
    
    return sample;
}
    
    
template<typename SampleType>
inline void HarmonizerVoice<SampleType>::startNewGrain (const int grainSize, const int synthesisHopSize) // synthesisHopSize = scale factor * orig period
{
    if (auto* newGrain = getAvailableGrain())
    {
        const auto arraySize = parent->indicesOfGrainOnsets.size();
        
        jassert (lastUsedGrainInArray >= 0 && lastUsedGrainInArray < arraySize);
        
        const auto grainStart = parent->indicesOfGrainOnsets.getUnchecked (lastUsedGrainInArray);
        
        ++lastUsedGrainInArray;
        
        if (lastUsedGrainInArray >= arraySize)
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
    nextFramesPeriod = 0;
    
    for (auto* grain : grains)
        grain->clear();
}

    
#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
#undef bvh_NUM_SYNTHESIS_GRAINS
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
