/*
    Part of module: bv_Harmonizer
    Parent file: bv_Harmonizer.h
    Classes: HarmonizerVoice
*/


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    

template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h):
parent(h), noteOnTime(0), isQuickFading(false), noteTurnedOff(true), keyIsDown(false), currentAftertouch(0),
    isPedalPitchVoice(false), isDescantVoice(false),
    playingButReleased(false)
{
    const double initSamplerate = 44100.0;
    
    adsr        .setSampleRate (initSamplerate);
    quickRelease.setSampleRate (initSamplerate);
    quickAttack .setSampleRate (initSamplerate);
    
    adsr        .setParameters (parent->getCurrentAdsrParams());
    quickRelease.setParameters (parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters (parent->getCurrentQuickAttackParams());
    
    synthesisIndex = 0;
    
    currentlyPlayingNote.store(-1);
    currentOutputFreq.store(-1.0f);
    lastRecievedVelocity.store(0.0f);
    currentVelocityMultiplier.store(0.0f);
    prevVelocityMultiplier.store(0.0f);
    prevSoftPedalMultiplier.store(1.0f);
}

template<typename SampleType>
HarmonizerVoice<SampleType>::~HarmonizerVoice()
{ }

    
#undef bvhv_VOID_TEMPLATE
#define bvhv_VOID_TEMPLATE template<typename SampleType> void HarmonizerVoice<SampleType>
    
    
bvhv_VOID_TEMPLATE::prepare (const int blocksize)
{
    // blocksize = maximum period
    
    jassert (blocksize > 0);
    
    const int doubleSize = blocksize * 2;
    
    constexpr SampleType zero = SampleType(0.0);
    
    synthesisBuffer.setSize (1, doubleSize);
    FloatVectorOperations::fill (synthesisBuffer.getWritePointer(0), zero, doubleSize);
    
    windowingBuffer.setSize (1, doubleSize);
    FloatVectorOperations::fill (windowingBuffer.getWritePointer(0), zero, doubleSize);
    
    copyingBuffer.setSize (1, doubleSize);
    FloatVectorOperations::fill (copyingBuffer.getWritePointer(0), zero, doubleSize);
    
    prevVelocityMultiplier.store(currentVelocityMultiplier.load());
    
    prevSoftPedalMultiplier.store(parent->getSoftPedalMultiplier());
    
    synthesisIndex = 0;
}
    

bvhv_VOID_TEMPLATE::releaseResources()
{
    synthesisBuffer.setSize (0, 0, false, false, false);
    windowingBuffer.setSize (0, 0, false, false, false);
    copyingBuffer.setSize (0, 0, false, false, false);
}


bvhv_VOID_TEMPLATE::renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                     const int origPeriod,
                                     const Array<int>& indicesOfGrainOnsets,
                                     const AudioBuffer<SampleType>& windowToUse)
{
    // determine if the voice is currently active
    
    jassert (inputAudio.getNumSamples() == outputBuffer.getNumSamples());
    
    const bool voiceIsOnRightNow = isQuickFading ? quickRelease.isActive()
                                                 : (parent->isADSRon() ? adsr.isActive() : ! noteTurnedOff);
    
    if (! voiceIsOnRightNow)
    {
        clearCurrentNote();
        return;
    }
    
    const int numSamples = inputAudio.getNumSamples();
    
    // puts shifted samples into the synthesisBuffer, from sample indices starting to starting + numSamples - 1
    sola (inputAudio.getReadPointer(0), numSamples,
          origPeriod,
          roundToInt (parent->getSamplerate() / currentOutputFreq.load()),  // new desired period, in samples
          indicesOfGrainOnsets,
          windowToUse.getReadPointer(0));  // precomputed window length must be input period * 2
    
    //  *****************************************************************************************************
    //  midi velocity gain (aftertouch is applied here as well)
    constexpr float inv127 = 1.0f / 127.0f;
    const float currentmult = currentVelocityMultiplier.load();
    const float velocityMultNow = parent->isAftertouchGainOn() ? currentmult + ((currentAftertouch * inv127) * (1.0f - currentmult))
                                                               : currentmult;

    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier.load(), velocityMultNow);
    prevVelocityMultiplier.store(velocityMultNow);

    //  *****************************************************************************************************
    //  soft pedal gain

    const float softPedalMult = parent->isSoftPedalDown() ? parent->getSoftPedalMultiplier() : 1.0f; // soft pedal gain
    synthesisBuffer.applyGainRamp (0, numSamples, prevSoftPedalMultiplier.load(), softPedalMult);
    prevSoftPedalMultiplier.store(softPedalMult);

    //  *****************************************************************************************************
    //  playing-but-released gain

    const float newPBRmult = playingButReleased ? parent->getPlayingButReleasedMultiplier() : 1.0f;
    synthesisBuffer.applyGainRamp (0, numSamples, lastPBRmult, newPBRmult);
    lastPBRmult = newPBRmult;

    //  *****************************************************************************************************
    //  ADSR

    if (parent->isADSRon())
        adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // midi-triggered adsr envelope
    else
        quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes if adsr is off

    if (isQuickFading)  // quick fade out for stopNote w/ no tail off, to prevent clicks from jumping to 0
        quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);

    //  *****************************************************************************************************
    //  write to output (and apply panning)
    
    for (int chan = 0; chan < 2; ++chan)
        outputBuffer.addFromWithRamp (chan, 0, synthesisBuffer.getReadPointer(0), numSamples,
                                      panner.getPrevGain(chan), panner.getGainMult(chan));
    
    moveUpSamples (numSamples);
}


