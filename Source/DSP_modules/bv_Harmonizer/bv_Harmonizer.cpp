/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: Harmonizer
*/

#include "bv_Harmonizer/bv_Harmonizer.h"

#include "bv_Harmonizer/bv_HarmonizerVoice.cpp"
#include "bv_Harmonizer/bv_Harmonizer_VoiceAllocation.cpp"
#include "bv_Harmonizer/bv_Harmonizer_Midi.cpp"
#include "bv_Harmonizer/bv_Harmonizer_AutomatedMidiFeatures.cpp"
#include "bv_Harmonizer/PanningManager/PanningManager.cpp"
#include "bv_Harmonizer/GrainExtractor/GrainExtractor.cpp"
#include "bv_Harmonizer/GrainExtractor/PsolaPeakFinding.cpp"



namespace bav

{


template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    pitchDetector(80, 2400, 44100.0),
    latchIsOn(false), intervalLatchIsOn(false), currentInputFreq(0.0f), sampleRate(44100.0), lastNoteOnCounter(0), lastPitchWheelValue(64), shouldStealNotes(true), lowestPannedNote(0),
    velocityConverter(100), pitchConverter(440, 69, 12), bendTracker(2, 2),
    adsrIsOn(true), lastMidiTimeStamp(0), lastMidiChannel(1), playingButReleasedMultiplier(1.0f), sustainPedalDown(false), sostenutoPedalDown(false), softPedalDown(false), windowSize(0)
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
    
    pedal.isOn = false;
    pedal.lastPitch = -1;
    pedal.upperThresh = 0;
    pedal.interval = 12;
    
    descant.isOn = false;
    descant.lastPitch = -1;
    descant.lowerThresh = 127;
    descant.interval = 12;
}


template<typename SampleType>
Harmonizer<SampleType>::~Harmonizer()
{
    voices.clear();
}


template<typename SampleType>
void Harmonizer<SampleType>::clearBuffers()
{
    for (auto* voice : voices)
        voice->clearBuffers();
    
    polarityReversalBuffer.clear();
}
    
    
template<typename SampleType>
void Harmonizer<SampleType>::initialize (const int initNumVoices, const double initSamplerate, const int initBlocksize)
{
    jassert (initNumVoices > 0 && initSamplerate > 0 && initBlocksize > 0);
    
    voices.clear();
    
    addNumVoices (initNumVoices);
    
    setCurrentPlaybackSampleRate (initSamplerate);
    
    prepare (initBlocksize);
}

 
template<typename SampleType>
void Harmonizer<SampleType>::prepare (const int blocksize)
{
    jassert (blocksize > 0);
    
    aggregateMidiBuffer.ensureSize (static_cast<size_t> (blocksize * 2));
    aggregateMidiBuffer.clear();
    
    for (auto* voice : voices)
        voice->prepare (blocksize);
    
    windowBuffer.setSize (1, blocksize * 2, true, true, true);
    polarityReversalBuffer.setSize (1, blocksize);
    
    indicesOfGrainOnsets.ensureStorageAllocated (blocksize);
    
    grains.prepare (blocksize);
    
    lastNoteOnCounter = 0;
}


template<typename SampleType>
void Harmonizer<SampleType>::setCurrentPlaybackSampleRate (const double newRate)
{
    jassert (newRate > 0);
    
    if (sampleRate == newRate)
        return;
    
    sampleRate = newRate;
    
    pitchDetector.setSamplerate (newRate);
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        voice->updateSampleRate (newRate);
}


template<typename SampleType>
void Harmonizer<SampleType>::setConcertPitchHz (const int newConcertPitchhz)
{
    jassert (newConcertPitchhz > 0);
    
    if (pitchConverter.getCurrentConcertPitchHz() == newConcertPitchhz)
        return;
    
    pitchConverter.setConcertPitchHz (newConcertPitchhz);
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
}


template<typename SampleType>
void Harmonizer<SampleType>::releaseResources()
{
    aggregateMidiBuffer.clear();
    usableVoices.clear();
    polarityReversalBuffer.clear();
    windowBuffer.clear();
    intervalsLatchedTo.clear();
    indicesOfGrainOnsets.clear();
    
    for (auto* voice : voices)
        voice->releaseResources();
    
    panner.releaseResources();
    
    grains.releaseResources();
    
    lastNoteOnCounter = 0;
    
    voices.clear();
}
    

template<typename SampleType>
void Harmonizer<SampleType>::setCurrentInputFreq (const float newInputFreq)
{
    jassert (newInputFreq > 0);
    
    currentInputFreq = newInputFreq;
    
    currentInputPeriod = roundToInt (sampleRate / newInputFreq);
    
    if (intervalLatchIsOn && ! intervalsLatchedTo.isEmpty())
        playIntervalSet (intervalsLatchedTo, 1.0f, false, true);
}


