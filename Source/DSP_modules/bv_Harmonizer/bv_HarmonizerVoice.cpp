
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


// multiplicative smoothing cannot ever actually reach 0
#define bvhv_MIN_SMOOTHED_GAIN 0.0000001
#define _SMOOTHING_ZERO_CHECK(inputGain) std::max(SampleType(bvhv_MIN_SMOOTHED_GAIN), SampleType (inputGain))


namespace bav
{
    

template<typename SampleType>
    HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), parent(h), synthesisIndex(0)
{
    
}


template<typename SampleType>
void HarmonizerVoice<SampleType>::prepare (const int blocksize)
{
    // blocksize = maximum period
    
    jassert (blocksize > 0);
    
    const int doubleSize = blocksize * 2;
    
    synthesisBuffer.setSize (1, doubleSize);
    FVO::fill (synthesisBuffer.getWritePointer(0), SampleType(0.0), doubleSize);
    
    copyingBuffer.setSize (1, doubleSize);
    FVO::fill (copyingBuffer.getWritePointer(0), SampleType(0.0), doubleSize);
    
    synthesisIndex = 0;
    lastUsedGrainInArray = 0;
    
    grain1.clear();
    grain2.clear();
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    synthesisBuffer.setSize (0, 0, false, false, false);
    copyingBuffer.setSize (0, 0, false, false, false);
    grain1.clear();
    grain2.clear();
}
    

template<typename SampleType>
void HarmonizerVoice<SampleType>::dataAnalyzed (const int period)
{
    jassert (period > 0);
    nextFramesPeriod = period;
    
    lastUsedGrainInArray = 0;
}
    
    
template<typename SampleType>
void HarmonizerVoice<SampleType>::bypassedBlockRecieved (int numSamples)
{
    moveUpSamples (numSamples);
    grain1.skipSamples (numSamples);
    grain2.skipSamples (numSamples);
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
    const int origPeriodTimesScaleFactor = juce::roundToInt (scaleFactor * nextFramesPeriod);
    
    const int grainSize = nextFramesPeriod * 2;
    
    for (int s = 0; s < output.getNumSamples(); ++s)
    {
        writing[s] = getNextSample (reading, window, grainSize, origPeriodTimesScaleFactor);
    }
}
    

template<typename SampleType>
inline SampleType HarmonizerVoice<SampleType>::getNextSample (const SampleType* inputSamples,
                                                              const SampleType* window,
                                                              const int grainSize, const int origPeriodTimesScaleFactor)
{
    if (grain1.isDone())
        storeNewGrain (grain1, inputSamples, grainSize, window, origPeriodTimesScaleFactor);
    
    if (grain2.isDone())
        storeNewGrain (grain2, inputSamples, grainSize, window, origPeriodTimesScaleFactor);
    
    return grain1.getNextSample() + grain2.getNextSample();
}
    

template<typename SampleType>
inline void HarmonizerVoice<SampleType>::storeNewGrain (Grain& grain,
                                                        const SampleType* inputSamples,
                                                        const int grainSize,
                                                        const SampleType* window,
                                                        const int origPeriodTimesScaleFactor)
{
    const int outputStart = std::max(grain1.getLastSynthesisMark(), grain2.getLastSynthesisMark())
                            + origPeriodTimesScaleFactor;
    
    int thisGrainStart;
    
    if (outputStart >= grain.getLastEndIndex() && lastUsedGrainInArray < parent->indicesOfGrainOnsets.size())
    {
        thisGrainStart = parent->indicesOfGrainOnsets.getUnchecked (lastUsedGrainInArray++);
    }
    else
    {
        thisGrainStart = grain.getLastStartIndex();
        lastUsedGrainInArray = 0;
    }
    
    grain.storeNewGrain (inputSamples, thisGrainStart, grainSize, window, outputStart);
}



