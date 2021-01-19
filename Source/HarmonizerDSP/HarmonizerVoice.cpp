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
parent(h), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), noteOnTime(0), currentMidipan(64), currentVelocityMultiplier(0.0f), prevVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), isQuickFading(false), noteTurnedOff(true), keyIsDown(false), currentAftertouch(64), prevSoftPedalMultiplier(1.0f)
{
    const double initSamplerate = std::max<double>(parent->getSamplerate(), 44100.0);
    
    adsr        .setSampleRate(initSamplerate);
    quickRelease.setSampleRate(initSamplerate);
    quickAttack .setSampleRate(initSamplerate);
    
    adsr        .setParameters(parent->getCurrentAdsrParams());
    quickRelease.setParameters(parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters(parent->getCurrentQuickAttackParams());
    
    // call fill window buffer in constuctor !
};

template<typename SampleType>
HarmonizerVoice<SampleType>::~HarmonizerVoice()
{ };

template<typename SampleType>
void HarmonizerVoice<SampleType>::prepare (const int blocksize)
{
    synthesisBuffer.setSize(1, blocksize * 2, true, true, true);
    synthesisBuffer.clear();
    
    copyingBuffer.setSize(1, blocksize, true, true, true);
    
    highestSBindexWritten = 0;
    
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    prevSoftPedalMultiplier = parent->getSoftPedalMultiplier();
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                                   const float origPeriod,
                                                   const Array<int>& indicesOfGrainOnsets)
{
    if ( (! ( parent->isSustainPedalDown() || parent->isSostenutoPedalDown() ) ) && (! keyIsDown) )
        stopNote(1.0f, false);
    
    // don't want to just use the ADSR to tell if the voice is currently active, bc if the user has turned the ADSR off, the voice would remain active for the release phase of the ADSR...
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
    
    const int numSamples = inputAudio.getNumSamples();
    
    const float newPeriod = 1.0f / currentOutputFreq * parent->getSamplerate();
    
    esola (inputAudio, origPeriod, newPeriod, indicesOfGrainOnsets); // puts shifted samples into the synthesisBuffer, from sample indices 0 to numSamples-1
    
    // midi velocity gain
    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier, currentVelocityMultiplier);
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    // soft pedal gain
    const float softPedalMult = parent->getSoftPedalMultiplier();
    if (parent->isSoftPedalDown())
        synthesisBuffer.applyGainRamp (0, numSamples, prevSoftPedalMultiplier, softPedalMult);
    prevSoftPedalMultiplier = softPedalMult;
    
    if (parent->isADSRon()) // only apply the envelope if the ADSR on/off user toggle is ON
        adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);
    else
        quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes
    
    if (isQuickFading)
        quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // quick fade out for stopNote() w/ allowTailOff = false
    
    // write to output & apply panning (w/ gain multipliers ramped)
    for (int chan = 0; chan < 2; ++chan)
        outputBuffer.addFromWithRamp (chan, 0, synthesisBuffer.getReadPointer(0), numSamples,
                                      panner.getPrevGain(chan), panner.getGainMult(chan));
    
    moveUpSamples (synthesisBuffer, numSamples, highestSBindexWritten);
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::esola (const AudioBuffer<SampleType>& inputAudio,
                                         const float origPeriod,
                                         const float newPeriod,
                                         const Array<int>& indicesOfGrainOnsets)
{
    const int origPeriodInt = roundToInt(origPeriod); // size of input grains in samples
    const int newPeriodInt  = floor (newPeriod); // OLA hop size
    
    int synthesisIndex = 0; // starting index for each synthesis grain being written
    int highestIndexWritten = 0; // highest written-to index in the synthesis buffer
    
    for (int i = 0; i < indicesOfGrainOnsets.size(); ++i)
    {
        const int readingStartSample = indicesOfGrainOnsets.getUnchecked(i);
        
        int readingEndSample = readingStartSample + origPeriodInt;
        
        if (readingEndSample >= inputAudio.getNumSamples())
            readingEndSample = inputAudio.getNumSamples() - 1;
        else if (i < (indicesOfGrainOnsets.size() - 1))
            if (readingEndSample > indicesOfGrainOnsets.getUnchecked(i + 1))
                readingEndSample = indicesOfGrainOnsets.getUnchecked(i + 1);
                
        const int grainSize = readingEndSample - readingStartSample + 1;
        
        while (synthesisIndex <= readingEndSample)
        {
            synthesisBuffer.addFrom (0, synthesisIndex, inputAudio, 0, readingStartSample, grainSize);
            highestIndexWritten = synthesisIndex + grainSize;
            synthesisIndex += newPeriodInt;
        }
    }
    
    highestSBindexWritten = highestIndexWritten;
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::moveUpSamples (AudioBuffer<SampleType>& targetBuffer,
                                                 const int numSamplesUsed,
                                                 const int highestIndexWritten)
{
    const int numSamplesLeft = highestIndexWritten - numSamplesUsed + 1;
    
    if (numSamplesLeft < 1)
        return;
    
    copyingBuffer.copyFrom (0, 0, targetBuffer, 0, numSamplesUsed, numSamplesLeft);
    
    targetBuffer.clear();
    
    targetBuffer.copyFrom (0, 0, copyingBuffer, 0, 0, numSamplesLeft);
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




template<typename SampleType>
void HarmonizerVoice<SampleType>::fillWindowBuffer(const int numSamples)
{
//    window.clear();
//    auto* writing = window.getWritePointer(0);
//    const auto samplemultiplier = MathConstants<SampleType>::pi / static_cast<SampleType> (numSamples - 1);
//
//    for(int i = 0; i < numSamples; ++i)
//        writing[i] = static_cast<SampleType> (0.5 - 0.5 * (std::cos(static_cast<SampleType> (2 * i) * samplemultiplier)) );
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
    
    setPan(64);
    
    if(quickRelease.isActive())
        quickRelease.reset();
    quickRelease.noteOn();
    
    if(adsr.isActive())
        adsr.reset();
    
    if(quickAttack.isActive())
        quickAttack.reset();
    
    clearBuffers();
    synthesisBuffer.clear();
};


// MIDI -----------------------------------------------------------------------------------------------------------
template<typename SampleType>
void HarmonizerVoice<SampleType>::startNote(const int midiPitch, const float velocity)
{
    currentlyPlayingNote = midiPitch;
    lastRecievedVelocity = velocity;
    currentVelocityMultiplier = parent->getWeightedVelocity(velocity);
    currentOutputFreq         = parent->getOutputFrequency(midiPitch);
    isQuickFading = false;
    noteTurnedOff = false;
    
    adsr.noteOn();
    quickAttack.noteOn();
    if(! quickRelease.isActive())
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
        if(! quickRelease.isActive())
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
void HarmonizerVoice<SampleType>::setPan (const int newPan)
{
    jassert (isPositiveAndBelow (newPan, 128));
    
    if (currentMidipan == newPan)
        return;
    
    parent->panValTurnedOff(currentMidipan);
    
    panner.setMidiPan (newPan);
    
    currentMidipan = newPan;
};

template<typename SampleType>
bool HarmonizerVoice<SampleType>::isPlayingButReleased() const noexcept
{
    bool isPlayingButReleased = isVoiceActive()
                                && (! (keyIsDown || parent->isSostenutoPedalDown() || parent->isSustainPedalDown()));
    
    if (parent->isPedalPitchOn())
        isPlayingButReleased = isPlayingButReleased && (currentlyPlayingNote != parent->getCurrentPedalPitchNote());
    
    if (parent->isDescantOn())
        isPlayingButReleased = isPlayingButReleased && (currentlyPlayingNote != parent->getCurrentDescantNote());
    
    return isPlayingButReleased;
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
