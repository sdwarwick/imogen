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
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h): parent(h), noteOnTime(0), isQuickFading(false), noteTurnedOff(false), keyIsDown(false), currentAftertouch(0), sustainingFromSostenutoPedal(false), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), lastRecievedVelocity(0.0f), isPedalPitchVoice(false), isDescantVoice(false), playingButReleased(false), lastPBRmult(1.0f), synthesisIndex(0)
{
    const double initSamplerate = 44100.0;
    
    adsr        .setSampleRate (initSamplerate);
    quickRelease.setSampleRate (initSamplerate);
    quickAttack .setSampleRate (initSamplerate);
    adsr        .setParameters (parent->getCurrentAdsrParams());
    quickRelease.setParameters (parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters (parent->getCurrentQuickAttackParams());
}

template<typename SampleType>
HarmonizerVoice<SampleType>::~HarmonizerVoice()
{ }

    
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
    
    resetRampedValues (blocksize);
}
    

bvhv_VOID_TEMPLATE::resetRampedValues (int blocksize)
{
    midiVelocityGain.reset (blocksize);
    softPedalGain.reset (blocksize);
    playingButReleasedGain.reset (blocksize);
    aftertouchGain.reset (blocksize);
    outputLeftGain.reset (blocksize);
    outputRightGain.reset (blocksize);
}
    

bvhv_VOID_TEMPLATE::releaseResources()
{
    synthesisBuffer.setSize (0, 0, false, false, false);
    windowingBuffer.setSize (0, 0, false, false, false);
    copyingBuffer.setSize (0, 0, false, false, false);
}
    

bvhv_VOID_TEMPLATE::bypassedBlock (const int numSamples)
{
    midiVelocityGain.skip (numSamples);
    softPedalGain.skip (numSamples);
    playingButReleasedGain.skip (numSamples);
    aftertouchGain.skip (numSamples);
    outputLeftGain.skip (numSamples);
    outputRightGain.skip (numSamples);
}


bvhv_VOID_TEMPLATE::renderNextBlock (const AudioBuffer& inputAudio, AudioBuffer& outputBuffer,
                                     const int origPeriod,
                                     const juce::Array<int>& indicesOfGrainOnsets,
                                     const AudioBuffer& windowToUse)
{
    jassert (inputAudio.getNumSamples() == outputBuffer.getNumSamples());
    
    // determine if the voice is currently active
    const bool voiceIsOnRightNow = isQuickFading ? quickRelease.isActive()
                                                 : ( parent->isADSRon() ? adsr.isActive() : ! noteTurnedOff );
    if (! voiceIsOnRightNow)
    {
        clearCurrentNote();
        return;
    }
    
    jassert (currentOutputFreq > 0);
    
    const int numSamples = inputAudio.getNumSamples();
    
    // puts shifted samples into the synthesisBuffer
    sola (inputAudio.getReadPointer(0), numSamples, origPeriod,
          juce::roundToInt (parent->getSamplerate() / currentOutputFreq),  // new desired period, in samples
          indicesOfGrainOnsets, // analysis grains are length origPeriod * 2, approx 50% overlap, centred on points of maximum energy in input signal
          windowToUse.getReadPointer(0));  // precomputed window length must be length origPeriod * 2
    
    auto* synthesisSamples = synthesisBuffer.getWritePointer(0);
    
    for (int s = 0; s < numSamples; ++s)
    {
        *(synthesisSamples + s) = synthesisSamples[s] * midiVelocityGain.getNextValue()
                                                      * softPedalGain.getNextValue()
                                                      * playingButReleasedGain.getNextValue()
                                                      * aftertouchGain.getNextValue();
    }
    
    //  ADSR
    if (parent->isADSRon())
        adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // midi-triggered adsr envelope
    else
        quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes if adsr is off

    if (isQuickFading)  // quick fade out for stopNote w/ no tail off, to prevent clicks from jumping to 0
        quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);

    //  write to output and apply panning
    const auto* reading = synthesisBuffer.getReadPointer(0);
    auto* leftOutput  = outputBuffer.getWritePointer(0);
    auto* rightOutput = outputBuffer.getWritePointer(1);
    
    for (int s = 0; s < numSamples; ++s)
    {
        const auto sample = reading[s];
        *(leftOutput + s)  = sample * outputLeftGain.getNextValue();
        *(rightOutput + s) = sample * outputRightGain.getNextValue();
    }
    
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
    {
        constexpr SampleType zero = SampleType(0.0);
        FVO::fill (synthesisBuffer.getWritePointer(0) + synthesisIndex,
                   zero, sampsLeft);
    }
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
        
        FVO::add (synthesisBufferWriting + synthesisIndex,
                  windowBufferReading, frameSize);
        
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
bvbv_INLINE_VOID_TEMPLATE::clearCurrentNote()
{
    lastRecievedVelocity = 0.0f;
    currentAftertouch = 0;
    currentlyPlayingNote = -1;
    noteOnTime = 0;
    isQuickFading = false;
    noteTurnedOff = true;
    keyIsDown = false;
    playingButReleased = false;
    isPedalPitchVoice = false;
    isDescantVoice = false;
    sustainingFromSostenutoPedal = false;
    
    setPan (64);
    
    if (quickRelease.isActive())
        quickRelease.reset();
    
    quickRelease.noteOn();
    
    if (adsr.isActive())
        adsr.reset();
    
    if (quickAttack.isActive())
        quickAttack.reset();
    
    synthesisBuffer.clear();
    
    resetRampedValues (juce::roundToInt (floor (synthesisBuffer.getNumSamples() * 0.5f)));
}


