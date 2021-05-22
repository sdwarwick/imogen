
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


#include "Harmonizer.h"


namespace bav
{
template < typename SampleType >
HarmonizerVoice< SampleType >::HarmonizerVoice (Harmonizer< SampleType >* h)
    : dsp::SynthVoiceBase< SampleType > (h)
    , parent (h)
    , shifter (&parent->analyzer)
{
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::prepared (const int blocksize)
{
    juce::ignoreUnused (blocksize);
    shifter.prepare();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::released()
{
    shifter.releaseResources();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::noteCleared()
{
    shifter.reset();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::newBlockComing (int previousBlocksize, int upcomingBlocksize)
{
    juce::ignoreUnused (upcomingBlocksize);
    shifter.newBlockComing (previousBlocksize);
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int startSampleOfOrigBuffer)
{
    jassert (desiredFrequency > 0 && currentSamplerate > 0);
    juce::ignoreUnused (startSampleOfOrigBuffer);

//    shifter.getSamples (output.getWritePointer (0),
//                        output.getNumSamples(),
//                        juce::roundToInt (currentSamplerate / desiredFrequency), // desired period
//                        parent->getCurrentPeriod());
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::bypassedBlockRecieved (float voicesLastOutputFreq, double currentSamplerate, int numSamples)
{
    juce::ignoreUnused (voicesLastOutputFreq, currentSamplerate, numSamples);
    shifter.bypassedBlockRecieved (numSamples);
}


template class HarmonizerVoice< float >;
template class HarmonizerVoice< double >;


} // namespace bav
