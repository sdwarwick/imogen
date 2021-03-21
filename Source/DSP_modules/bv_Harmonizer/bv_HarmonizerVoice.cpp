/*
    Part of module: bv_Harmonizer
    Parent file: bv_Harmonizer.h
    Classes: HarmonizerVoice
*/


#include "bv_Harmonizer.h"


// multiplicative smoothing cannot ever actually reach 0
#define bvhv_MIN_SMOOTHED_GAIN 0.0000001
#define _SMOOTHING_ZERO_CHECK(inputGain) std::max(SampleType(bvhv_MIN_SMOOTHED_GAIN), SampleType (inputGain))


namespace bav
{
    

template<typename SampleType>
    HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): Base(h), synthesisIndex(0)
{
    
}


    
#undef bvhv_VOID_TEMPLATE
#define bvhv_VOID_TEMPLATE template<typename SampleType> void HarmonizerVoice<SampleType>
    
#undef bvbv_INLINE_VOID_TEMPLATE
#define bvbv_INLINE_VOID_TEMPLATE template<typename SampleType> inline void HarmonizerVoice<SampleType>
    
    
bvhv_VOID_TEMPLATE::prepare (const int blocksize)
{
    // blocksize = maximum period
    
    jassert (blocksize > 0);
    
    const int doubleSize = blocksize * 2;
    
    constexpr SampleType zero = SampleType(0.0);
    
    synthesisBuffer.setSize (1, doubleSize);
    FVO::fill (synthesisBuffer.getWritePointer(0), zero, doubleSize);
    
    windowingBuffer.setSize (1, doubleSize);
    FVO::fill (windowingBuffer.getWritePointer(0), zero, doubleSize);
    
    copyingBuffer.setSize (1, doubleSize);
    FVO::fill (copyingBuffer.getWritePointer(0), zero, doubleSize);
    
    synthesisIndex = 0;
    
    Base::resetRampedValues (blocksize);
}

    

template<typename SampleType>
void HarmonizerVoice<SampleType>::released()
{
    synthesisBuffer.setSize (0, 0, false, false, false);
    windowingBuffer.setSize (0, 0, false, false, false);
    copyingBuffer.setSize (0, 0, false, false, false);
}


bvhv_VOID_TEMPLATE::renderNextBlock (const AudioBuffer& inputAudio, AudioBuffer& outputBuffer,
                                     const int origPeriod,
                                     const juce::Array<int>& indicesOfGrainOnsets,
                                     const AudioBuffer& windowToUse)
{
    jassert (inputAudio.getNumSamples() == outputBuffer.getNumSamples());
    
    // determine if the voice is currently active
    const bool voiceIsOnRightNow = Base::isQuickFading ? Base::quickRelease.isActive()
                                                       : ( Base::parent->isADSRon() ? Base::adsr.isActive() : ! Base::noteTurnedOff );
    if (! voiceIsOnRightNow)
    {
        Base::clearCurrentNote();
        return;
    }
    
    jassert (Base::currentOutputFreq > 0);
    
    const int numSamples = inputAudio.getNumSamples();
    
    // puts shifted samples into the synthesisBuffer
    sola (inputAudio.getReadPointer(0), numSamples, origPeriod,
          juce::roundToInt (Base::parent->getSamplerate() / Base::currentOutputFreq),  // new desired period, in samples
          indicesOfGrainOnsets, windowToUse.getReadPointer(0));
    
    //  smoothed gain modulations
    Base::midiVelocityGain.applyGain (synthesisBuffer, numSamples);
    Base::softPedalGain.applyGain (synthesisBuffer, numSamples);
    Base::playingButReleasedGain.applyGain (synthesisBuffer, numSamples);
    Base::aftertouchGain.applyGain (synthesisBuffer, numSamples);
    
    //  ADSR
    if (Base::parent->isADSRon())
        Base::adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // midi-triggered adsr envelope
    else
        Base::quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes if adsr is off

    if (Base::isQuickFading)  // quick fade out for stopNote w/ no tail off, to prevent clicks from jumping to 0
        Base::quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);

    //  write to output and apply panning
    outputBuffer.copyFrom (0, 0, synthesisBuffer, 0, 0, numSamples);
    outputBuffer.copyFrom (1, 0, synthesisBuffer, 0, 0, numSamples);
    Base::outputLeftGain.applyGain (outputBuffer.getWritePointer(0), numSamples);
    Base::outputRightGain.applyGain (outputBuffer.getWritePointer(1), numSamples);
    
    moveUpSamples (numSamples);
}


bvbv_INLINE_VOID_TEMPLATE::sola (const SampleType* input, const int totalNumInputSamples,
                                 const int origPeriod, // size of analysis grains = origPeriod * 2
                                 const int newPeriod,  // OLA hop size
                                 const juce::Array<int>& indicesOfGrainOnsets, // sample indices marking the beginning of each analysis grain
                                 const SampleType* window) // Hanning window, length origPeriod * 2
{
    if (synthesisIndex > totalNumInputSamples)
        return;
    
    const int analysisGrainLength = 2 * origPeriod; // length of the analysis grains & the pre-computed Hanning window
    
    for (int grainStart : indicesOfGrainOnsets)
    {
        const int grainEnd = grainStart + analysisGrainLength;
        
        if (grainEnd >= totalNumInputSamples)
            break;
        
        if (synthesisIndex > grainEnd)
            continue;
        
        olaFrame (input, grainStart, grainEnd, analysisGrainLength, window, newPeriod);
    }
    
    // fill from the last written sample to the end of the buffer with zeroes
    const int sampsLeft = synthesisBuffer.getNumSamples() - synthesisIndex;
    
    if (sampsLeft > 0)
        FVO::fill (synthesisBuffer.getWritePointer(0) + synthesisIndex, SampleType(0.0), sampsLeft);
}


bvbv_INLINE_VOID_TEMPLATE::olaFrame (const SampleType* inputAudio,
                                     const int frameStartSample, const int frameEndSample, const int frameSize,
                                     const SampleType* window,
                                     const int newPeriod)
{
    // apply the window before OLAing. Writes windowed input samples into the windowingBuffer
    FVO::multiply (windowingBuffer.getWritePointer(0), window,
                   inputAudio + frameStartSample, frameSize);
    
    const SampleType* windowBufferReading = windowingBuffer.getReadPointer(0);
    SampleType* synthesisBufferWriting = synthesisBuffer.getWritePointer(0);
    
    do {
        jassert (synthesisIndex + frameSize < synthesisBuffer.getNumSamples());
        
        FVO::add (synthesisBufferWriting + synthesisIndex, windowBufferReading, frameSize);
        
        synthesisIndex += newPeriod;
    }
    while (synthesisIndex < frameEndSample);
}


bvbv_INLINE_VOID_TEMPLATE::moveUpSamples (const int numSamplesUsed)
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
bvbv_INLINE_VOID_TEMPLATE::noteCleared()
{
    synthesisBuffer.clear();
}

    
/*+++++++++++++++++++++++++++++++++++++
 DANGER!!!
 FOR NON REAL TIME ONLY!!!!!!!!
 ++++++++++++++++++++++++++++++++++++++*/
bvhv_VOID_TEMPLATE::increaseBufferSizes(const int newMaxBlocksize)
{
    synthesisBuffer.setSize(1, newMaxBlocksize, true, true, true);
}


#undef bvhv_VOID_TEMPLATE
#undef bvbv_INLINE_VOID_TEMPLATE
#undef bvhv_MIN_SMOOTHED_GAIN
#undef _SMOOTHING_ZERO_CHECK
    
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
