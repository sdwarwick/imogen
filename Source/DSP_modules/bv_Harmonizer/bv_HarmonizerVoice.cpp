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
    
    windowingBuffer.setSize (1, doubleSize);
    FVO::fill (windowingBuffer.getWritePointer(0), SampleType(0.0), doubleSize);
    
    copyingBuffer.setSize (1, doubleSize);
    FVO::fill (copyingBuffer.getWritePointer(0), SampleType(0.0), doubleSize);
    
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
    

template<typename SampleType>
void HarmonizerVoice<SampleType>::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate)
{
    jassert (currentSamplerate > 0 && desiredFrequency > 0);
    
    const int numSamples = output.getNumSamples();
    
    // writes shifted samples to synthesisBuffer
    sola (parent->thisFramesInput.getReadPointer(0), numSamples, parent->nextFramesPeriod,
          juce::roundToInt (currentSamplerate / desiredFrequency),
          parent->indicesOfGrainOnsets, parent->windowBuffer.getReadPointer(0));
    
    FVO::copy (output.getWritePointer(0), synthesisBuffer.getReadPointer(0), numSamples);
    
    moveUpSamples (numSamples);
}



template<typename SampleType>
inline void HarmonizerVoice<SampleType>::sola (const SampleType* input,
                                               const int totalNumInputSamples,
                                               const int origPeriod, // size of analysis grains = origPeriod * 2
                                               const int newPeriod,  // OLA hop size
                                               const juce::Array<int>& indicesOfGrainOnsets, // sample indices marking the start of each analysis grain
                                               const SampleType* window) // Hanning window, length origPeriod * 2
{
    if (synthesisIndex > totalNumInputSamples)
        return;
    
    jassert (origPeriod > 0);
    jassert (newPeriod > 0);
    jassert (! indicesOfGrainOnsets.isEmpty());
    jassert (window != nullptr);
    
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


template<typename SampleType>
inline void HarmonizerVoice<SampleType>::olaFrame (const SampleType* inputAudio,
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