bvhv_VOID_TEMPLATE::sola (const SampleType* input, const int totalNumInputSamples,
                          const int origPeriod, // size of analysis grains = origPeriod * 2
                          const int newPeriod,  // OLA hop size
                          const Array<int>& indicesOfGrainOnsets, // sample indices marking the beginning of each analysis grain
                          const SampleType* window) // Hanning window, length origPeriod * 2
{
    if (synthesisIndex > totalNumInputSamples)
        return;
    
    const int analysisGrainLength = 2 * origPeriod; // length of the analysis grains & the pre-computed Hanning window
    
    for (int grainStart : indicesOfGrainOnsets)
    {
        const int grainEnd = grainStart + analysisGrainLength;
        
        if (grainEnd > synthesisBuffer.getNumSamples())
            break;
        
        if (synthesisIndex > grainEnd)
            continue;

        olaFrame (input, grainStart, grainEnd, analysisGrainLength, window, newPeriod);
    }
    
    // fill from the last written sample to the end of the buffer with zeroes
    constexpr SampleType zero = SampleType(0.0);
    FloatVectorOperations::fill (synthesisBuffer.getWritePointer(0) + synthesisIndex,
                                 zero,
                                 synthesisBuffer.getNumSamples() - synthesisIndex);
}


bvhv_VOID_TEMPLATE::olaFrame (const SampleType* inputAudio,
                              const int frameStartSample, const int frameEndSample, const int frameSize,
                              const SampleType* window,
                              const int newPeriod)
{
//    if (synthesisIndex + frameSize > synthesisBuffer.getNumSamples())
//        return;
    
    // apply the window before OLAing. Writes windowed input samples into the windowingBuffer
    FloatVectorOperations::multiply (windowingBuffer.getWritePointer(0),
                                     window,
                                     inputAudio + frameStartSample,
                                     frameSize);
    
    const SampleType* windowBufferReading = windowingBuffer.getReadPointer(0);
    SampleType* synthesisBufferWriting = synthesisBuffer.getWritePointer(0);
    
    do {
        jassert (synthesisIndex + frameSize <= synthesisBuffer.getNumSamples());
        
        FloatVectorOperations::add (synthesisBufferWriting + synthesisIndex,
                                    windowBufferReading,
                                    frameSize);
        
        synthesisIndex += newPeriod;
    }
    while (synthesisIndex < frameEndSample && synthesisIndex + frameSize <= synthesisBuffer.getNumSamples());
}


bvhv_VOID_TEMPLATE::moveUpSamples (const int numSamplesUsed)
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
bvhv_VOID_TEMPLATE::clearCurrentNote()
{
    currentVelocityMultiplier.store(0.0f);
    lastRecievedVelocity.store(0.0f);
    currentAftertouch = 0;
    currentlyPlayingNote.store(-1);
    noteOnTime    = 0;
    isQuickFading = false;
    noteTurnedOff = true;
    keyIsDown     = false;
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
}


// MIDI -----------------------------------------------------------------------------------------------------------
bvhv_VOID_TEMPLATE::startNote (const int midiPitch, const float velocity,
                                             const uint32 noteOnTimestamp,
                                             const bool keyboardKeyIsDown,
                                             const bool isPedal, const bool isDescant)
{
    noteOnTime = noteOnTimestamp;
    currentlyPlayingNote.store(midiPitch);
    lastRecievedVelocity.store(velocity);
    currentVelocityMultiplier.store(parent->getWeightedVelocity (velocity));
    currentOutputFreq.store(parent->getOutputFrequency (midiPitch));
    isQuickFading = false;
    noteTurnedOff = false;
    
    adsr.noteOn();
    quickAttack.noteOn();
    
    if (! quickRelease.isActive())
        quickRelease.noteOn();
    
    isPedalPitchVoice = isPedal;
    isDescantVoice = isDescant;
    
    setKeyDown (keyboardKeyIsDown);
}
    
    
bvhv_VOID_TEMPLATE::setKeyDown (bool isNowDown) noexcept
{
    keyIsDown = isNowDown;
    
    if (isNowDown)
        playingButReleased = false;
    else
        if (isPedalPitchVoice || isDescantVoice)
            playingButReleased = false;
        else if (parent->isLatched() || parent->intervalLatchIsOn)
            playingButReleased = false;
        else
            playingButReleased = isVoiceActive();
}


bvhv_VOID_TEMPLATE::stopNote (const float velocity, const bool allowTailOff)
{
    ignoreUnused (velocity);
    
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
}


bvhv_VOID_TEMPLATE::setPan (int newPan)
{
    newPan = jlimit(0, 127, newPan);
    
    if (panner.getLastMidiPan() != newPan)
        panner.setMidiPan (newPan);
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
    
template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;


} // namespace
