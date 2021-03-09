
#include "bv_ImogenEngine.h"


#undef bvie_VOID_TEMPLATE
#define bvie_VOID_TEMPLATE template<typename SampleType> void ImogenEngine<SampleType>


namespace bav
{

bvie_VOID_TEMPLATE::updateNumVoices (const int newNumVoices)
{
    if (harmonizer.getNumVoices() == newNumVoices)
        return;
        
    const ScopedLock sl (lock);
    
    harmonizer.changeNumVoices (newNumVoices);
}
    

bvie_VOID_TEMPLATE::setModulatorSource (const int newSource)
{
    jassert (newSource == 0 || newSource == 1 || newSource == 2);
    modulatorInput.store (newSource);
}


bvie_VOID_TEMPLATE::updateInputGain (const float newInGain)
{
    prevInputGain.store(inputGain.load());
    inputGain.store(newInGain);
}

bvie_VOID_TEMPLATE::updateOutputGain (const float newOutGain)
{
    prevOutputGain.store(outputGain.load());
    outputGain.store(newOutGain);
}


bvie_VOID_TEMPLATE::updateDryVoxPan  (const int newMidiPan)
{
    if (dryPanner.getLastMidiPan() == newMidiPan)
        return;
        
    const ScopedLock sl (lock);
    dryPanner.setMidiPan (newMidiPan);
}


bvie_VOID_TEMPLATE::updateDryWet (const int percentWet)
{
    constexpr SampleType pointOne = SampleType(0.01);
    wetMixPercent.store (percentWet * pointOne);
}


bvie_VOID_TEMPLATE::updateAdsr (const float attack, const float decay, const float sustain, const float release, const bool isOn)
{
    const ScopedLock sl (lock);
    harmonizer.updateADSRsettings (attack, decay, sustain, release);
    harmonizer.setADSRonOff (isOn);
}

bvie_VOID_TEMPLATE::updateQuickKill (const int newMs)
{
    const ScopedLock sl (lock);
    harmonizer.updateQuickReleaseMs (newMs);
}

bvie_VOID_TEMPLATE::updateQuickAttack (const int newMs)
{
    const ScopedLock sl (lock);
    harmonizer.updateQuickAttackMs (newMs);
}


bvie_VOID_TEMPLATE::updateStereoWidth (const int newStereoWidth, const int lowestPannedNote)
{
    const ScopedLock sl (lock);
    harmonizer.updateLowestPannedNote (lowestPannedNote);
    harmonizer.updateStereoWidth (newStereoWidth);
}


bvie_VOID_TEMPLATE::updateMidiVelocitySensitivity (const int newSensitivity)
{
    const ScopedLock sl (lock);
    harmonizer.updateMidiVelocitySensitivity (newSensitivity);
}


bvie_VOID_TEMPLATE::updatePitchbendSettings (const int rangeUp, const int rangeDown)
{
    const ScopedLock sl (lock);
    harmonizer.updatePitchbendSettings (rangeUp, rangeDown);
}


bvie_VOID_TEMPLATE::updatePedalPitch (const bool isOn, const int upperThresh, const int interval)
{
    const ScopedLock sl (lock);
    harmonizer.setPedalPitch (isOn);
    harmonizer.setPedalPitchUpperThresh (upperThresh);
    harmonizer.setPedalPitchInterval (interval);
}


bvie_VOID_TEMPLATE::updateDescant (const bool isOn, const int lowerThresh, const int interval)
{
    const ScopedLock sl (lock);
    harmonizer.setDescant (isOn);
    harmonizer.setDescantLowerThresh (lowerThresh);
    harmonizer.setDescantInterval (interval);
}


bvie_VOID_TEMPLATE::updateConcertPitch (const int newConcertPitchHz)
{
    const ScopedLock sl (lock);
    harmonizer.setConcertPitchHz (newConcertPitchHz);
}


bvie_VOID_TEMPLATE::updateNoteStealing (const bool shouldSteal)
{
    const ScopedLock sl (lock);
    harmonizer.setNoteStealingEnabled (shouldSteal);
}


bvie_VOID_TEMPLATE::updateMidiLatch (const bool isLatched)
{
    const ScopedLock sl (lock);
    harmonizer.setMidiLatch (isLatched, true);
}


bvie_VOID_TEMPLATE::updateIntervalLock (const bool isLocked)
{
    const ScopedLock sl (lock);
    harmonizer.setIntervalLatch (isLocked, true);
}


bvie_VOID_TEMPLATE::updateLimiter (const bool isOn)
{
    limiterIsOn.store(isOn);
}


bvie_VOID_TEMPLATE::updateSoftPedalGain (const float newGain)
{
    const ScopedLock sl (lock);
    harmonizer.setSoftPedalGainMultiplier (newGain);
}


bvie_VOID_TEMPLATE::updatePitchDetectionHzRange (const int minHz, const int maxHz)
{
    const ScopedLock sl (lock);
    
    jassert (harmonizer.getSamplerate() > 0);
    
    harmonizer.updatePitchDetectionHzRange (minHz, maxHz);
    
    FIFOEngine::changeLatency (harmonizer.getLatencySamples());
}


bvie_VOID_TEMPLATE::updatePitchDetectionConfidenceThresh (const float newUpperThresh, const float newLowerThresh)
{
    const ScopedLock sl (lock);
    harmonizer.updatePitchDetectionConfidenceThresh (newUpperThresh, newLowerThresh);
}


bvie_VOID_TEMPLATE::updateAftertouchGainOnOff (const bool shouldBeOn)
{
    const ScopedLock sl (lock);
    harmonizer.setAftertouchGainOnOff (shouldBeOn);
}


bvie_VOID_TEMPLATE::updateUsingChannelPressure (const bool useChannelPressure)
{
    const ScopedLock sl (lock);
    harmonizer.shouldUseChannelPressure (useChannelPressure);
}


bvie_VOID_TEMPLATE::updatePlayingButReleasedGain (const float newGainMult)
{
    const ScopedLock sl (lock);
    harmonizer.setPlayingButReleasedGain (newGainMult);
}


#undef bvie_VOID_TEMPLATE


}  // namespace
