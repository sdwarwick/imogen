
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


#define bvh_NUM_SYNTHESIS_GRAINS 48  // these are cheap, no reason not to have a lot


// multiplicative smoothing cannot ever actually reach 0
#define bvhv_MIN_SMOOTHED_GAIN 0.0000001
#define _SMOOTHING_ZERO_CHECK(inputGain) std::max(SampleType(bvhv_MIN_SMOOTHED_GAIN), SampleType (inputGain))


namespace bav
{
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), parent(h)
{
    nextSynthesisIndex = 0;
}


template<typename SampleType>
void HarmonizerVoice<SampleType>::prepared (const int blocksize)
{
    jassert (blocksize > 0);

    while (synthesisGrains.size() < bvh_NUM_SYNTHESIS_GRAINS)
        synthesisGrains.add (new Synthesis_Grain());
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    nextSynthesisIndex = 0;
    lastPeriod = 0;
    
    synthesisGrains.clear();
}

    
template<typename SampleType>
void HarmonizerVoice<SampleType>::bypassedBlockRecieved (int numSamples)
{
    const auto period = lastPeriod > 0 ? lastPeriod : 250;
    
    for (int i = 0; i < numSamples; ++i)
        getNextSample (period);
}
        

template<typename SampleType>
void HarmonizerVoice<SampleType>::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int origStartSample)
{
    juce::ignoreUnused (origStartSample);
    jassert (desiredFrequency > 0 && currentSamplerate > 0);
    
    auto* writing = output.getWritePointer(0);

    const auto newPeriod = juce::roundToInt (currentSamplerate / desiredFrequency);
    
    for (int s = 0; s < output.getNumSamples(); ++s)
    {
        writing[s] = getNextSample (newPeriod);
    }
}
    

template<typename SampleType>
inline SampleType HarmonizerVoice<SampleType>::getNextSample (const int newPeriod)
{
    jassert (newPeriod > 0);
    
    lastPeriod = newPeriod;
    
    if (! anyGrainsAreActive())
    {
        nextSynthesisIndex = 0;
        startNewGrain (newPeriod);
    }
    
    jassert (anyGrainsAreActive());
    
    auto sample = SampleType(0);
    
    for (auto* grain : synthesisGrains)
    {
        if (! grain->isActive())
            continue;

        sample += grain->getNextSample();
        
        if (! anyGrainsAreActive())
        {
            nextSynthesisIndex = 0;
            startNewGrain (newPeriod);
            continue;
        }
        
        const auto numLeft = grain->samplesLeft();
        
        if (numLeft == newPeriod || numLeft == grain->halfwayIndex())
            startNewGrain (newPeriod);
    }
    
    if (nextSynthesisIndex > 0)
        --nextSynthesisIndex;
    
    return sample;
}
    

// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
template<typename SampleType>
void HarmonizerVoice<SampleType>::noteCleared()
{
    nextSynthesisIndex = 0;
    lastPeriod = 0;
    
    for (auto* grain : synthesisGrains)
        if (grain->isActive())
            grain->stop();
}

    
#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
#undef bvh_NUM_SYNTHESIS_GRAINS
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
