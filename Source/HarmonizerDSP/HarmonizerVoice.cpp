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
parent(h), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), noteOnTime(0), currentMidipan(64), currentVelocityMultiplier(0.0f), prevVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), isQuickFading(false), noteTurnedOff(true), keyIsDown(false), currentAftertouch(64), softPedalMultiplier(1.0f), prevSoftPedalMultiplier(1.0f)
{
    panningMults[0] = 0.5f;
    panningMults[1] = 0.5f;
    
    prevPanningMults[0] = 0.5f;
    prevPanningMults[1] = 0.5f;
    
    const double initSamplerate = std::max<double>(parent->getSamplerate(), 44100.0);
    
    adsr        .setSampleRate(initSamplerate);
    quickRelease.setSampleRate(initSamplerate);
    quickAttack .setSampleRate(initSamplerate);
    
    adsr        .setParameters(parent->getCurrentAdsrParams());
    quickRelease.setParameters(parent->getCurrentQuickReleaseParams());
    quickAttack .setParameters(parent->getCurrentQuickAttackParams());
    
    for (int w = 0; w < 8; ++w)
        wavelets.add (new WaveletGenerator<SampleType>);
    
    // call fill window buffer in constuctor !
};

template<typename SampleType>
HarmonizerVoice<SampleType>::~HarmonizerVoice()
{ };

template<typename SampleType>
void HarmonizerVoice<SampleType>::prepare (const int blocksize)
{
    synthesisBuffer.setSize(1, blocksize, true, true, true);
    window         .setSize(1, blocksize, true, true, true);
    
    finalWindow.ensureStorageAllocated(blocksize);
    
    prevPanningMults[0] = panningMults[0];
    prevPanningMults[1] = panningMults[1];
    
    prevVelocityMultiplier = currentVelocityMultiplier;
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                                   const Array<int>& epochIndices, const int numOfEpochsPerFrame,
                                                   const SampleType currentInputFreq)
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
    
    const float shiftingRatio = 1 / ( 1 + ( (currentInputFreq - currentOutputFreq) / currentOutputFreq ) );
    
    // puts shifted samples into the synthesisBuffer, from sample indices 0 to numSamples-1
    esola (inputAudio, shiftingRatio);
    
    // midi velocity gain
    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier, currentVelocityMultiplier);
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    // soft pedal gain
    if (parent->isSoftPedalDown())
        softPedalMultiplier = parent->getSoftPedalMultiplier();
    else
        softPedalMultiplier = 1.0f;
    synthesisBuffer.applyGainRamp (0, numSamples, prevSoftPedalMultiplier, softPedalMultiplier);
    prevSoftPedalMultiplier = softPedalMultiplier;
    
    if (parent->isADSRon()) // only apply the envelope if the ADSR on/off user toggle is ON
        adsr.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples);
    else
        quickAttack.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // to prevent pops at start of notes
    
    if (isQuickFading)
        quickRelease.applyEnvelopeToBuffer (synthesisBuffer, 0, numSamples); // quick fade out for stopNote() w/ allowTailOff = false
    
    // write to output & apply panning (w/ gain multipliers ramped)
    for (int chan = 0; chan < 2; ++chan)
    {
        outputBuffer.addFromWithRamp (chan, 0, synthesisBuffer.getReadPointer(0), numSamples, prevPanningMults[chan], panningMults[chan]);
        prevPanningMults[chan] = panningMults[chan];
    }
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::esola (const AudioBuffer<SampleType>& inputAudio,
                                         const float shiftingRatio)
{
    // the OlA will look something like this...
    
    
    // this block of code should process grains of audio 2 pitch periods long!
    // center these grains on the epochs of the signal
    
    synthesisBuffer.clear();
    
    const int totalNumSamples = inputAudio.getNumSamples();
    
    const int sampleIncrement = floor (totalNumSamples * shiftingRatio);
    const int olaIncrement    = floor (sampleIncrement / 2.0f);
    
    int totalNumSamplesWritten = 0;
    int startSample = 0;
    
    for (auto* wavelet : wavelets)
    {
        wavelet->ola (inputAudio, synthesisBuffer, shiftingRatio, startSample, parent->olaWindow);
        
        totalNumSamplesWritten += sampleIncrement;
        startSample            += olaIncrement;
        
        if (totalNumSamplesWritten >= totalNumSamples)
            break;
    }
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::releaseResources()
{
    synthesisBuffer.setSize(0, 0, false, false, false);
    window.setSize(0, 0, false, false, false);
    finalWindow.clear();
    
    prevPanningMults[0] = panningMults[0];
    prevPanningMults[1] = panningMults[1];
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::clearBuffers()
{
    synthesisBuffer.clear();
    window.clear();
    finalWindow.clearQuick();
};




template<typename SampleType>
void HarmonizerVoice<SampleType>::fillWindowBuffer(const int numSamples)
{
    window.clear();
    auto* writing = window.getWritePointer(0);
    const auto samplemultiplier = MathConstants<SampleType>::pi / static_cast<SampleType> (numSamples - 1);
    
    for(int i = 0; i < numSamples; ++i)
        writing[i] = static_cast<SampleType> (0.5 - 0.5 * (std::cos(static_cast<SampleType> (2 * i) * samplemultiplier)) );
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
void HarmonizerVoice<SampleType>::setPan(const int newPan)
{
    jassert(isPositiveAndBelow(newPan, 128));
    
    if (currentMidipan == newPan)
        return;
    
    parent->panValTurnedOff(currentMidipan);
    
    prevPanningMults[0] = panningMults[0];
    prevPanningMults[1] = panningMults[1];
    
    if(newPan == 64) // save time for the simplest case
    {
        panningMults[0] = 0.5f;
        panningMults[1] = 0.5f;
    }
    else
    {
        const float Rpan = newPan / 127.0f;
        panningMults[1] = Rpan; // R channel
        panningMults[0] = 1.0f - Rpan; // L channel
    }
    
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
    window         .setSize(1, newMaxBlocksize, true, true, true);
    finalWindow.ensureStorageAllocated(newMaxBlocksize);
};


template class HarmonizerVoice<float>;
template class HarmonizerVoice<double>;
