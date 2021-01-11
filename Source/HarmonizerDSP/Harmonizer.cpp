/*
 ==============================================================================
 
 Harmonizer.cpp
 Created: 13 Dec 2020 7:53:39pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#include "Harmonizer.h"

template<typename SampleType>
HarmonizerVoice<SampleType>::HarmonizerVoice(Harmonizer<SampleType>* h):
    parent(h), currentlyPlayingNote(-1), currentOutputFreq(-1.0f), noteOnTime(0), currentMidipan(64), currentVelocityMultiplier(0.0f), prevVelocityMultiplier(0.0f), lastRecievedVelocity(0.0f), isQuickFading(false), noteTurnedOff(true), keyIsDown(false), currentAftertouch(64)
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
void HarmonizerVoice<SampleType>::renderNextBlock(const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                                                  const Array<int>& epochIndices, const int numOfEpochsPerFrame,
                                                  const SampleType currentInputFreq)
{
    if ( (! ( parent->isSustainPedalDown() || parent->isSostenutoPedalDown() ) ) && (! keyIsDown) )
        stopNote(1.0f, false);
    
    // don't want to just use the ADSR to tell if the voice is currently active, bc if the user has turned the ADSR off, the voice would remain active for the release phase of the ADSR...
    bool voiceIsOnRightNow;
    if ( (! isQuickFading) && parent->isADSRon() )
        voiceIsOnRightNow = adsr.isActive();
    else
        voiceIsOnRightNow = isQuickFading ? quickRelease.isActive() : (! noteTurnedOff);
    
    if (! voiceIsOnRightNow)
    {
        clearCurrentNote();
        return;
    }
    
    const int numSamples = inputAudio.getNumSamples();
    
    const float shiftingRatio = 1 / ( 1 + ( (currentInputFreq - currentOutputFreq) / currentOutputFreq ) );
    
    // puts shifted samples into the synthesisBuffer, from sample indices 0 to numSamples-1
    esola (inputAudio, epochIndices, numOfEpochsPerFrame, shiftingRatio);
    
    // midi velocity gain
    synthesisBuffer.applyGainRamp (0, numSamples, prevVelocityMultiplier, currentVelocityMultiplier);
    prevVelocityMultiplier = currentVelocityMultiplier;
    
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
                                         const Array<int>& epochIndices, const int numOfEpochsPerFrame,
                                         const float shiftingRatio)
{
    const int numSamples = inputAudio.getNumSamples();
    
    int targetLength = 0;
    int highestIndexWrittenTo = -1;
    
    int lastEpochIndex = epochIndices.getUnchecked(0);
    
    finalWindow.clearQuick();
    synthesisBuffer.clear();
    
    for(int i = 0; i < epochIndices.size() - numOfEpochsPerFrame; ++i)
    {
        const int hop = epochIndices.getUnchecked(i + 1) - lastEpochIndex;
        
        if(targetLength >= highestIndexWrittenTo)
        {
            const int frameLength = ( (i + numOfEpochsPerFrame < epochIndices.size()) ?
                                      epochIndices.getUnchecked(i + numOfEpochsPerFrame) : numSamples ) - epochIndices.getUnchecked(i) - 1;
            const int bufferIncrease = frameLength - highestIndexWrittenTo + lastEpochIndex;
            fillWindowBuffer(frameLength);
            
            if(bufferIncrease > 0)
            {
                const auto* reading = inputAudio.getReadPointer(0);
                      auto* writing = synthesisBuffer.getWritePointer(0);
                int writingindex = highestIndexWrittenTo + 1;
                int readingindex = epochIndices.getUnchecked(i) + frameLength - 1 - bufferIncrease;
                int windowindex  = frameLength - 1 - bufferIncrease;
                const auto* windowreading = window.getReadPointer(0);
                
                for(int s = 0; s < bufferIncrease; ++s)
                {
                    writing[writingindex] = reading[readingindex] * windowreading[s];
                    finalWindow.add(windowreading[windowindex]);
                    ++writingindex;
                    ++readingindex;
                    ++windowindex;
                }
                
                highestIndexWrittenTo += (frameLength - 1);
            }
            
            // OLA
            int olaindex  = epochIndices.getUnchecked(i);
            int wolaindex = 0;
            const auto* reading = synthesisBuffer.getReadPointer(0);
                  auto* writing = synthesisBuffer.getWritePointer(0);
            
            for(int s = lastEpochIndex; s < lastEpochIndex + frameLength - bufferIncrease; ++s)
            {
                writing[s] = reading[s] + reading[olaindex];
                
                if(s < finalWindow.size())
                    finalWindow.set(s,
                                    finalWindow.getUnchecked(s) + ( (wolaindex < finalWindow.size()) ? finalWindow.getUnchecked(wolaindex) : 0) );
                else
                    finalWindow.add( ((wolaindex < finalWindow.size()) ? finalWindow.getUnchecked(wolaindex) : 0) );
                
                ++olaindex;
                ++wolaindex;
            }
            
            lastEpochIndex += hop;
        }
        
        targetLength += ceil(hop * shiftingRatio);
    }
    
    // normalize & write to output
    const auto* r = synthesisBuffer.getReadPointer(0);
          auto* w = synthesisBuffer.getWritePointer(0);
    
    for(int s = 0; s < numSamples; ++s)
        w[s] = r[s] / ( (s < finalWindow.size()) ? (std::max<SampleType>(finalWindow.getUnchecked(s), 1e-4)) : 1e-4 );
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
    prevPanningMults[0] = panningMults[0];
    prevPanningMults[1] = panningMults[1];
    
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
    return isVoiceActive() && ! (keyIsDown || parent->isSostenutoPedalDown() || parent->isSustainPedalDown());
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    latchIsOn(false), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), lowestPannedNote(0), lastPitchWheelValue(64), pedalPitchIsOn(false), lastPedalPitch(-1), pedalPitchUpperThresh(0), pedalPitchInterval(12), descantIsOn(false), lastDescantPitch(-1), descantLowerThresh(127), descantInterval(12),
    velocityConverter(100), pitchConverter(440, 69, 12), bendTracker(2, 2),
    adsrIsOn(true), lastMidiTimeStamp(0), lastMidiChannel(1), sustainPedalDown(false), sostenutoPedalDown(false)
{
    adsrParams.attack  = 0.035f;
    adsrParams.decay   = 0.06f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.01f;
    
    quickReleaseParams.attack  = 0.01f;
    quickReleaseParams.decay   = 0.005f;
    quickReleaseParams.sustain = 1.0f;
    quickReleaseParams.release = 0.015f;
    
    quickAttackParams.attack  = 0.015f;
    quickAttackParams.decay   = 0.01f;
    quickAttackParams.sustain = 1.0f;
    quickAttackParams.release = 0.015f;
    
    updateStereoWidth(100);
    setConcertPitchHz(440);
    setCurrentPlaybackSampleRate(44100.0);
};

template<typename SampleType>
Harmonizer<SampleType>::~Harmonizer()
{
    voices.clear();
};

template<typename SampleType>
void Harmonizer<SampleType>::clearBuffers()
{
    for (auto* voice : voices)
        voice->clearBuffers(); 
   
    epochIndices.clearQuick();
    slicedEpochIndices.clearQuick();
};


template<typename SampleType>
void Harmonizer<SampleType>::prepare (const int blocksize)
{
    epochIndices.ensureStorageAllocated(blocksize);
    epochIndices.clearQuick();
    
    slicedEpochIndices.ensureStorageAllocated(blocksize);
    slicedEpochIndices.clearQuick();
    
    aggregateMidiBuffer.ensureSize(ceil(blocksize * 1.5f));
    
    newMaxNumVoices(voices.size());
    
    for(auto* voice : voices)
        voice->prepare(blocksize);
    
    epochs.prepare(blocksize);
    pitch .prepare(blocksize);
};



template<typename SampleType>
void Harmonizer<SampleType>::newMaxNumVoices(const int newMaxNumVoices)
{
    currentlyActiveNotes.ensureStorageAllocated(newMaxNumVoices);
    currentlyActiveNotes.clearQuick();
    
    currentlyActiveNoReleased.ensureStorageAllocated(newMaxNumVoices);
    currentlyActiveNoReleased.clearQuick();
    
    unLatched.ensureStorageAllocated(newMaxNumVoices);
    unLatched.clearQuick();
    
    latchManager.newMaxNumVoices(newMaxNumVoices);
    
    panner.prepare(newMaxNumVoices);
};



template<typename SampleType>
void Harmonizer<SampleType>::releaseResources()
{
    epochIndices.clear();
    currentlyActiveNotes.clear();
    currentlyActiveNoReleased.clear();
    unLatched.clear();
    aggregateMidiBuffer.clear();
    
    for(auto* voice : voices)
        voice->releaseResources();
    
    epochs.releaseResources();
    pitch .releaseResources();
    panner.releaseResources();
    
    latchManager.reset();
};


template<typename SampleType>
void Harmonizer<SampleType>::processMidi (MidiBuffer& midiMessages)
{
    aggregateMidiBuffer.clear();
    
    auto midiIterator = midiMessages.findNextSamplePosition(0);
    
    if (midiIterator == midiMessages.cend())
    {
        lastMidiTimeStamp = -1;
        return;
    }
    
    lastMidiTimeStamp = 0;
    
    std::for_each (midiIterator,
                   midiMessages.cend(),
                   [&] (const MidiMessageMetadata& meta)
                       { handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
        
    midiMessages.swapWith (aggregateMidiBuffer);
    
    lastMidiTimeStamp = -1;
};




// audio rendering-----------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::renderVoices (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer)
{
    outputBuffer.clear(); // outputBuffer will be a subset of samples of the AudioProcessor's "wetBuffer", which will contain the previous sample values from the last frame's output when passed into this method, so we clear the proxy buffer before processing.
    
    epochs.extractEpochSampleIndices (inputAudio, sampleRate, epochIndices);
    
    currentInputFreq = pitch.getPitch (inputAudio, sampleRate); // do some kind of time curve for changes in pitch during block...?
    
    // get subset of epochIndices array accounting for sample offset # numSamples
    
    //jassert (currentInputFreq > 0);
    
    currentInputFreq = 1;
    
    epochs.makeSubsetOfEpochIndicesArray (epochIndices, slicedEpochIndices, 0, inputAudio.getNumSamples());
    
    // retrieve pitch for this precise chunk within the buffer passed to analyzeInput()
    // using sampleOffsetFromOriginalAnalyzedBuffer & inputAudio.getNumSamples()
    
    const int averageDistanceBetweenEpochs = epochs.averageDistanceBetweenEpochs (slicedEpochIndices);
    
    const int periodInSamples = ceil ( (1 / currentInputFreq) * sampleRate );
    
    int numOfEpochsPerFrame = (averageDistanceBetweenEpochs >= periodInSamples) ?
                               3 :
                               ceil(periodInSamples / averageDistanceBetweenEpochs) * 3;
    
    if (slicedEpochIndices.size() < numOfEpochsPerFrame)
        numOfEpochsPerFrame = slicedEpochIndices.size();
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->renderNextBlock (inputAudio, outputBuffer, slicedEpochIndices, numOfEpochsPerFrame, currentInputFreq);
};





template<typename SampleType>
int Harmonizer<SampleType>::getNumActiveVoices() const
{
    int actives = 0;
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            ++actives;
    
    return actives;
};

template<typename SampleType>
bool Harmonizer<SampleType>::isPitchActive(const int midiPitch, const bool countRingingButReleased) const
{
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiPitch)
        {
            if (countRingingButReleased)
                return true;
            
            if (! voice->isPlayingButReleased())
                return true;
        }
    }
    
    return false;
};

template<typename SampleType>
void Harmonizer<SampleType>::setCurrentPlaybackSampleRate(const double newRate)
{
    jassert (newRate > 0);
    
    if (sampleRate == newRate)
        return;

    const ScopedLock sl (lock);
    
    sampleRate = newRate;
    
    for (auto* voice : voices)
        voice->updateSampleRate(newRate);
};

template<typename SampleType>
void Harmonizer<SampleType>::setConcertPitchHz(const int newConcertPitchhz)
{
    jassert (newConcertPitchhz > 0);
    
    if (pitchConverter.getCurrentConcertPitchHz() == newConcertPitchhz)
        return;

    const ScopedLock sl (lock);
    
    pitchConverter.setConcertPitchHz(newConcertPitchhz);
    
    for (auto* voice : voices)
        if(voice->isVoiceActive())
           voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};


// stereo width -------------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateStereoWidth(const int newWidth)
{
    jassert(isPositiveAndBelow(newWidth, 101));
    
    if (panner.getCurrentStereoWidth() == newWidth)
        return;

    const ScopedLock sl (lock);
    
    panner.updateStereoWidth(newWidth);
    
    for (auto* voice : voices)
    {
        if(voice->isVoiceActive())
        {
            if(voice->getCurrentlyPlayingNote() >= lowestPannedNote)
                voice->setPan(panner.getClosestNewPanValFromOld(voice->getCurrentMidiPan()));
            else if(voice->getCurrentMidiPan() != 64)
                voice->setPan(64);
        }
    }
};

template<typename SampleType>
void Harmonizer<SampleType>::updateLowestPannedNote(const int newPitchThresh) noexcept
{
    if (lowestPannedNote == newPitchThresh)
        return;

    const ScopedLock sl (lock);
    
    lowestPannedNote = newPitchThresh;
    
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive())
        {
            if (voice->getCurrentlyPlayingNote() < newPitchThresh)
            {
                if (voice->getCurrentMidiPan() != 64)
                    voice->setPan(64);
            }
            else
            {
                if (voice->getCurrentMidiPan() == 64)
                    voice->setPan(panner.getNextPanVal());
            }
        }
    }
};


// MIDI events --------------------------------------------------------------------------------------------------------------------------------------


// this function is called any time the harmonizer's overall pitch collection is changed. With standard keyboard input, this would be once every note event; for chord triggering, this should be called once after the new chord note on/off events have been handled.
template<typename SampleType>
void Harmonizer<SampleType>::pitchCollectionChanged()
{
    if (pedalPitchIsOn)
        applyPedalPitch();
    
    if (descantIsOn)
        applyDescant();
};

// report active notes --------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::reportActiveNotes(Array<int>& outputArray) const
{
    outputArray.clearQuick();
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            outputArray.add (voice->getCurrentlyPlayingNote());
    
    if (! outputArray.isEmpty())
        outputArray.sort();
};

template<typename SampleType>
void Harmonizer<SampleType>::reportActivesNoReleased(Array<int>& outputArray) const
{
    outputArray.clearQuick();
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && (! (voice->isPlayingButReleased())))
            outputArray.add(voice->getCurrentlyPlayingNote());
    
    if (! outputArray.isEmpty())
        outputArray.sort();
};


// midi velocity sensitivity ---------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateMidiVelocitySensitivity(const int newSensitivity)
{
    const float newSens = newSensitivity/100.0f;
    
    if (velocityConverter.getCurrentSensitivity() == newSens)
        return;

    const ScopedLock sl (lock);
    
    velocityConverter.setFloatSensitivity(newSens);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setVelocityMultiplier(velocityConverter.floatVelocity(voice->getLastRecievedVelocity()));
};


// midi latch ------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::setMidiLatch(const bool shouldBeOn, const bool allowTailOff)
{
    if (latchIsOn == shouldBeOn)
        return;
    
    latchIsOn = shouldBeOn;
    
    if (shouldBeOn)
        return;

    latchManager.turnOffLatch(unLatched);
    
    if (! unLatched.isEmpty())
        turnOffList (unLatched, !allowTailOff, allowTailOff, false);
};


template<typename SampleType>
void Harmonizer<SampleType>::turnOnList (const Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    if (toTurnOn.isEmpty())
        return;
    
    const ScopedLock sl (lock);
    
    for (int i = 0; i < toTurnOn.size(); ++i)
        noteOn (toTurnOn.getUnchecked(i), velocity, false, true);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


template<typename SampleType>
void Harmonizer<SampleType>::turnOffList(const Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord)
{
    if (toTurnOff.isEmpty())
        return;
    
    const ScopedLock sl (lock);
    
    for (int i = 0; i < toTurnOff.size(); ++i)
        noteOff (toTurnOff.getUnchecked(i), velocity, allowTailOff, true, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


template<typename SampleType>
void Harmonizer<SampleType>::playChord (Array<int>& desiredPitches, const float velocity, const bool allowTailOffOfOld)
{
    const ScopedLock sl (lock);
    
    // turn off the pitches that were previously on that are not included in the list of desired pitches
    
    reportActivesNoReleased (currentlyActiveNoReleased);
    
    currentlyActiveNoReleased.removeValuesNotIn (desiredPitches);
   
    turnOffList (currentlyActiveNoReleased, !allowTailOffOfOld, allowTailOffOfOld, true);
    
    
    // turn on the desired pitches that aren't already on
    
    reportActivesNoReleased (currentlyActiveNoReleased);
    
    desiredPitches.removeValuesIn (currentlyActiveNoReleased);
    
    turnOnList (desiredPitches, velocity, true);
    
    pitchCollectionChanged();
};



// functions for propogating midi events to HarmonizerVoices -----------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::handleMidiEvent(const MidiMessage& m, const int samplePosition)
{
    bool shouldAddToAggregateMidiBuffer = true;
    lastMidiChannel   = m.getChannel();
    lastMidiTimeStamp = samplePosition;
    
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity(), true, false);
    else if (m.isNoteOff())
    {
        noteOff (m.getNoteNumber(), m.getFloatVelocity(), true, false, true);
        shouldAddToAggregateMidiBuffer = (! latchIsOn);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        allNotesOff (false);
    else if (m.isPitchWheel())
        handlePitchWheel (m.getPitchWheelValue());
    else if (m.isAftertouch())
        handleAftertouch (m.getNoteNumber(), m.getAfterTouchValue());
    else if (m.isChannelPressure())
        handleChannelPressure (m.getChannelPressureValue());
    else if (m.isController())
        handleController (m.getControllerNumber(), m.getControllerValue());
    
    if (shouldAddToAggregateMidiBuffer)
        aggregateMidiBuffer.addEvent (m, samplePosition);
};

template<typename SampleType>
void Harmonizer<SampleType>::noteOn(const int midiPitch, const float velocity, const bool isKeyboard, const bool partOfList)
{
    // N.B. the `isKeyboard` flag should be true if this note on event was triggered directly from the midi keyboard input; this flag is false if this note on event was triggered automatically by pedal pitch or descant.
    
    const ScopedLock sl (lock);
    
    for (auto* voice : voices)
    {
        if ( (voice->getCurrentlyPlayingNote() != midiPitch)
             || (! voice->isVoiceActive()) )
            continue;
        
        if (! voice->isPlayingButReleased())
            return; // there should be only one instance of a midi note playing at a time...
        
        break;
    }
    
    startVoice (findFreeVoice(midiPitch, shouldStealNotes), midiPitch, velocity, isKeyboard);
    
    if (latchIsOn)
        latchManager.noteOnRecieved(midiPitch);
    
    const bool isAutomatedEvent = ( (! isKeyboard) || partOfList );
        
    if (isAutomatedEvent)
        aggregateMidiBuffer.addEvent (MidiMessage::noteOn(lastMidiChannel, midiPitch, velocity), ++lastMidiTimeStamp);
    else
        pitchCollectionChanged(); // apply pedal pitch / descant
};

template<typename SampleType>
void Harmonizer<SampleType>::startVoice(HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard)
{
    if (! voice)
        return;
    
    voice->setNoteOnTime (++lastNoteOnCounter);
    
    if (! voice->isKeyDown()) // if the key wasn't already marked as down...
        voice->setKeyDown (isKeyboard); // then mark it as down IF this start command is because of a keyboard event
    
    if (midiPitch < lowestPannedNote)
        voice->setPan(64);
    else if (! voice->isVoiceActive())
        voice->setPan(panner.getNextPanVal());
    
    voice->startNote (midiPitch, velocity);
};

template<typename SampleType>
void Harmonizer<SampleType>::noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool partofList, const bool isKeyboard)
{
    // N.B. the `isKeyboard` flag should be true if this note off event was triggered directly from the midi keyboard input; this flag is false if this note off event was triggered automatically by pedal pitch or descant.
    // N.B. the `partofList` flag should be false if this is a singular note off event; this flag should be true if this is a note off event in a known sequence of many quick note offs, so that pedal pitch & descant will not be updated after every single one of these events.
    
    const ScopedLock sl (lock);
    
    if (latchIsOn && isKeyboard)
    {
        latchManager.noteOffRecieved(midiNoteNumber);
        return;
    }
    
    const bool isAutomatedEvent = ( (! isKeyboard) || partofList );
    
    bool stoppedVoice = false;
    
    for (auto* voice : voices)
    {
        if (voice->getCurrentlyPlayingNote() != midiNoteNumber)
            continue;
        
        if (isAutomatedEvent)
        { // this is an "auto- note-off" : ie, it's part of an automated list of notes to turn off, or it's coming from pedal pitch/descant and not the keyboard.
          // check that the note being auto-turned off is note still being held down by a physical keyboard key, and if it's not, then turn it off without checking the state of the sustain or sostenuto pedal.
            if (! voice->isKeyDown())
            {
                stopVoice (voice, velocity, allowTailOff);
                stoppedVoice = true;
            }
        }
        else
        {
            voice->setKeyDown (false);
            
            if (! (sustainPedalDown || sostenutoPedalDown) )
            {
                stopVoice (voice, velocity, allowTailOff);
                stoppedVoice = true;
            }
        }
        
        break; // there should only be one instance of a midi note playing at a time, so we can save time here by breaking the loop once we've found it & taken the appropriate action
    }
    
    if (! stoppedVoice)
        return;

    if (midiNoteNumber == lastDescantPitch)
        lastDescantPitch = -1;
    
    if (midiNoteNumber == lastPedalPitch)
        lastPedalPitch = -1;
    
    if (latchIsOn)
    {
        latchManager.noteOffRecieved(midiNoteNumber);
        return;
    }
    
    if (isAutomatedEvent)
        aggregateMidiBuffer.addEvent (MidiMessage::noteOff(lastMidiChannel, midiNoteNumber, velocity), ++lastMidiTimeStamp);
    else
        pitchCollectionChanged(); // apply descant / pedal pitch
};

template<typename SampleType>
void Harmonizer<SampleType>::stopVoice (HarmonizerVoice<SampleType>* voice, const float velocity, const bool allowTailOff)
{
    if (! voice)
        return;

    voice->stopNote (velocity, allowTailOff);
};

template<typename SampleType>
void Harmonizer<SampleType>::allNotesOff(const bool allowTailOff)
{
    const ScopedLock sl (lock);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->stopNote (1.0f, allowTailOff);
    
    latchManager.reset();
    panner.reset(false);
    lastDescantPitch = -1;
    lastPedalPitch   = -1;
};



template<typename SampleType>
void Harmonizer<SampleType>::handlePitchWheel(const int wheelValue)
{
    if (lastPitchWheelValue == wheelValue)
        return;

    const ScopedLock sl (lock);
    
    lastPitchWheelValue = wheelValue;
    bendTracker.newPitchbendRecieved(wheelValue);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};

template<typename SampleType>
void Harmonizer<SampleType>::updatePitchbendSettings(const int rangeUp, const int rangeDown)
{
    if ((bendTracker.getCurrentRangeUp() == rangeUp) && (bendTracker.getCurrentRangeDown() == rangeDown))
        return;

    bendTracker.setRange(rangeUp, rangeDown);
    
    if (lastPitchWheelValue == 64)
        return;

    const ScopedLock sl (lock);
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};

template<typename SampleType>
void Harmonizer<SampleType>::handleAftertouch(const int midiNoteNumber, const int aftertouchValue)
{
    const ScopedLock sl (lock);
    
    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            voice->aftertouchChanged (aftertouchValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleChannelPressure(const int channelPressureValue)
{
    const ScopedLock sl (lock);
    
    for (auto* voice : voices)
        voice->aftertouchChanged(channelPressureValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleController(const int controllerNumber, const int controllerValue)
{
    switch (controllerNumber)
    {
        case 0x1:   handleModWheel        (controllerValue);        return;
        case 0x2:   handleBreathController(controllerValue);        return;
        case 0x4:   handleFootController  (controllerValue);        return;
        case 0x5:   handlePortamentoTime  (controllerValue);        return;
        case 0x7:   handleMainVolume      (controllerValue);        return;
        case 0x8:   handleBalance         (controllerValue);        return;
        case 0x40:  handleSustainPedal    (controllerValue >= 64);  return;
        case 0x42:  handleSostenutoPedal  (controllerValue >= 64);  return;
        case 0x43:  handleSoftPedal       (controllerValue >= 64);  return;
        case 0x44:  handleLegato          (controllerValue >= 64);  return;
        default:    return;
    }
};

template<typename SampleType>
void Harmonizer<SampleType>::handleSustainPedal(const bool isDown)
{
    if (sustainPedalDown == isDown)
        return;
    
    const ScopedLock sl (lock);
    
    sustainPedalDown = isDown;
    
    if (isDown)
        return;

    for (auto* voice : voices)
        if (! voice->isKeyDown())
            stopVoice (voice, 1.0f, true);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleSostenutoPedal(const bool isDown)
{
    if (sostenutoPedalDown == isDown)
        return;
    
    const ScopedLock sl (lock);
    
    sostenutoPedalDown = isDown;
    
    if (isDown)
        return;

    for (auto* voice : voices)
        if (! voice->isKeyDown())
            stopVoice (voice, 1.0f, true);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleSoftPedal(const bool isDown)
{
    ignoreUnused(isDown);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleModWheel(const int wheelValue)
{
    ignoreUnused(wheelValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBreathController(const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleFootController(const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handlePortamentoTime(const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleMainVolume(const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBalance(const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleLegato(const bool isOn)
{
    ignoreUnused(isOn);
};


// pedal pitch -----------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::applyPedalPitch()
{
    const ScopedLock sl (lock);
    
    const float velocity = 1.0f;
    
    int currentLowest = 128;
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive()
            && (voice->getCurrentlyPlayingNote() < currentLowest)
            && (voice->getCurrentlyPlayingNote() != lastPedalPitch))
        {
            currentLowest = voice->getCurrentlyPlayingNote();
        }
    }
    
    if ((currentLowest == 128)
        || (currentLowest > pedalPitchUpperThresh))
    {
        if (lastPedalPitch > -1)
            noteOff (lastPedalPitch, 1.0f, false, false, false);
        lastPedalPitch = -1;
        return;
    }
    
    const int newPedalPitch = currentLowest - pedalPitchInterval;
    
    if (newPedalPitch == lastPedalPitch)
        return;
    
    if (lastPedalPitch > -1)
        noteOff (lastPedalPitch, 1.0f, false, false, false);
    
    if (newPedalPitch < 0)
    {
        lastPedalPitch = -1;
        return;
    }
    
    if (! isPitchActive (newPedalPitch, false))
    {
        lastPedalPitch = newPedalPitch;
        noteOn (newPedalPitch, velocity, false, false);
    }
    else
        lastPedalPitch = -1;
};

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitch(const bool isOn)
{
    if (pedalPitchIsOn == isOn)
        return;
    
    pedalPitchIsOn = isOn;
    
    if (! isOn)
    {
        if(lastPedalPitch > -1)
            noteOff (lastPedalPitch, 1.0f, false, false, false);
        lastPedalPitch = -1;
    }
    else
        applyPedalPitch();
};

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchUpperThresh(const int newThresh)
{
    if (pedalPitchUpperThresh == newThresh)
        return;
    
    pedalPitchUpperThresh = newThresh;
    
    if (pedalPitchIsOn)
        applyPedalPitch();
};

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchInterval(const int newInterval)
{
    if (pedalPitchInterval == newInterval)
        return;

    pedalPitchInterval = newInterval;
    
    if (pedalPitchIsOn)
        applyPedalPitch();
};

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentPedalPitchVoice()
{
    if (! pedalPitchIsOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && (voice->getCurrentlyPlayingNote() == lastPedalPitch))
            return voice;
    
    return nullptr;
};

// descant ----------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::applyDescant()
{
    const ScopedLock sl (lock);
    
    const float velocity = 1.0f;
    
    int currentHighest = -1;
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive()
            && (voice->getCurrentlyPlayingNote() > currentHighest)
            && (voice->getCurrentlyPlayingNote() != lastDescantPitch))
        {
            currentHighest = voice->getCurrentlyPlayingNote();
        }
    }
    
    if ((currentHighest == -1)
        || (currentHighest < descantLowerThresh))
    {
        if (lastDescantPitch > -1)
            noteOff (lastDescantPitch, 1.0f, false, false, false);
        lastDescantPitch = -1;
        return;
    }
    
    const int newDescantPitch = currentHighest + descantInterval;
    
    if (newDescantPitch == lastDescantPitch)
        return;

    if (lastDescantPitch > -1)
        noteOff (lastDescantPitch, 1.0f, false, false, false);
    
    if (newDescantPitch > 127)
    {
        lastDescantPitch = -1;
        return;
    }
    
    if (! isPitchActive(newDescantPitch, false))
    {
        lastDescantPitch = newDescantPitch;
        noteOn (newDescantPitch, velocity, false, false);
    }
    else
        lastDescantPitch = -1; // if we get here, the new descant pitch is already on, being played by a MIDI keyboard key
};

template<typename SampleType>
void Harmonizer<SampleType>::setDescant(const bool isOn)
{
    if (descantIsOn == isOn)
        return;

    descantIsOn = isOn;
    
    if (! isOn)
    {
        if (lastDescantPitch > -1)
            noteOff (lastDescantPitch, 1.0f, false, false, false);
        lastDescantPitch = -1;
    }
    else
        applyDescant();
};

template<typename SampleType>
void Harmonizer<SampleType>::setDescantLowerThresh(const int newThresh)
{
    if (descantLowerThresh == newThresh)
        return;

    descantLowerThresh = newThresh;
    
    if (descantIsOn)
        applyDescant();
};

template<typename SampleType>
void Harmonizer<SampleType>::setDescantInterval(const int newInterval)
{
    if (descantInterval == newInterval)
        return;

    descantInterval = newInterval;
    
    if (descantIsOn)
        applyDescant();
};

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentDescantVoice()
{
    if (! descantIsOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && (voice->getCurrentlyPlayingNote() == lastDescantPitch))
            return voice;
    
    return nullptr;
};


// voice allocation----------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);
    
    for (auto* voice : voices)
        if (! voice->isVoiceActive())
            return voice;
    
    if (stealIfNoneAvailable)
        return findVoiceToSteal (midiNoteNumber);
    
    return nullptr;
};

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findVoiceToSteal (const int midiNoteNumber) const
{
    // This voice-stealing algorithm applies the following heuristics:
    // - Re-use the oldest notes first
    // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.
    
    jassert (! voices.isEmpty());
    
    // These are the voices we want to protect (ie: only steal if unavoidable)
    HarmonizerVoice<SampleType>* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
    HarmonizerVoice<SampleType>* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase
    
    // this is a list of voices we can steal, sorted by how long they've been running
    Array<HarmonizerVoice<SampleType>*> usableVoices;
    usableVoices.ensureStorageAllocated (voices.size());
    
    for (auto* voice : voices)
    {
        if(voice->isVoiceActive())
        {
            usableVoices.add (voice);
            
            // NB: Using a functor rather than a lambda here due to scare-stories about compilers generating code containing heap allocations..
            struct Sorter
            {
                bool operator() (const HarmonizerVoice<SampleType>* a, const HarmonizerVoice<SampleType>* b) const noexcept { return a->wasStartedBefore (*b); }
            };
            
            std::sort (usableVoices.begin(), usableVoices.end(), Sorter());
            
            if (! voice->isPlayingButReleased()) // Don't protect released notes
            {
                auto note = voice->getCurrentlyPlayingNote();
                
                if (low == nullptr || note < low->getCurrentlyPlayingNote())
                    low = voice;
                
                if (top == nullptr || note > top->getCurrentlyPlayingNote())
                    top = voice;
            }
        }
    }
    
    // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
    if (top == low)
        top = nullptr;
    
    // The oldest note that's playing with the target pitch is ideal..
    for (auto* voice : usableVoices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            return voice;
    
    // Oldest voice that has been released (no finger on it and not held by sustain pedal)
    for (auto* voice : usableVoices)
        if (voice != low && voice != top && voice->isPlayingButReleased())
            return voice;
    
    // Oldest voice that doesn't have a finger on it:
    for (auto* voice : usableVoices)
        if (voice != low && voice != top && (! voice->isKeyDown()))
            return voice;
    
    // Oldest voice that isn't protected
    for (auto* voice : usableVoices)
        if (voice != low && voice != top)
            return voice;
    
    // Duophonic synth: give priority to the bass note:
    if (top != nullptr)
        return top;
    
    return low;
};


// update ADSR settings------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateADSRsettings(const float attack, const float decay, const float sustain, const float release)
{
    // attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0

    adsrParams.attack  = attack;
    adsrParams.decay   = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    
    for (auto* voice : voices)
        voice->setAdsrParameters(adsrParams);
};

template<typename SampleType>
void Harmonizer<SampleType>::updateQuickReleaseMs(const int newMs)
{
    jassert(newMs > 0);
    
    const float desiredR = newMs / 1000.0f;
    
    if (quickReleaseParams.release == desiredR)
        return;

    const ScopedLock sl (lock);
    
    quickReleaseParams.release = desiredR;
    quickAttackParams .release = desiredR;
    
    for(auto* voice : voices)
    {
        voice->setQuickReleaseParameters(quickReleaseParams);
        voice->setQuickAttackParameters(quickAttackParams);
    }
};

template<typename SampleType>
void Harmonizer<SampleType>::updateQuickAttackMs(const int newMs)
{
    jassert(newMs > 0);
    
    const float desiredA = newMs / 1000.0f;
    
    if (quickAttackParams.attack == desiredA)
        return;

    const ScopedLock sl (lock);
    
    quickAttackParams .attack = desiredA;
    quickReleaseParams.attack = desiredA;
    
    for(auto* voice : voices)
    {
        voice->setQuickAttackParameters(quickAttackParams);
        voice->setQuickReleaseParameters(quickReleaseParams);
    }
};


// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::addVoice(HarmonizerVoice<SampleType>* newVoice)
{
    const ScopedLock sl (lock);
    
    panner.setNumberOfVoices(voices.size() + 1);
    
    return voices.add(newVoice);
};



template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getVoicePlayingNote (const int midiPitch)
{
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        if (voice->getCurrentlyPlayingNote() == midiPitch)
            return voice;
    }
    
    return nullptr;
};




template<typename SampleType>
void Harmonizer<SampleType>::removeNumVoices(const int voicesToRemove)
{
    const ScopedLock sl (lock);
    
    int voicesRemoved = 0;
    while(voicesRemoved < voicesToRemove)
    {
        int indexToRemove = -1;
        for (auto* voice : voices)
        {
            if (! voice->isVoiceActive())
            {
                indexToRemove = voices.indexOf(voice);
                break;
            }
        }
        
        const int indexRemoving = std::max(indexToRemove, 0);
        
        HarmonizerVoice<SampleType>* removing = voices[indexRemoving];
        if (removing->isVoiceActive())
        {
            panner.panValTurnedOff(removing->getCurrentMidiPan());
            if (latchIsOn && latchManager.isNoteLatched(removing->getCurrentlyPlayingNote()))
                latchManager.noteOffRecieved(removing->getCurrentlyPlayingNote());
        }
        
        voices.remove(indexRemoving);
        
        ++voicesRemoved;
    }
    
    panner.setNumberOfVoices (std::max (voices.size(), 1));
};



template class Harmonizer<float>;
template class Harmonizer<double>;

