
#include "bv_Harmonizer.h"


#undef bvh_VOID_TEMPLATE
#define bvh_VOID_TEMPLATE template<typename SampleType> void Harmonizer<SampleType>


namespace bav
{
    

bvh_VOID_TEMPLATE::changeNumVoices (const int newNumVoices)
{
    const int currentVoices = voices.size();
    
    if (currentVoices == newNumVoices)
        return;
    
    if (newNumVoices > currentVoices)
        addNumVoices (newNumVoices - currentVoices);
    else
        removeNumVoices (currentVoices - newNumVoices);
    
    numVoicesChanged();
}

// midi latch: when active, holds note offs recieved from the keyboard instead of sending them immediately; held note offs are sent once latch is deactivated.
bvh_VOID_TEMPLATE::setMidiLatch (const bool shouldBeOn, const bool allowTailOff)
{
    if (latchIsOn == shouldBeOn)
        return;
    
    latchIsOn = shouldBeOn;
    
    if (shouldBeOn)
        return;
    
    turnOffAllKeyupNotes (allowTailOff, false, !allowTailOff, false);
    
    pitchCollectionChanged();
}


// interval latch
bvh_VOID_TEMPLATE::setIntervalLatch (const bool shouldBeOn, const bool allowTailOff)
{
    if (intervalLatchIsOn == shouldBeOn)
        return;
    
    intervalLatchIsOn = shouldBeOn;
    
    if (shouldBeOn)
    {
        updateIntervalsLatchedTo();
    }
    else if (! latchIsOn)
    {
        turnOffAllKeyupNotes (allowTailOff, false, !allowTailOff, false);
        pitchCollectionChanged();
    }
}
    
    
// descant settings ----------------------------------------------------------------------------------------------------------------------------

bvh_VOID_TEMPLATE::setDescant (const bool isOn)
{
    if (descant.isOn == isOn)
        return;
    
    descant.isOn = isOn;
    
    if (isOn)
        applyDescant();
    else
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        descant.lastPitch = -1;
    }
}


bvh_VOID_TEMPLATE::setDescantLowerThresh (int newThresh)
{
    jassert (newThresh >= 0 && newThresh <= 127);
    
    if (descant.lowerThresh == newThresh)
        return;
    
    descant.lowerThresh = newThresh;
    
    if (descant.isOn)
        applyDescant();
}


bvh_VOID_TEMPLATE::setDescantInterval (const int newInterval)
{
    if (descant.interval == newInterval)
        return;
    
    descant.interval = newInterval;
    
    if (descant.isOn)
        applyDescant();
}


// pedal pitch settings -----------------------------------------------------------------------------------------------------------------------

bvh_VOID_TEMPLATE::setPedalPitch (const bool isOn)
{
    if (pedal.isOn == isOn)
        return;
    
    pedal.isOn = isOn;
    
    if (isOn)
        applyPedalPitch();
    else
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        pedal.lastPitch = -1;
    }
}


bvh_VOID_TEMPLATE::setPedalPitchUpperThresh (int newThresh)
{
    jassert (newThresh >= 0 && newThresh <= 127);
    
    if (pedal.upperThresh == newThresh)
        return;
    
    pedal.upperThresh = newThresh;
    
    if (pedal.isOn)
        applyPedalPitch();
}


bvh_VOID_TEMPLATE::setPedalPitchInterval (const int newInterval)
{
    if (pedal.interval == newInterval)
        return;
    
    pedal.interval = newInterval;
    
    if (pedal.isOn)
        applyPedalPitch();
}

    
// stereo width ---------------------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updateStereoWidth (int newWidth)
{
    jassert (newWidth >= 0 && newWidth <= 100);
    
    if (panner.getCurrentStereoWidth() == newWidth)
        return;
    
    panner.updateStereoWidth (newWidth);
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        if (voice->getCurrentlyPlayingNote() < lowestPannedNote)
        {
            const int currentPan = voice->getCurrentMidiPan();
            
            if (currentPan != 64)
            {
                panner.panValTurnedOff (currentPan);
                voice->setPan (64);
            }
        }
        else
        {
            voice->setPan (panner.getClosestNewPanValFromOld (voice->getCurrentMidiPan()));
        }
    }
}


bvh_VOID_TEMPLATE::updateLowestPannedNote (int newPitchThresh)
{
    jassert (newPitchThresh >= 0 && newPitchThresh <= 127);
    
    const int prevLowestnote = lowestPannedNote;
    
    if (prevLowestnote == newPitchThresh)
        return;
    
    lowestPannedNote = newPitchThresh;
    
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
        else if (note < prevLowestnote) // because we haven't updated the lowestPannedNote member variable yet, voices with pitches higher than newPitchThresh but lower than lowestPannedNote are the voices that now qualify for panning
        {
            if (currentPan == 64)
                voice->setPan (panner.getNextPanVal());
        }
    }
}


// concert pitch hz ------------------------------------------------------------------------------------------------

bvh_VOID_TEMPLATE::setConcertPitchHz (const int newConcertPitchhz)
{
    jassert (newConcertPitchhz > 0);
    
    if (pitchConverter.getCurrentConcertPitchHz() == newConcertPitchhz)
        return;
    
    pitchConverter.setConcertPitchHz (newConcertPitchhz);
    
    if (currentInputFreq > 0.0f)
        setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
}


// ADSR settings------------------------------------------------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updateADSRsettings (const float attack, const float decay, const float sustain, const float release)
{
    // attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
    
    adsrParams.attack  = attack;
    adsrParams.decay   = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    
    for (auto* voice : voices)
        voice->setAdsrParameters (adsrParams);
}


bvh_VOID_TEMPLATE::updateQuickReleaseMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
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


bvh_VOID_TEMPLATE::updateQuickAttackMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
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
    

/***********************************************************************************************************************************************
 // midi preferences ---------------------------------------------------------------------------------------------------------------------------
 ***********************************************************************************************************************************************/

// midi velocity sensitivity -------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updateMidiVelocitySensitivity (int newSensitivity)
{
    jassert (newSensitivity >= 0 && newSensitivity <= 100);
    
    const float newSens = newSensitivity / 100.0f;
    
    if (velocityConverter.getCurrentSensitivity() == newSens)
        return;
    
    velocityConverter.setFloatSensitivity (newSens);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setVelocityMultiplier (getWeightedVelocity (voice->getLastRecievedVelocity()));
}


// pitch bend settings -------------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updatePitchbendSettings (const int rangeUp, const int rangeDown)
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


}  // namespace

#undef bvh_VOID_TEMPLATE