//template<typename SampleType>
//inline void HarmonizerVoice<SampleType>::sola (const SampleType* input,
//                                               const int totalNumInputSamples,
//                                               const int origPeriod, // size of analysis grains = origPeriod * 2
//                                               const int newPeriod,  // OLA hop size
//                                               const juce::Array<int>& indicesOfGrainOnsets, // sample indices marking the start of each analysis grain
//                                               const SampleType* window) // Hanning window, length origPeriod * 2
//{
//    if (synthesisIndex > totalNumInputSamples)
//        return;
//
//    jassert (origPeriod > 0);
//    jassert (newPeriod > 0);
//    jassert (! indicesOfGrainOnsets.isEmpty());
//    jassert (window != nullptr && input != nullptr);
//
//    const int analysisGrainLength = 2 * origPeriod; // length of the analysis grains & the pre-computed Hanning window
//
//    for (int grainStart : indicesOfGrainOnsets)
//    {
//        const int grainEnd = grainStart + analysisGrainLength;
//
//        if (grainEnd >= totalNumInputSamples)
//            break;
//
//        if (synthesisIndex > grainEnd)
//            continue;
//
//        olaFrame (input, grainStart, grainEnd, analysisGrainLength, window, newPeriod);
//    }
//
//    const int sampsLeft = synthesisBuffer.getNumSamples() - synthesisIndex;
//
//    if (sampsLeft > 0)  // fill from the last written sample to the end of the buffer with zeroes
//        FVO::fill (synthesisBuffer.getWritePointer(0) + synthesisIndex, SampleType(0.0), sampsLeft);
//}
//
//
//template<typename SampleType>
//inline void HarmonizerVoice<SampleType>::olaFrame (const SampleType* inputAudio,
//                                                   const int frameStartSample, const int frameEndSample, const int frameSize,
//                                                   const SampleType* window,
//                                                   const int newPeriod)
//{
//    jassert (frameEndSample - frameStartSample == frameSize);
//
//    // apply the window before OLAing. Writes windowed input samples into the windowingBuffer
//    juce::FloatVectorOperations::multiply (windowingBuffer.getWritePointer(0), window,
//                                           inputAudio + frameStartSample, frameSize);
//
//    const SampleType* windowBufferReading = windowingBuffer.getReadPointer(0);
//    SampleType* synthesisBufferWriting = synthesisBuffer.getWritePointer(0);
//
//    do {
//        jassert (synthesisIndex + frameSize < synthesisBuffer.getNumSamples());
//
//        juce::FloatVectorOperations::add (synthesisBufferWriting + synthesisIndex, windowBufferReading, frameSize);
//
//        synthesisIndex += newPeriod;
//    }
//    while (synthesisIndex < frameEndSample);
//}


template<typename SampleType>
inline void HarmonizerVoice<SampleType>::moveUpSamples (const int numSamplesUsed)
{
    synthesisIndex -= numSamplesUsed;
    
    if (synthesisIndex < 1)
    {
        synthesisBuffer.clear();
        synthesisIndex = 0;
        return;
    }
    
    copyingBuffer.copyFrom (0, 0, synthesisBuffer, 0, numSamplesUsed, synthesisIndex);
    synthesisBuffer.clear();
    synthesisBuffer.copyFrom (0, 0, copyingBuffer, 0, 0, synthesisIndex);
}
    


// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
template<typename SampleType>
void HarmonizerVoice<SampleType>::noteCleared()
{
    synthesisIndex = 0;
    synthesisBuffer.clear();
    copyingBuffer.clear();
    grain1.clear();
    grain2.clear();
    lastUsedGrainInArray = -1;
}

    
/*+++++++++++++++++++++++++++++++++++++
 DANGER!!!
 FOR NON REAL TIME ONLY!!!!!!!!
 ++++++++++++++++++++++++++++++++++++++*/
template<typename SampleType>
void HarmonizerVoice<SampleType>::increaseBufferSizes(const int newMaxBlocksize)
{
    synthesisBuffer.setSize (1, newMaxBlocksize, true, true, true);
    copyingBuffer.setSize (1, newMaxBlocksize, true, true, true);
}


#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
