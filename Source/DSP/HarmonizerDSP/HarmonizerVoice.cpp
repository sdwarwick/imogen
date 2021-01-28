/*
  ==============================================================================

    HarmonizerVoice.cpp
    Created: 11 Jan 2021 7:06:49pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "Harmonizer.h"


template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h):
parent(h), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), noteOnTime(0), currentMidipan(64), currentVelocityMultiplier(0.0f), prevVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), isQuickFading(false), noteTurnedOff(true), keyIsDown(false), currentAftertouch(64), prevSoftPedalMultiplier(1.0f),
    isPedalPitchVoice(false), isDescantVoice(false),
    playingButReleased(false)
{
    const double initSamplerate = std::max<double>(parent->getSamplerate(), 44100.0);
    
    adsr        .setSampleRate(initSamplerate);
    quickRelease.setSampleRate(initSamplerate);
    quickAttack .setSampleRate(initSamplerate);
    
    adsr        .setParameters(parent->getCurrentAdsrParams());
    quickRelease.setParameters(parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters(parent->getCurrentQuickAttackParams());
};

template<typename SampleType>
HarmonizerVoice<SampleType>::~HarmonizerVoice()
{ };

template<typename SampleType>
void HarmonizerVoice<SampleType>::prepare (const int blocksize)
{
    // blocksize = maximum period
    
    synthesisBuffer.setSize (1, blocksize * 4);
    synthesisBuffer.clear();
    nextSBindex = 0;
    
    windowingBuffer.setSize (1, blocksize * 2);
    
    copyingBuffer.setSize (1, blocksize * 2);
    
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    prevSoftPedalMultiplier = parent->getSoftPedalMultiplier();
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::setKeyDown (bool isNowDown) noexcept
{
    if (keyIsDown == isNowDown)
        return;
    
    keyIsDown = isNowDown;
    
    if (! isNowDown)
    {
        if (isPedalPitchVoice || isDescantVoice)
            playingButReleased = false;
        else
            playingButReleased = isVoiceActive();
    }
    else
    {
        playingButReleased = false;
    }
}



template<typename SampleType>
void HarmonizerVoice<SampleType>::renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                                   const int origPeriod,
                                                   const Array<int>& indicesOfGrainOnsets,
                                                   const AudioBuffer<SampleType>& windowToUse)
{
    if (! keyIsDown)
    {
        bool shouldStopNote;
        
        if (isPedalPitchVoice || isDescantVoice)
            shouldStopNote = false;
        else if (parent->isLatched() || parent->isIntervalLatchOn())
            shouldStopNote = false;
        else if (parent->isSustainPedalDown() || parent->isSostenutoPedalDown())
            shouldStopNote = false;
        else
            shouldStopNote = true;
        
        if (shouldStopNote)
            stopNote (1.0f, false);
    }
    
    bool voiceIsOnRightNow;
    
    if (isQuickFading)
        voiceIsOnRightNow = quickRelease.isActive();
    else
        voiceIsOnRightNow = (parent->isADSRon()) ? adsr.isActive() : (! noteTurnedOff);
    
    if (! voiceIsOnRightNow)
    {
        clearCurrentNote();
        return;
    }
    
    const float newPeriod = static_cast<float> (parent->getSamplerate() / currentOutputFreq);
    
    sola (inputAudio, origPeriod, roundToInt(newPeriod), indicesOfGrainOnsets, windowToUse.getReadPointer(0)); // puts shifted samples into the synthesisBuffer, from sample indices 0 to numSamples-1
    
    const int numSamples = inputAudio.getNumSamples();
    
    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier, currentVelocityMultiplier); // midi velocity gain
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    // soft pedal gain
    const float softPedalMult = parent->isSoftPedalDown() ? parent->getSoftPedalMultiplier() : 1.0f;
    synthesisBuffer.applyGainRamp (0, numSamples, prevSoftPedalMultiplier, softPedalMult);
    prevSoftPedalMultiplier = softPedalMult;
    
    if (parent->isADSRon())
        adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);
    else
        quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes
    
    if (isQuickFading)
        quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // quick fade out for stopNote() w/ allowTailOff = false, to prevent clicks from jumping to 0
    
    for (int chan = 0; chan < 2; ++chan)
        outputBuffer.addFromWithRamp (chan, 0, synthesisBuffer.getReadPointer(0), numSamples,
                                      panner.getPrevGain(chan), panner.getGainMult(chan)); // panning is applied while writing to output
    
    moveUpSamples (numSamples);
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::sola (const AudioBuffer<SampleType>& inputAudio, 
                                        const int origPeriod, // size of analysis grains = origPeriod * 2
                                        const int newPeriod,  // OLA hop size
                                        const Array<int>& indicesOfGrainOnsets, // sample indices marking the beginning of each analysis grain
                                        const SampleType* window) // Hanning window, length origPeriod * 2
{
    const int totalNumInputSamples = inputAudio.getNumSamples();
    
    const auto* input = inputAudio.getReadPointer(0);
    
    if (nextSBindex >= totalNumInputSamples)
        return;
    
    const int analysisGrainLength = 2 * origPeriod; // length of the analysis grains & the pre-computed Hanning window
    
    for (int grainStart : indicesOfGrainOnsets)
    {
        const int grainEnd = grainStart + analysisGrainLength;
        
        if (grainEnd > totalNumInputSamples)
            break;
        
        if (nextSBindex > grainEnd)
            continue;
        
        olaFrame (input, grainStart, grainEnd, window, newPeriod);
        
        if (nextSBindex >= totalNumInputSamples)
            break;
    }
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::olaFrame (const SampleType* inputAudio,
                                            const int frameStartSample, const int frameEndSample,
                                            const SampleType* window,
                                            const int newPeriod)
{
    // this processes one analysis frame of input samples, from readingStartSample to readingEndSample
    // the frame size should be 2 * the original input period
    
    const int frameSize = frameEndSample - frameStartSample; // frame size should equal the original period * 2
    // NB: the length of the pre-computed Hanning window passed to this function MUST equal the frame size!

    // 1. apply the window before OLAing, so that windowing only has to be done once per analysis grain
    
    auto* w = windowingBuffer.getWritePointer(0);
    
    for (int s = frameStartSample, // reading index from orignal audio
         wi = 0;                   // writing index in the windowing multiplication storage buffer
         s < frameEndSample;
         ++s, ++wi)                // frame length must equal window length
    {
        w[wi] = inputAudio[s] * window[wi];
    }
    
    int synthesisIndex = nextSBindex;
    
    // 2. resynthesis / OLA
    
    while (synthesisIndex < frameEndSample)
    {
        auto* writing = synthesisBuffer.getWritePointer(0);
        const auto* reading = windowingBuffer.getReadPointer(0);
        
        for (int s = nextSBindex, r = 0; // pre-windowed analysis grain starts at index 0 of windowingBuffer
             r < frameSize;
             ++s, ++r)
        {
            writing[s] += reading[r];
        }
        
        synthesisIndex += newPeriod;
    }
    
    nextSBindex = synthesisIndex + 1;
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::moveUpSamples (const int numSamplesUsed)
{
    const int numSamplesLeft = nextSBindex - numSamplesUsed;
    
    if (numSamplesLeft < 1)
    {
        synthesisBuffer.clear();
        nextSBindex = 0;
        return;
    }
    
    copyingBuffer.copyFrom (0, 0, synthesisBuffer, 0, numSamplesUsed, numSamplesLeft);
    synthesisBuffer.clear();
    synthesisBuffer.copyFrom (0, 0, copyingBuffer, 0, 0, numSamplesLeft);
    
    nextSBindex = numSamplesLeft;
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::releaseResources()
{
    synthesisBuffer.setSize(0, 0, false, false, false);
    
    prevPanningMults[0] = panningMults[0];
    prevPanningMults[1] = panningMults[1];
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::clearBuffers()
{
    synthesisBuffer.clear();
};


// this function is called to reset the HarmonizerVoice's internal state to neutral / initial
template<typename SampleType>
void HarmonizerVoice<SampleType>::clearCurrentNote()
{
    currentVelocityMultiplier = 0.0f;
    lastRecievedVelocity      = 0.0f;
    currentlyPlayingNote = -1;
    noteOnTime    = 0;
    isQuickFading = false;
    noteTurnedOff = true;
    keyIsDown     = false;
    
    setPan (64);
    
    if (quickRelease.isActive())
        quickRelease.reset();
    
    quickRelease.noteOn();
    
    if (adsr.isActive())
        adsr.reset();
    
    if (quickAttack.isActive())
        quickAttack.reset();
    
    clearBuffers();
    synthesisBuffer.clear();
    
    nextSBindex = 0;
};


// MIDI -----------------------------------------------------------------------------------------------------------
template<typename SampleType>
void HarmonizerVoice<SampleType>::startNote (const int midiPitch, const float velocity, const bool wasStolen)
{
    currentlyPlayingNote = midiPitch;
    lastRecievedVelocity = velocity;
    currentVelocityMultiplier = parent->getWeightedVelocity (velocity);
    currentOutputFreq = parent->getOutputFrequency (midiPitch);
    isQuickFading = false;
    noteTurnedOff = false;
    
    adsr.noteOn();
    quickAttack.noteOn();
    
    if (! quickRelease.isActive())
        quickRelease.noteOn();
};

template<typename SampleType>
void HarmonizerVoice<SampleType>::stopNote(const float velocity, const bool allowTailOff)
{
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
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::aftertouchChanged(const int newAftertouchValue)
{
    currentAftertouch = newAftertouchValue;
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::setPan (const int newPan,  const bool reportOldToParent)
{
    jassert (isPositiveAndBelow (newPan, 128));
    
    if (currentMidipan == newPan)
        return;
    
    if (reportOldToParent)
        parent->panValTurnedOff (currentMidipan);
    
    panner.setMidiPan (newPan);
    
    currentMidipan = newPan;
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::updateSampleRate(const double newSamplerate)
{
    adsr        .setSampleRate(newSamplerate);
    quickRelease.setSampleRate(newSamplerate);
    quickAttack .setSampleRate(newSamplerate);
    
    adsr        .setParameters(parent->getCurrentAdsrParams());
    quickRelease.setParameters(parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters(parent->getCurrentQuickAttackParams());
};


/*+++++++++++++++++++++++++++++++++++++
 DANGER!!!
 FOR NON REAL TIME ONLY!!!!!!!!
 ++++++++++++++++++++++++++++++++++++++*/
template<typename SampleType>
void HarmonizerVoice<SampleType>::increaseBufferSizes(const int newMaxBlocksize)
{
    synthesisBuffer.setSize(1, newMaxBlocksize, true, true, true);
};


template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;
