
#include "bv_ImogenEngine.h"


#undef bvie_VOID_TEMPLATE
#define bvie_VOID_TEMPLATE template<typename SampleType> void ImogenEngine<SampleType>


namespace bav
{

bvie_VOID_TEMPLATE::updateNumVoices (const int newNumVoices)
{
    if (harmonizer.getNumVoices() != newNumVoices)
        harmonizer.changeNumVoices (newNumVoices);
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

bvie_VOID_TEMPLATE::updateDryGain (const float newDryGain)
{
    prevDryGain.store(dryGain.load());
    dryGain.store(newDryGain);
}

bvie_VOID_TEMPLATE::updateWetGain (const float newWetGain)
{
    prevWetGain.store(wetGain.load());
    wetGain.store(newWetGain);
}


bvie_VOID_TEMPLATE::updateDryVoxPan  (const int newMidiPan)
{
    if (dryPanner.getLastMidiPan() != newMidiPan)
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
    harmonizer.updateQuickReleaseMs (newMs);
}

bvie_VOID_TEMPLATE::updateQuickAttack (const int newMs)
{
    harmonizer.updateQuickAttackMs (newMs);
}


bvie_VOID_TEMPLATE::updateStereoWidth (const int newStereoWidth, const int lowestPannedNote)
{
    harmonizer.updateLowestPannedNote (lowestPannedNote);
    harmonizer.updateStereoWidth (newStereoWidth);
}


bvie_VOID_TEMPLATE::updateMidiVelocitySensitivity (const int newSensitivity)
{
    harmonizer.updateMidiVelocitySensitivity (newSensitivity);
}


bvie_VOID_TEMPLATE::updatePitchbendSettings (const int rangeUp, const int rangeDown)
{
    harmonizer.updatePitchbendSettings (rangeUp, rangeDown);
}


bvie_VOID_TEMPLATE::updatePedalPitch (const bool isOn, const int upperThresh, const int interval)
{
    harmonizer.setPedalPitch (isOn);
    harmonizer.setPedalPitchUpperThresh (upperThresh);
    harmonizer.setPedalPitchInterval (interval);
}


bvie_VOID_TEMPLATE::updateDescant (const bool isOn, const int lowerThresh, const int interval)
{
    harmonizer.setDescant (isOn);
    harmonizer.setDescantLowerThresh (lowerThresh);
    harmonizer.setDescantInterval (interval);
}


bvie_VOID_TEMPLATE::updateConcertPitch (const int newConcertPitchHz)
{
    harmonizer.setConcertPitchHz (newConcertPitchHz);
}


bvie_VOID_TEMPLATE::updateNoteStealing (const bool shouldSteal)
{
    harmonizer.setNoteStealingEnabled (shouldSteal);
}


bvie_VOID_TEMPLATE::updateMidiLatch (const bool isLatched)
{
    harmonizer.setMidiLatch (isLatched, true);
}


bvie_VOID_TEMPLATE::updateIntervalLock (const bool isLocked)
{
    harmonizer.setIntervalLatch (isLocked, true);
}


bvie_VOID_TEMPLATE::updateLimiter (const float thresh, const int release, const bool isOn)
{
    limiterIsOn.store(isOn);
    limiterThresh.store(thresh);
    limiterRelease.store(float(release));
}


bvie_VOID_TEMPLATE::updateSoftPedalGain (const float newGain)
{
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
    harmonizer.updatePitchDetectionConfidenceThresh (newUpperThresh, newLowerThresh);
}


bvie_VOID_TEMPLATE::updateAftertouchGainOnOff (const bool shouldBeOn)
{
    harmonizer.setAftertouchGainOnOff (shouldBeOn);
}


bvie_VOID_TEMPLATE::updateUsingChannelPressure (const bool useChannelPressure)
{
    harmonizer.shouldUseChannelPressure (useChannelPressure);
}


bvie_VOID_TEMPLATE::updatePlayingButReleasedGain (const float newGainMult)
{
    harmonizer.setPlayingButReleasedGain (newGainMult);
}


#undef bvie_VOID_TEMPLATE


}  // namespace
