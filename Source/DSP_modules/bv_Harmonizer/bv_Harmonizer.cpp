/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: Harmonizer
 */


#include "bv_Harmonizer/bv_Harmonizer.h"

#include "bv_Harmonizer/bv_HarmonizerVoice.cpp"
#include "bv_Harmonizer/bv_HarmonizerMidi.cpp"
#include "bv_Harmonizer/PanningManager/PanningManager.cpp"
#include "bv_Harmonizer/GrainExtractor/GrainExtractor.cpp"
#include "bv_Harmonizer/GrainExtractor/PsolaPeakFinding.cpp"
#include "bv_Harmonizer/GrainExtractor/ZeroCrossingFinding.cpp"


namespace bav

{


template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    latchIsOn(false), currentInputFreq(0.0f), sampleRate(44100.0), shouldStealNotes(true), lastNoteOnCounter(0), lowestPannedNote(0), lastPitchWheelValue(64),
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
    
    pedal.isOn = false;
    pedal.lastPitch = -1;
    pedal.upperThresh = 0;
    pedal.interval = 12;
    
    descant.isOn = false;
    descant.lastPitch = -1;
    descant.lowerThresh = 127;
    descant.interval = 12;
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
    aggregateMidiBuffer.ensureSize(static_cast<size_t>(blocksize * 2));
    
    newMaxNumVoices(voices.size());
    
    for (auto* voice : voices)
        voice->prepare(blocksize);
    
    windowBuffer.setSize (1, blocksize * 2, true, true, true);
    unpitchedWindow.setSize (1, unpitchedGrainRate * 2, true, true, true);
    calculateHanningWindow (unpitchedWindow, unpitchedGrainRate * 2);
    
    indicesOfGrainOnsets.ensureStorageAllocated(blocksize);
    
    intervalsLatchedTo.ensureStorageAllocated(voices.size());
    
    grains.prepare (blocksize);
    
    lastNoteOnCounter = 0;
};


template<typename SampleType>
void Harmonizer<SampleType>::setCurrentPlaybackSampleRate (const double newRate)
{
    jassert (newRate > 0);
    
    if (sampleRate == newRate)
        return;
    
    sampleRate = newRate;
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        voice->updateSampleRate (newRate);
};


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
};



template<typename SampleType>
void Harmonizer<SampleType>::newMaxNumVoices(const int newMaxNumVoices)
{
    panner.prepare(newMaxNumVoices);
    
    intervalsLatchedTo.ensureStorageAllocated(newMaxNumVoices);
};



template<typename SampleType>
void Harmonizer<SampleType>::releaseResources()
{
    aggregateMidiBuffer.clear();
    
    for (auto* voice : voices)
        voice->releaseResources();
    
    panner.releaseResources();
    
    grains.releaseResources();
    
    lastNoteOnCounter = 0;
};


template<typename SampleType>
void Harmonizer<SampleType>::setCurrentInputFreq (const float newInputFreq)
{
    currentInputFreq = newInputFreq;
    
    currentInputPeriod = juce::roundToInt (sampleRate / newInputFreq);
    
    fillWindowBuffer (currentInputPeriod * 2);
    
    if (intervalLatchIsOn)
        if (! intervalsLatchedTo.isEmpty())
            playChordFromIntervalSet (intervalsLatchedTo);
};