/***********************************************************************************************************************************************
// audio rendering------------------------------------------------------------------------------------------------------------------------------
 **********************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::renderVoices (const AudioBuffer<SampleType>& inputAudio,
                                           AudioBuffer<SampleType>& outputBuffer,
                                           MidiBuffer& midiMessages)
{
    const float inputFrequency = pitchDetector.detectPitch (inputAudio);  // outputs -1 if frame is unpitched
    
    processMidi (midiMessages);
    
    outputBuffer.clear();
    
    if (getNumActiveVoices() == 0)
        return;
    
    const bool frameIsPitched = (inputFrequency >= 0.0f);
    
    int periodThisFrame;
    bool polarityReversed = false;
    
    if (frameIsPitched)
    {
        if (currentInputFreq != inputFrequency)
            setCurrentInputFreq (inputFrequency);
        
        periodThisFrame = currentInputPeriod;
    }
    else
    {
        // for unpitched frames, an arbitrary "period" is imposed on the signal for analysis grain extraction; this arbitrary period is randomized within a certain range
        
        Random& rand = Random::getSystemRandom();
        
        periodThisFrame = rand.nextInt (unpitchedArbitraryPeriodRange);
        
        if ((rand.nextInt (100) % 2) == 0)  // reverse the polarity approx 50% of the time
        {
            FloatVectorOperations::negate (polarityReversalBuffer.getWritePointer(0),
                                           inputAudio.getReadPointer(0),
                                           inputAudio.getNumSamples());
            
            polarityReversed = true;
        }
    }
    
    fillWindowBuffer (periodThisFrame * 2);
    
    const AudioBuffer<SampleType>& actualInput = polarityReversed ? polarityReversalBuffer : inputAudio;
    
    grains.getGrainOnsetIndices (indicesOfGrainOnsets, actualInput, periodThisFrame);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->renderNextBlock (actualInput, outputBuffer, periodThisFrame, indicesOfGrainOnsets, windowBuffer);
}



// calculate Hanning window ------------------------------------------------------

template<typename SampleType>
void Harmonizer<SampleType>::fillWindowBuffer (const int numSamples)
{
    jassert (numSamples > 1);
    
    if (windowSize == numSamples)
        return;
    
    constexpr SampleType zero = SampleType(0.0);
    FloatVectorOperations::fill (windowBuffer.getWritePointer(0), zero, windowBuffer.getNumSamples());
    
    jassert (numSamples <= windowBuffer.getNumSamples());
    
#if BV_HARMONIZER_USE_VDSP
    if constexpr (std::is_same_v <SampleType, float>)
        vDSP_hann_window (windowBuffer.getWritePointer(0), vDSP_Length(numSamples), 2);
    else if constexpr (std::is_same_v <SampleType, double>)
        vDSP_hann_windowD (windowBuffer.getWritePointer(0), vDSP_Length(numSamples), 2);
#else
    const SampleType samplemultiplier = static_cast<SampleType>( (MathConstants<SampleType>::pi * 2.0) / (numSamples - 1) );
    constexpr SampleType one = SampleType(1.0);
    constexpr SampleType pointFive = SampleType(0.5);
    
    auto* writing = windowBuffer.getWritePointer(0);
    
    for (int i = 1; i < numSamples; ++i)
        writing[i] = static_cast<SampleType>( (one - (std::cos (i * samplemultiplier))) * pointFive );
#endif
    
    windowSize = numSamples;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS FOR UPDATING PARAMETERS -----------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// stereo width ---------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateStereoWidth (int newWidth)
{
    newWidth = jlimit (0, 100, newWidth);
    
    if (panner.getCurrentStereoWidth() == newWidth)
        return;
    
    panner.updateStereoWidth (newWidth);
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        const int currentPan = voice->getCurrentMidiPan();
        
        if (voice->getCurrentlyPlayingNote() < lowestPannedNote)
        {
            if (currentPan != 64)
            {
                panner.panValTurnedOff (currentPan);
                voice->setPan (64);
            }
        }
        else
        {
            voice->setPan (panner.getClosestNewPanValFromOld (currentPan));
        }
    }
}


template<typename SampleType>
void Harmonizer<SampleType>::updateLowestPannedNote (int newPitchThresh)
{
    newPitchThresh = jlimit (0, 127, newPitchThresh);
    
    if (lowestPannedNote.load() == newPitchThresh)
        return;
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        const int note = voice->getCurrentlyPlayingNote();
        const int currentPan = voice->getCurrentMidiPan();
    
        if (note < newPitchThresh)
        {
            if (currentPan != 64)
            {
                panner.panValTurnedOff (currentPan);
                voice->setPan (64);
            }
        }
        else if (note < lowestPannedNote.load()) // because we haven't updated the lowestPannedNote member variable yet, voices with pitches higher than newPitchThresh but lower than lowestPannedNote are the voices that now qualify for panning
        {
            if (currentPan == 64)
                voice->setPan (panner.getNextPanVal());
        }
    }
    
    lowestPannedNote.store(newPitchThresh);
}


// midi velocity sensitivity -------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateMidiVelocitySensitivity (int newSensitivity)
{
    newSensitivity = jlimit (0, 100, newSensitivity);
    
    const float newSens = newSensitivity / 100.0f;
    
    if (velocityConverter.getCurrentSensitivity() == newSens)
        return;
    
    velocityConverter.setFloatSensitivity (newSens);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setVelocityMultiplier (velocityConverter.floatVelocity (voice->getLastRecievedVelocity()));
}


// pitch bend settings -------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updatePitchbendSettings (const int rangeUp, const int rangeDown)
{
    if ((bendTracker.getCurrentRangeUp() == rangeUp) && (bendTracker.getCurrentRangeDown() == rangeDown))
        return;
    
    bendTracker.setRange (rangeUp, rangeDown);
    
    if (lastPitchWheelValue == 64)
        return;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
}


// descant settings ----------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::setDescant (const bool isOn)
{
    if (descant.isOn == isOn)
        return;
    
    if (isOn)
        applyDescant();
    else
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        descant.lastPitch = -1;
    }
    
   descant.isOn = isOn;
}
    

template<typename SampleType>
void Harmonizer<SampleType>::setDescantLowerThresh (int newThresh)
{
    newThresh = jlimit (0, 127, newThresh);
    
    if (descant.lowerThresh == newThresh)
        return;
    
    descant.lowerThresh = newThresh;
    
    if (descant.isOn)
        applyDescant();
}
    

template<typename SampleType>
void Harmonizer<SampleType>::setDescantInterval (const int newInterval)
{
    if (descant.interval == newInterval)
        return;
    
    descant.interval = newInterval;
    
    if (newInterval == 0)
        descant.isOn = false;
    
    if (descant.isOn)
        applyDescant();
}


// pedal pitch settings -----------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitch (const bool isOn)
{
    if (pedal.isOn == isOn)
        return;
    
    if (isOn)
        applyPedalPitch();
    else
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        pedal.lastPitch = -1;
    }
    
    pedal.isOn = isOn;
}
    

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchUpperThresh (int newThresh)
{
    newThresh = jlimit (0, 127, newThresh);
    
    if (pedal.upperThresh == newThresh)
        return;
    
    pedal.upperThresh = newThresh;
    
    if (pedal.isOn)
        applyPedalPitch();
}
    

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchInterval (const int newInterval)
{
    if (pedal.interval == newInterval)
        return;
    
    pedal.interval = newInterval;
    
    if (newInterval == 0)
        pedal.isOn = false;
    
    if (pedal.isOn)
        applyPedalPitch();
}


// ADSR settings------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateADSRsettings (const float attack, const float decay, const float sustain, const float release)
{
    // attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
    
    const ScopedLock sl (lock);
    
    adsrParams.attack  = attack;
    adsrParams.decay   = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    
    for (auto* voice : voices)
        voice->setAdsrParameters(adsrParams);
}
    

template<typename SampleType>
void Harmonizer<SampleType>::updateQuickReleaseMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
    const ScopedLock sl (lock);
    
    const float desiredR = newMs / 1000.0f;
    
    if (quickReleaseParams.release == desiredR)
        return;

    quickReleaseParams.release = desiredR;
    quickAttackParams .release = desiredR;
    
    for (auto* voice : voices)
    {
        voice->setQuickReleaseParameters(quickReleaseParams);
        voice->setQuickAttackParameters (quickAttackParams);
    }
}
    

template<typename SampleType>
void Harmonizer<SampleType>::updateQuickAttackMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
    const ScopedLock sl (lock);
    
    const float desiredA = newMs / 1000.0f;
    
    if (quickAttackParams.attack == desiredA)
        return;
    
    quickAttackParams .attack = desiredA;
    quickReleaseParams.attack = desiredA;
    
    for (auto* voice : voices)
    {
        voice->setQuickAttackParameters (quickAttackParams);
        voice->setQuickReleaseParameters(quickReleaseParams);
    }
}



template class Harmonizer<float>;
template class Harmonizer<double>;


} // namespace
