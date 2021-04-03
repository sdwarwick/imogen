
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


#define bvh_NUM_SYNTHESIS_GRAINS 12


// multiplicative smoothing cannot ever actually reach 0
#define bvhv_MIN_SMOOTHED_GAIN 0.0000001
#define _SMOOTHING_ZERO_CHECK(inputGain) std::max(SampleType(bvhv_MIN_SMOOTHED_GAIN), SampleType (inputGain))


namespace bav
{
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), parent(h)
{
    nextFramesPeriod = 0;
    nextSynthesisIndex = 0;
}


template<typename SampleType>
void HarmonizerVoice<SampleType>::prepared (const int blocksize)
{
    jassert (blocksize > 0);

    while (synthesisGrains.size() < bvh_NUM_SYNTHESIS_GRAINS)
        synthesisGrains.add (new SynthesisGrain());
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    nextSynthesisIndex = 0;
    nextFramesPeriod = 0;
}
    

template<typename SampleType>
void HarmonizerVoice<SampleType>::dataAnalyzed (const int period)
{
    jassert (period > 0);
    nextFramesPeriod = period;
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::bypassedBlockRecieved (int numSamples)
{
    nextSynthesisIndex = std::max (0, nextSynthesisIndex - numSamples);
}
        

template<typename SampleType>
void HarmonizerVoice<SampleType>::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int origStartSample)
{
    juce::ignoreUnused (origStartSample);
    jassert (desiredFrequency > 0 && currentSamplerate > 0);
    
    const auto origPeriod = nextFramesPeriod;
    jassert (origPeriod > 0);

    auto* writing = output.getWritePointer(0);

    const auto newPeriod = juce::roundToInt (currentSamplerate / desiredFrequency);
//    const auto scaleFactor = (float(newPeriod) / float(origPeriod));
//    const auto synthesisHopSize = juce::roundToInt (scaleFactor * origPeriod);
    
    const auto numSamples = output.getNumSamples();
    
    for (int s = 0; s < numSamples; ++s)
    {
        writing[s] = getNextSample (origPeriod, newPeriod);
    }
    
    nextSynthesisIndex = std::max (0, nextSynthesisIndex - numSamples);
}
    

template<typename SampleType>
inline SampleType HarmonizerVoice<SampleType>::getNextSample (const int halfGrainSize, const int newPeriod)
{
    if (! anyGrainsAreActive())
        startNewGrain (newPeriod);
    
    jassert (anyGrainsAreActive());
    jassert (halfGrainSize > 0 && newPeriod > 0);
    
    SampleType sample = 0;
    
    for (auto* grain : synthesisGrains)
    {
        if (! grain->isActive())
            continue;

        sample += grain->getNextSample();
        
        if (grain->samplesLeft() == halfGrainSize)
            startNewGrain (newPeriod);
    }
    
    return sample;
}
    
    
template<typename SampleType>
inline void HarmonizerVoice<SampleType>::startNewGrain (const int newPeriod)
{
    if (auto* newGrain = getAvailableGrain())
    {
        if (auto* analysisGrain = parent->findClosestGrain (nextSynthesisIndex))
        {
            newGrain->startNewGrain (analysisGrain, nextSynthesisIndex);
            nextSynthesisIndex += newPeriod;
        }
    }
}
    

// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
template<typename SampleType>
void HarmonizerVoice<SampleType>::noteCleared()
{
    nextSynthesisIndex = 0;
}

    
#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
#undef bvh_NUM_SYNTHESIS_GRAINS
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