/****************************************************************************************************************************************************
// audio rendering-----------------------------------------------------------------------------------------------------------------------------------
 ***************************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::renderVoices (const AudioBuffer<SampleType>& inputAudio,
                                           AudioBuffer<SampleType>& outputBuffer,
                                           const float inputFrequency, const bool frameIsPitched,
                                           MidiBuffer& midiMessages)
{
    if (frameIsPitched && currentInputFreq != inputFrequency)
        setCurrentInputFreq (inputFrequency);
    
    processMidi (midiMessages);
    
    outputBuffer.clear();
    
    if (getNumActiveVoices() == 0)
        return;
    
    const int periodThisFrame = frameIsPitched ? currentInputPeriod : unpitchedGrainRate;
    
    grains.getGrainOnsetIndices (indicesOfGrainOnsets, inputAudio, periodThisFrame);
    
    AudioBuffer<SampleType>& windowToUse = frameIsPitched ? windowBuffer : unpitchedWindow;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->renderNextBlock (inputAudio, outputBuffer, periodThisFrame, indicesOfGrainOnsets, windowToUse);
};



// calculate Hanning window ------------------------------------------------------

template<typename SampleType>
void Harmonizer<SampleType>::fillWindowBuffer (const int numSamples)
{
    if (windowSize == numSamples)
        return;
    
    jassert (numSamples <= windowBuffer.getNumSamples());
    
    calculateHanningWindow (windowBuffer, numSamples);
    
    windowSize = numSamples;
};


template<typename SampleType>
void Harmonizer<SampleType>::calculateHanningWindow (AudioBuffer<SampleType>& windowToFill, const int numSamples)
{
    windowToFill.clear();
    
    jassert (numSamples > 1);
    
    auto* writing = windowToFill.getWritePointer(0);
    
    const auto samplemultiplier = MathConstants<SampleType>::pi / static_cast<SampleType> (numSamples - 1);
    
    for (int i = 0; i < numSamples; ++i)
        writing[i] = static_cast<SampleType> (0.5 - 0.5 * ( std::cos (static_cast<SampleType> (2.0 * i) * samplemultiplier) ) );
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
void Harmonizer<SampleType>::reportActiveNotes (Array<int>& outputArray, const bool includePlayingButReleased) const
{
    outputArray.clearQuick();
    
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive())
        {
            if (includePlayingButReleased)
                outputArray.add (voice->getCurrentlyPlayingNote());
            else if (! voice->isPlayingButReleased())
                outputArray.add (voice->getCurrentlyPlayingNote());
        }
    }
    
    if (! outputArray.isEmpty())
        outputArray.sort();
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS FOR UPDATING PARAMETERS ----------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// stereo width ---------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateStereoWidth (const int newWidth)
{
    jassert (isPositiveAndBelow (newWidth, 101));
    
    if (panner.getCurrentStereoWidth() == newWidth)
        return;
    
    panner.updateStereoWidth(newWidth);
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        if (voice->getCurrentlyPlayingNote() < lowestPannedNote)
        {
            if (voice->getCurrentMidiPan() != 64)
            {
                panner.panValTurnedOff (voice->getCurrentMidiPan());
                voice->setPan (64);
            }
        }
        else
            voice->setPan (panner.getClosestNewPanValFromOld (voice->getCurrentMidiPan()));
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::updateLowestPannedNote (const int newPitchThresh) 
{
    if (lowestPannedNote == newPitchThresh)
        return;
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        const int note = voice->getCurrentlyPlayingNote();
    
        if (note < newPitchThresh)
        {
            if (voice->getCurrentMidiPan() != 64)
            {
                panner.panValTurnedOff (voice->getCurrentMidiPan());
                voice->setPan (64);
            }
        }
        else if (note < lowestPannedNote) // because we haven't updated the lowestPannedNote member variable yet, voices with pitches higher than newPitchThresh but lower than lowestPannedNote are the voices that now qualify for panning
            if (voice->getCurrentMidiPan() == 64)
                voice->setPan (panner.getNextPanVal());
    }
    
    lowestPannedNote = newPitchThresh;
};


// midi velocity sensitivity -------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateMidiVelocitySensitivity (const int newSensitivity)
{
    const float newSens = newSensitivity/100.0f;
    
    if (velocityConverter.getCurrentSensitivity() == newSens)
        return;
    
    velocityConverter.setFloatSensitivity (newSens);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setVelocityMultiplier (velocityConverter.floatVelocity (voice->getLastRecievedVelocity()));
};


// pitch bend settings -------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updatePitchbendSettings (const int rangeUp, const int rangeDown)
{
    if ((bendTracker.getCurrentRangeUp() == rangeUp) && (bendTracker.getCurrentRangeDown() == rangeDown))
        return;
    
    bendTracker.setRange(rangeUp, rangeDown);
    
    if (lastPitchWheelValue == 64)
        return;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};


// descant settings -----------------------------------------------------------------------------------------------------------------------------
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
};

template<typename SampleType>
void Harmonizer<SampleType>::setDescantLowerThresh (const int newThresh)
{
    if (descant.lowerThresh == newThresh)
        return;
    
    descant.lowerThresh = newThresh;
    
    if (descant.isOn)
        applyDescant();
};

template<typename SampleType>
void Harmonizer<SampleType>::setDescantInterval (const int newInterval)
{
    if (descant.interval == newInterval)
        return;
    
    descant.interval = newInterval;
    
    if (descant.isOn)
        applyDescant();
};


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
};

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchUpperThresh (const int newThresh)
{
    if (pedal.upperThresh == newThresh)
        return;
    
    pedal.upperThresh = newThresh;
    
    if (pedal.isOn)
        applyPedalPitch();
};

template<typename SampleType>
void Harmonizer<SampleType>::setPedalPitchInterval (const int newInterval)
{
    if (pedal.interval == newInterval)
        return;
    
    pedal.interval = newInterval;
    
    if (pedal.isOn)
        applyPedalPitch();
};


// ADSR settings------------------------------------------------------------------------------------------------------------------------------
template<typename SampleType>
void Harmonizer<SampleType>::updateADSRsettings (const float attack, const float decay, const float sustain, const float release)
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
void Harmonizer<SampleType>::updateQuickReleaseMs (const int newMs)
{
    jassert (newMs > 0);
    
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
};

template<typename SampleType>
void Harmonizer<SampleType>::updateQuickAttackMs(const int newMs)
{
    jassert (newMs > 0);
    
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
};

/***************************************************************************************************************************************************
// functions for management of HarmonizerVoices------------------------------------------------------------------------------------------------------
****************************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::addVoice (HarmonizerVoice<SampleType>* newVoice)
{
    voices.add (newVoice);
    
    panner.setNumberOfVoices (voices.size());
};


template<typename SampleType>
void Harmonizer<SampleType>::removeNumVoices (const int voicesToRemove)
{
    if (voicesToRemove == 0)
        return;
    
    int voicesRemoved = 0;
    
    while (voicesRemoved < voicesToRemove)
    {
        if (voices.isEmpty())
            break;
        
        HarmonizerVoice<SampleType>* removing = nullptr;
        
        for (auto* voice : voices)
        {
            if (! voice->isVoiceActive())
            {
                removing = voice;
                break;
            }
        }
        
        if (removing == nullptr)
            removing = findVoiceToSteal (0);
        
        if (removing == nullptr)
            removing = voices[0];
        
        if (removing->isVoiceActive())
            panner.panValTurnedOff (removing->getCurrentMidiPan());
        
        voices.removeObject (removing, true);
        
        ++voicesRemoved;
    }
    
    panner.setNumberOfVoices (std::max (voices.size(), 1));
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getVoicePlayingNote (const int midiPitch) const
{
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiPitch)
            return voice;
    
    return nullptr;
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentDescantVoice() const
{
    if (! descant.isOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->isCurrentDescantVoice())
            return voice;
    
    return nullptr;
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentPedalPitchVoice() const
{
    if (! pedal.isOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->isCurrentPedalVoice())
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


}; // namespace