// MIDI -----------------------------------------------------------------------------------------------------------
bvhv_VOID_TEMPLATE::startNote (const int midiPitch, const float velocity,
                               const uint32 noteOnTimestamp,
                               const bool keyboardKeyIsDown,
                               const bool isPedal, const bool isDescant)
{
    noteOnTime = noteOnTimestamp;
    currentlyPlayingNote = midiPitch;
    lastRecievedVelocity = velocity;
    currentOutputFreq = parent->getOutputFrequency (midiPitch);
    isQuickFading = false;
    noteTurnedOff = false;
    
    adsr.noteOn();
    quickAttack.noteOn();
    
    if (! quickRelease.isActive())
        quickRelease.noteOn();
    
    isPedalPitchVoice = isPedal;
    isDescantVoice = isDescant;
    
    setKeyDown (keyboardKeyIsDown);
    
    midiVelocityGain.setTargetValue (_SMOOTHING_ZERO_CHECK (parent->getWeightedVelocity (velocity)));
}
    
    
bvhv_VOID_TEMPLATE::setKeyDown (bool isNowDown)
{
    keyIsDown = isNowDown;
    
    if (isNowDown)
        playingButReleased = false;
    else
    {
        if (isPedalPitchVoice || isDescantVoice)
            playingButReleased = false;
        else if (parent->isLatched())
            playingButReleased = false;
        else
            playingButReleased = isVoiceActive();
    }
    
    if (playingButReleased)
        playingButReleasedGain.setTargetValue (_SMOOTHING_ZERO_CHECK (parent->getPlayingButReleasedMultiplier()));
    else
        playingButReleasedGain.setTargetValue (SampleType(1.0));
}
    

bvhv_VOID_TEMPLATE::softPedalChanged (bool isDown)
{
    if (isDown)
        softPedalGain.setTargetValue (_SMOOTHING_ZERO_CHECK (parent->getSoftPedalMultiplier()));
    else
        softPedalGain.setTargetValue (SampleType(1.0));
}


bvhv_VOID_TEMPLATE::stopNote (const float velocity, const bool allowTailOff)
{
    midiVelocityGain.setTargetValue (_SMOOTHING_ZERO_CHECK (parent->getWeightedVelocity (lastRecievedVelocity - velocity)));
    
    if (allowTailOff)
    {
        adsr.noteOff();
        isQuickFading = false;
    }
    else
    {
        if (! quickRelease.isActive())
            quickRelease.noteOn();
        
        isQuickFading = true;
        
        quickRelease.noteOff();
    }
    
    noteTurnedOff = true;
    keyIsDown = false;
    playingButReleased = false;
}


bvhv_VOID_TEMPLATE::aftertouchChanged (const int newAftertouchValue)
{
    currentAftertouch = newAftertouchValue;
    
    constexpr auto inv127 = SampleType (1.0 / 127.0);
    
    if (parent->isAftertouchGainOn())
        aftertouchGain.setTargetValue (_SMOOTHING_ZERO_CHECK (newAftertouchValue * inv127));
    else
        aftertouchGain.setTargetValue (SampleType(1.0));
}


bvhv_VOID_TEMPLATE::setPan (int newPan)
{
    newPan = juce::jlimit(0, 127, newPan);
    
    if (panner.getLastMidiPan() == newPan)
        return;
    
    panner.setMidiPan (newPan);
    
    outputLeftGain.setTargetValue (_SMOOTHING_ZERO_CHECK (panner.getLeftGain()));
    outputRightGain.setTargetValue (_SMOOTHING_ZERO_CHECK (panner.getRightGain()));
}


bvhv_VOID_TEMPLATE::updateSampleRate (const double newSamplerate)
{
    adsr        .setSampleRate (newSamplerate);
    quickRelease.setSampleRate (newSamplerate);
    quickAttack .setSampleRate (newSamplerate);
    
    adsr        .setParameters (parent->getCurrentAdsrParams());
    quickRelease.setParameters (parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters (parent->getCurrentQuickAttackParams());
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
