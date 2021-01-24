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
    // blocksize = maximum period
    
    synthesisBuffer.setSize(1, blocksize * 4, true, true, true);
    synthesisBuffer.clear();
    highestSBindexWritten = 0;
    synthesisIndex = 0;
    
    windowingBuffer.setSize(1, blocksize * 2);
    
    copyingBuffer.setSize(1, blocksize * 2, true, true, true);
    
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    prevSoftPedalMultiplier = parent->getSoftPedalMultiplier();
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                                   const int origPeriod,
                                                   const Array<int>& indicesOfGrainOnsets,
                                                   const AudioBuffer<SampleType>& windowToUse)
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
    
    const float newPeriod = (float) (parent->getSamplerate() / currentOutputFreq);
    
    sola (inputAudio, origPeriod, roundToInt(newPeriod), indicesOfGrainOnsets, windowToUse.getReadPointer(0)); // puts shifted samples into the synthesisBuffer, from sample indices 0 to numSamples-1
    
    const int numSamples = inputAudio.getNumSamples();
    
    // midi velocity gain
    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier, currentVelocityMultiplier);
    prevVelocityMultiplier = currentVelocityMultiplier;
    
    // soft pedal gain
    const float softPedalMult = parent->isSoftPedalDown() ? parent->getSoftPedalMultiplier() : 1.0f;
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
void HarmonizerVoice<SampleType>::sola (const AudioBuffer<SampleType>& inputAudio, 
                                        const int origPeriod, // size of analysis grains = origPeriod * 2
                                        const int newPeriod,  // OLA hop size
                                        const Array<int>& indicesOfGrainOnsets, // sample indices marking the beginning of each analysis grain
                                        const SampleType* window) // Hanning window, length origPeriod * 2
{
    const int totalNumInputSamples = inputAudio.getNumSamples();
    
    const auto* input = inputAudio.getReadPointer(0);
    
    if (synthesisIndex >= totalNumInputSamples)
        // don't need ANY of the analysis frames from this audio chunk, already written enough samples from previous frames...
    {
        highestSBindexWritten = synthesisIndex;
        synthesisIndex -= totalNumInputSamples;
        return;
    }
    
    const int analysisGrain = 2 * origPeriod; // length of the analysis grains & the pre-computed Hanning window
    
    for (int i = 0; i < indicesOfGrainOnsets.size(); ++i)
    {
        const int readingStartSample = indicesOfGrainOnsets.getUnchecked(i);
        const int readingEndSample = readingStartSample + analysisGrain;
        
        if (readingEndSample > totalNumInputSamples)
            break;
        
        olaFrame (input, readingStartSample, readingEndSample, window, analysisGrain, newPeriod);
        
        if (synthesisIndex >= totalNumInputSamples || readingEndSample == totalNumInputSamples)
            break;
    }
    
    synthesisIndex -= totalNumInputSamples;
    if (synthesisIndex < 0) synthesisIndex = 0;
};


template<typename SampleType>
void HarmonizerVoice<SampleType>::olaFrame (const SampleType* inputAudio, const int readingStartSample, const int readingEndSample,
                                            const SampleType* window, const int windowSize, // window size = origPeriod * 2
                                            const int newPeriod)
{
    // this processes one analysis frame of input samples, from readingStartSample to readingEndSample
    
    if (synthesisIndex > readingEndSample)
        return;
    
    jassert ((readingEndSample - readingStartSample) == windowSize);

    // 1. multiply the analysis grain by the window before OLAing, so that window multiplication only has to be done once per analysis grain
    
    auto* w = windowingBuffer.getWritePointer(0);
    
    for (int s = readingStartSample, // reading index from orignal audio
         wi = 0;                     // writing index in the windowing multiplication storage buffer
         s < readingEndSample;
         ++s, ++wi)
    {
        w[wi] = inputAudio[s] * window[wi];
    }
    
    int highestIndexWritten = 0;
    
    // 2. resynthesis / OLA
    while (synthesisIndex < readingEndSample)
    {
        auto* writing = synthesisBuffer.getWritePointer(0);
        const auto* reading = windowingBuffer.getReadPointer(0); // pre-windowed analysis grain starts at index 0
        
        for (int s = synthesisIndex, r = 0;
             r < windowSize;
             ++s, ++r)
        {
            writing[s] += reading[r];
        }
        
        highestIndexWritten = synthesisIndex + windowSize;
        synthesisIndex += newPeriod;
    }
    
    highestSBindexWritten = highestIndexWritten;
};



template<typename SampleType>
void HarmonizerVoice<SampleType>::moveUpSamples (AudioBuffer<SampleType>& targetBuffer,
                                                 const int numSamplesUsed,
                                                 const int highestIndexWritten)
{
    const int numSamplesLeft = highestIndexWritten - numSamplesUsed;
    
    if (numSamplesLeft < 1)
    {
        targetBuffer.clear();
        highestSBindexWritten = 0;
        return;
    }
    
    copyingBuffer.copyFrom (0, 0, targetBuffer, 0, numSamplesUsed, numSamplesLeft);
    targetBuffer.clear();
    targetBuffer.copyFrom (0, 0, copyingBuffer, 0, 0, numSamplesLeft);
    
    highestSBindexWritten = numSamplesLeft;
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
void HarmonizerVoice<SampleType>::startNote (const int midiPitch, const float velocity, const bool wasStolen)
{
    currentlyPlayingNote = midiPitch;
    lastRecievedVelocity = velocity;
    currentVelocityMultiplier = parent->getWeightedVelocity(velocity);
    currentOutputFreq         = parent->getOutputFrequency(midiPitch);
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
    bool isPlayingButReleased = isVoiceActive() && (! (keyIsDown || parent->isSostenutoPedalDown() || parent->isSustainPedalDown()));
    
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
