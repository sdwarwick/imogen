/*
 ==============================================================================
 
 Harmonizer.cpp
 Created: 13 Dec 2020 7:53:39pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#include "Harmonizer.h"


template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    latchIsOn(false), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), lowestPannedNote(0), lastPitchWheelValue(64), pedalPitchIsOn(false), lastPedalPitch(-1), pedalPitchUpperThresh(0), pedalPitchInterval(12), descantIsOn(false), lastDescantPitch(-1), descantLowerThresh(127), descantInterval(12),
    velocityConverter(100), pitchConverter(440, 69, 12), bendTracker(2, 2),
    adsrIsOn(true), lastMidiTimeStamp(0), lastMidiChannel(1), sustainPedalDown(false), sostenutoPedalDown(false), softPedalDown(false)
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
    
    windowSize = 0;
    
    intervalLatchIsOn = false;
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
};


template<typename SampleType>
void Harmonizer<SampleType>::prepare (const int blocksize)
{
    aggregateMidiBuffer.ensureSize(ceil(blocksize * 1.5f));
    
    newMaxNumVoices(voices.size());
    
    for(auto* voice : voices)
        voice->prepare(blocksize);
    
    windowBuffer.setSize (1, blocksize * 2, true, true, true);
    
    indicesOfGrainOnsets.ensureStorageAllocated(blocksize);
    peakIndices.ensureStorageAllocated(blocksize);
    
    intervalsLatchedTo.ensureStorageAllocated(voices.size());
};


template<typename SampleType>
void Harmonizer<SampleType>::setCurrentPlaybackSampleRate (const double newRate)
{
    jassert (newRate > 0);
    
    if (sampleRate == newRate)
        return;
    
    const ScopedLock sl (lock);
    
    sampleRate = newRate;
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        voice->updateSampleRate(newRate);
};


template<typename SampleType>
void Harmonizer<SampleType>::setConcertPitchHz (const int newConcertPitchhz)
{
    jassert (newConcertPitchhz > 0);
    
    if (pitchConverter.getCurrentConcertPitchHz() == newConcertPitchhz)
        return;
    
    const ScopedLock sl (lock);
    
    pitchConverter.setConcertPitchHz(newConcertPitchhz);
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        if(voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};



template<typename SampleType>
void Harmonizer<SampleType>::newMaxNumVoices(const int newMaxNumVoices)
{
    currentlyActiveNotes.ensureStorageAllocated(newMaxNumVoices);
    currentlyActiveNotes.clearQuick();
    
    currentlyActiveNoReleased.ensureStorageAllocated(newMaxNumVoices);
    currentlyActiveNoReleased.clearQuick();
    
    panner.prepare(newMaxNumVoices);
    
    intervalsLatchedTo.ensureStorageAllocated(newMaxNumVoices);
};



template<typename SampleType>
void Harmonizer<SampleType>::releaseResources()
{
    currentlyActiveNotes.clear();
    currentlyActiveNoReleased.clear();
    aggregateMidiBuffer.clear();
    
    for(auto* voice : voices)
        voice->releaseResources();
    
    panner.releaseResources();
};


template<typename SampleType>
void Harmonizer<SampleType>::setCurrentInputFreq (const SampleType newInputFreq)
{
    currentInputFreq = newInputFreq;
    
    currentInputPeriod = roundToInt (sampleRate / newInputFreq);
    
    fillWindowBuffer (currentInputPeriod * 2);
    
    if (intervalLatchIsOn)
    {
        const int currentMidiPitch = roundToInt (pitchConverter.ftom (currentInputFreq));
        
        currentlyActiveNotes.clearQuick();
        
        for (int i = 0; i < intervalsLatchedTo.size(); ++i)
            currentlyActiveNotes.add (currentMidiPitch + intervalsLatchedTo.getUnchecked(i));
        
        playChord (currentlyActiveNotes, 1.0f, false);
    }
};

/****************************************************************************************************************************************************
// audio rendering-----------------------------------------------------------------------------------------------------------------------------------
 ***************************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::renderVoices (const AudioBuffer<SampleType>& inputAudio,
                                           AudioBuffer<SampleType>& outputBuffer,
                                           const bool frameIsPitched)
{
    outputBuffer.clear();
    
    if (inputAudio.getNumSamples() == 0)
        return;
    
    // how to handle unpitched frames???
    
    if (windowSize != currentInputPeriod * 2)
        fillWindowBuffer (currentInputPeriod * 2);
    
    extractGrainOnsetIndices (indicesOfGrainOnsets, inputAudio, currentInputPeriod);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->renderNextBlock (inputAudio, outputBuffer, currentInputPeriod, indicesOfGrainOnsets, windowBuffer);
};


// identify analysis grains--------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::extractGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    // PART ONE - find sample indices of points of maximum energy for every pitch period
    // this function identifies a local extrema for every period of the fundamental frequency of the input audio, but these peaks are not garunteed to be spaced evenly, or exhibit any symmetry. The only garuntee is that there are as many peaks are there are full repitions of the signal's period & that each peak represents a local extrema.
    
    extractPeakIndicesForEachPeriod (peakIndices, inputAudio, period);
    
    
    // PART TWO - create array of grain onset indices, such that grains are 2 pitch periods long, centred on points of maximum energy, w/ approx 50% overlap
    
    targetArray.clearQuick();
    
    
    // step 1 - shift from peak indices to grain onset indices
    
    for (int p = 0; p < peakIndices.size(); ++p)
    {
        const int peakIndex = peakIndices.getUnchecked (p);
        
        int grainStart = peakIndex - period; // offset the peak index by the period so that each grain will be centered on a peak
        
        if (grainStart > 0)
            targetArray.add (grainStart);
    }

    
    /*
     step 2 - attempt to preserve/create symmetry such that for all grains grainStart[p]:
     
         continuous grains in the same "stream":
         grainStart[p - 2] = grainStart[p] - (2 * period)
         grainStart[p + 2] = grainStart[p] + (2 * period)
 
         neighboring overlapping grains:
         grainStart[p - 1] = grainStart[p] - period
         grainStart[p + 1] = grainStart[p] + period
     */
    
    const int periodDoubled = 2 * period;
    const int numSamples = inputAudio.getNumSamples();
    
    const int last = targetArray.getLast() + period; // just in case...
    if (last < numSamples)
        targetArray.add (last);
    
    for (int p = 0; p < targetArray.size(); ++p)
    {
        int g = targetArray.getUnchecked (p);
        
        if ((p - 1) >= 0)
        {
            if ((p - 2) >= 0)
            {
                /*
                 this is the neighboring grain in the SAME CONTINUOUS STREAM of grains! The symmetry here is most important
                 shift the CURRENT grain (grainStart[p]) to make this equation true:
                    grainStart[p - 2] = grainStart[p] - 2 * period
                 or
                    grainStart[p] = grainStart[p - 2] + 2 * period
                 */
                
                const int g00 = targetArray.getUnchecked (p - 2);
                
                const int targetG = g00 + periodDoubled;
                
                if (targetG < numSamples && g != targetG)
                {
                    targetArray.set (p, targetG);
                    g = targetG;
                }
            }
            
            // this is the neighboring OVERLAPPING grain - the overlap need not be *exactly* 50%, but keeping it as synchronous as possible is desirable - so we're only correcting this index if the OLA % of the grain's current position is lower than 25% or higher than 75%
            
            const int target_g0 = g - period;
            const int actual_g0 = targetArray.getUnchecked (p - 1);
            
            if (target_g0 != actual_g0 && target_g0 > 0)
            {
                const float olaPcnt = abs(g - actual_g0) / periodDoubled;
            
                if (olaPcnt > 0.75f || olaPcnt < 0.25f)
                    targetArray.set (p - 1, target_g0);
            }
        }
        
        if ((p + 1) < targetArray.size())
        {
            if ((p + 2) < targetArray.size())
            {
                // this is the neighboring grain in the SAME CONTINUOUS STREAM of grains! The symmetry here is most important
                // shift the FUTURE grain (grainStart[p + 2]) to make this equation true: grainStart[p + 2] = grainStart[p] + 2 * period
                
                const int target_g2 = g + periodDoubled;
                
                if ((target_g2 != targetArray.getUnchecked (p + 2)) && (target_g2 < numSamples))
                    targetArray.set (p + 2, target_g2);
            }
            
            // this is the neighboring OVERLAPPING grain - the overlap need not be *exactly* 50%, but keeping it as synchronous as possible is desirable
            
            const int target_g1 = g + period;
            const int actual_g1 = targetArray.getUnchecked (p + 1);
            
            if (target_g1 != actual_g1 && target_g1 < numSamples)
            {
                const float olaPcnt = abs(g - actual_g1) / periodDoubled;
                
                if (olaPcnt > 0.75f || olaPcnt < 0.25f)
                    targetArray.set (p + 1, target_g1);
            }
        }
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::extractPeakIndicesForEachPeriod (Array<int>& targetArray,
                                                              const AudioBuffer<SampleType>& inputAudio,
                                                              const int period)
{
    targetArray.clearQuick();
    
    int analysisIndex = 0;
    
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    while (analysisIndex < inputAudio.getNumSamples())
    {
        SampleType localMin = reading[analysisIndex];
        SampleType localMax = reading[analysisIndex];
        int indexOfLocalMin = analysisIndex;
        int indexOfLocalMax = analysisIndex;
        
        for (int s = analysisIndex + 1;
             s < std::min (analysisIndex + period, inputAudio.getNumSamples());
             ++s)
        {
            const SampleType currentSample = reading[s];
            
            if (currentSample < localMin)
            {
                localMin = currentSample;
                indexOfLocalMin = s;
            }
            
            if (currentSample > localMax)
            {
                localMax = currentSample;
                indexOfLocalMax = s;
            }
        }
        
        const int prevPeak = targetArray.isEmpty() ? 0 : targetArray.getLast();
        
        const int posDelta = abs(indexOfLocalMax - prevPeak - period); // for peaks perfectly spaced 1 period apart, this will be 0
        const int negDelta = abs(indexOfLocalMin - prevPeak - period);
        
        const int peakIndex = (posDelta < negDelta) ? indexOfLocalMax : indexOfLocalMin;
        
        targetArray.add (peakIndex);
        
        analysisIndex = peakIndex + 1;
    }
};


// calculate Hanning window ------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::fillWindowBuffer (const int numSamples)
{
    if (windowSize == numSamples)
        return;
    
    jassert (numSamples <= windowBuffer.getNumSamples());
    
    windowBuffer.clear();
    
    auto* writing = windowBuffer.getWritePointer(0);
    
    const auto samplemultiplier = MathConstants<SampleType>::pi / static_cast<SampleType> (numSamples - 1);

    for (int i = 0; i < numSamples; ++i)
        writing[i] = static_cast<SampleType> (0.5 - 0.5 * (std::cos(static_cast<SampleType> (2.0 * i) * samplemultiplier)) );
    
    windowSize = numSamples;
};



/***************************************************************************************************************************************************
// functions for meta midi & note management -------------------------------------------------------------------------------------------------------
****************************************************************************************************************************************************/

template<typename SampleType>
bool Harmonizer<SampleType>::isPitchActive (const int midiPitch, const bool countRingingButReleased) const
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



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS FOR UPDATING PARAMETERS ----------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// stereo width ---------------------------------------------------------------------------------------------------
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


// midi velocity sensitivity -------------------------------------------------------------------------------------
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


// pitch bend settings -------------------------------------------------------------------------------------------
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


// descant settings -----------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::setDescant(const bool isOn)
{
    if (descantIsOn == isOn)
        return;
    
    descantIsOn = isOn;
    
    if (! isOn)
    {
        if (lastDescantPitch > -1)
            noteOff (lastDescantPitch, 1.0f, false, false);
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


// pedal pitch settings -----------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitch(const bool isOn)
{
    if (pedalPitchIsOn == isOn)
        return;
    
    pedalPitchIsOn = isOn;
    
    if (! isOn)
    {
        if(lastPedalPitch > -1)
            noteOff (lastPedalPitch, 1.0f, false, false);
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


// ADSR settings------------------------------------------------------------------------------------------------------------------------------
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

/****************************************************************************************************************************************************
// voice allocation----------------------------------------------------------------------------------------------------------------------------------
****************************************************************************************************************************************************/

 template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable) 
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
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findVoiceToSteal (const int midiNoteNumber) 
{
    // This voice-stealing algorithm applies the following heuristics:
    // - Re-use the oldest notes first
    // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.
    // - Protects the pedal & descant auto voices, if active, and only releases them as a last resort to avoid stealing the lowest & highest notes being played manually
    
    jassert (! voices.isEmpty());
    
    // These are the voices we want to protect (ie: only steal if unavoidable)
    HarmonizerVoice<SampleType>* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
    HarmonizerVoice<SampleType>* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase
    
    // this is a list of voices we can steal, sorted by how long they've been running
    Array< HarmonizerVoice<SampleType>* > usableVoices;
    usableVoices.ensureStorageAllocated (voices.size());
    
    for (auto* voice : voices)
    {
        if ((pedalPitchIsOn && voice->getCurrentlyPlayingNote() == lastPedalPitch)
            || (descantIsOn && voice->getCurrentlyPlayingNote() == lastDescantPitch))
            continue; // save these, only use if absolutely necessary
        
        usableVoices.add (voice);
        
        // NB: Using a functor rather than a lambda here due to scare-stories about compilers generating code containing heap allocations..
        struct Sorter
        {
            bool operator() (const HarmonizerVoice<SampleType>* a, const HarmonizerVoice<SampleType>* b) const noexcept
            { return a->wasStartedBefore (*b); }
        };
        
        std::sort (usableVoices.begin(), usableVoices.end(), Sorter());
        
        if (voice->isVoiceActive() && ! voice->isPlayingButReleased())
        {
            auto note = voice->getCurrentlyPlayingNote();
            
            if (low == nullptr || note < low->getCurrentlyPlayingNote())
                low = voice;
            
            if (top == nullptr || note > top->getCurrentlyPlayingNote())
                top = voice;
        }
    }
    
    // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
    if (top == low)
        top = nullptr;
    
    for (auto* voice : usableVoices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            return voice;
    
    for (auto* voice : usableVoices)
        if (voice != low && voice != top && voice->isPlayingButReleased())
            return voice;
    
    for (auto* voice : usableVoices)
        if (voice != low && voice != top && ! voice->isKeyDown())
            return voice;
    
    // Oldest voice that isn't protected
    for (auto* voice : usableVoices)
        if (voice != low && voice != top)
            return voice;
    
    // only protected top & bottom voices are left now - time to use the pedal pitch & descant voices...
    
    if (descantIsOn) // give preference to the bass
    {
        lastDescantPitch = -1;
        return getCurrentDescantVoice();
    }
    
    if (pedalPitchIsOn)
    {
        lastPedalPitch = -1;
        return getCurrentPedalPitchVoice();
    }
    
    // Duophonic synth: give priority to the bass note
    if (top != nullptr)
        return top;
    
    return low;
};

/***************************************************************************************************************************************************
// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------
****************************************************************************************************************************************************/

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::addVoice(HarmonizerVoice<SampleType>* newVoice)
{
    const ScopedLock sl (lock);
    
    panner.setNumberOfVoices(voices.size() + 1);
    
    return voices.add(newVoice);
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
            panner.panValTurnedOff(removing->getCurrentMidiPan());
        
        voices.remove(indexRemoving);
        
        ++voicesRemoved;
    }
    
    panner.setNumberOfVoices (std::max (voices.size(), 1));
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getVoicePlayingNote (const int midiPitch) const
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
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentDescantVoice() const
{
    if (! descantIsOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && (voice->getCurrentlyPlayingNote() == lastDescantPitch))
            return voice;
    
    return nullptr;
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentPedalPitchVoice() const
{
    if (! pedalPitchIsOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && (voice->getCurrentlyPlayingNote() == lastPedalPitch))
            return voice;
    
    return nullptr;
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



template class Harmonizer<float>;
template class Harmonizer<double>;

