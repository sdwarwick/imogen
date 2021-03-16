
#include "bv_ImogenEngine.h"


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
    
    
bvie_VOID_TEMPLATE::updateBypassStates (bool leadIsBypassed, bool harmoniesAreBypassed)
{
    leadBypass.store (leadIsBypassed);
    harmonyBypass.store (harmoniesAreBypassed);
}
    

bvie_VOID_TEMPLATE::setModulatorSource (const int newSource)
{
    jassert (newSource == 1 || newSource == 2 || newSource == 3);
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
    dryPanner.setMidiPan (newMidiPan);
}


bvie_VOID_TEMPLATE::updateDryWet (const int percentWet)
{
    constexpr SampleType pointOne = SampleType(0.01);
    dryWetMixer.setWetMixProportion (percentWet * pointOne);
}


bvie_VOID_TEMPLATE::updateAdsr (const float attack, const float decay, const float sustain, const float release, const bool isOn)
{
    harmonizer.updateADSRsettings (attack, decay, sustain, release);
    harmonizer.setADSRonOff (isOn);
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


bvie_VOID_TEMPLATE::updatePitchbendRange (const int rangeST)
{
    harmonizer.updatePitchbendSettings (rangeST, rangeST);
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


bvie_VOID_TEMPLATE::updateLimiter (const bool isOn)
{
    limiterIsOn.store (isOn);
}
    

bvie_VOID_TEMPLATE::updateCompressor (const float threshDB, const float ratio, const bool isOn)
{
    compressor.setThreshold (SampleType(threshDB));
    compressor.setRatio (SampleType(ratio));
    compressorIsOn.store (isOn);
}
    

bvie_VOID_TEMPLATE::updateDeEsser (const float deEssAmount, const float thresh_dB, const bool isOn)
{
    deEsser.setThresh (thresh_dB);
    deEsser.setDeEssAmount (deEssAmount);
    deEsserIsOn.store (isOn);
}


bvie_VOID_TEMPLATE::updatePitchDetectionHzRange (const int minHz, const int maxHz)
{
    const ScopedLock sl (lock);
    
    jassert (harmonizer.getSamplerate() > 0);
    
    harmonizer.updatePitchDetectionHzRange (minHz, maxHz);
    
    FIFOEngine::changeLatency (harmonizer.getLatencySamples());
}


bvie_VOID_TEMPLATE::updateAftertouchGainOnOff (const bool shouldBeOn)
{
    harmonizer.setAftertouchGainOnOff (shouldBeOn);
}

    
bvie_VOID_TEMPLATE::updateNoiseGate (const float newThreshDB, const bool isOn)
{
    gate.setThreshold (SampleType(newThreshDB));
    noiseGateIsOn.store (isOn);
}


#undef bvie_VOID_TEMPLATE


}  // namespace
