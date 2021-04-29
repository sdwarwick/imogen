
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 bv_ImogenEngineParameters.cpp: This file includes functions dealing with updating the ImogenEngine's parameters.
 
======================================================================================================================================================*/


#include "bv_ImogenEngine.h"



#define bvie_VOID_TEMPLATE template<typename SampleType> void ImogenEngine<SampleType>


namespace bav
{
    
    
bvie_VOID_TEMPLATE::setModulatorSource (const int newSource)
{
    jassert (newSource == 1 || newSource == 2 || newSource == 3);
    modulatorInput.store (newSource);
}


bvie_VOID_TEMPLATE::updateInputGain (const float newInGain)
{
    inputGain.setTargetValue (smoothingZeroCheck (newInGain));
}

bvie_VOID_TEMPLATE::updateOutputGain (const float newOutGain)
{
    outputGain.setTargetValue (smoothingZeroCheck (newOutGain));
}


bvie_VOID_TEMPLATE::updateDryVoxPan  (int newMidiPan)
{
    jassert (newMidiPan >= 0 && newMidiPan <= 127);
    dryPanner.setMidiPan (newMidiPan);
    dryLgain.setTargetValue (smoothingZeroCheck (dryPanner.getLeftGain()));
    dryRgain.setTargetValue (smoothingZeroCheck (dryPanner.getRightGain()));
}


bvie_VOID_TEMPLATE::updateDryWet (float percentWet)
{
    jassert (percentWet >= 0.0f && percentWet <= 1.0f);
    dryWetMixer.setWetMixProportion (percentWet);
}


bvie_VOID_TEMPLATE::updateAdsrAttack (float attack)
{
    
}

bvie_VOID_TEMPLATE::updateAdsrDecay  (float decay)
{
    
}

bvie_VOID_TEMPLATE::updateAdsrSustain (float sustain)
{
    
}

bvie_VOID_TEMPLATE::updateAdsrRelease (float release)
{
    
}

bvie_VOID_TEMPLATE::updateStereoWidth (int width)
{
    jassert (width >= 0 && width <= 100);
    harmonizer.updateStereoWidth (width);
    reverb.setWidth (static_cast<float>(width) * 0.01f);
}

bvie_VOID_TEMPLATE::updateLowestPannedNote (int note)
{
    jassert (note >= 0 && note <= 127);
    harmonizer.updateLowestPannedNote (note);
}


bvie_VOID_TEMPLATE::updateMidiVelocitySensitivity (const int newSensitivity)
{
    harmonizer.updateMidiVelocitySensitivity (newSensitivity);
}


bvie_VOID_TEMPLATE::updatePitchbendRange (const int rangeST)
{
    harmonizer.updatePitchbendSettings (rangeST, rangeST);
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

bvie_VOID_TEMPLATE::updateDeEsser (const float deEssAmount, const float thresh_dB, const bool isOn)
{
    deEsser.setThresh (thresh_dB);
    deEsser.setDeEssAmount (deEssAmount);
    deEsserIsOn.store (isOn);
}


bvie_VOID_TEMPLATE::updateAftertouchGainOnOff (const bool shouldBeOn)
{
    harmonizer.setAftertouchGainOnOff (shouldBeOn);
}
    

bvie_VOID_TEMPLATE::updateReverb (int wetPcnt, float decay, float duckAmount, float loCutFreq, float hiCutFreq, bool isOn)
{
    reverbIsOn.store (isOn);
    reverb.setDryWet (wetPcnt);
    reverb.setDamping (1.0f - decay);
    reverb.setRoomSize (decay);
    reverb.setDuckAmount (duckAmount);
    //reverb.setLoCutFrequency (loCutFreq);
    //reverb.setHiCutFrequency (hiCutFreq);
    juce::ignoreUnused (loCutFreq, hiCutFreq);
}


bvie_VOID_TEMPLATE::updateDelay (int dryWet, int delayInSamples, bool isOn)
{
    delayIsOn.store (isOn);
    delay.setDelay (delayInSamples);
    jassert (dryWet >= 0 && dryWet <= 100);
    delay.setDryWet (dryWet);
}

#undef bvie_VOID_TEMPLATE


}  // namespace
