
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
 
 bv_ImogenEngine.h: This file defines the central engine class that performs all of Imogen's audio processing. An ImogenAudioProcessor essentially acts as a wrapper around two instances of ImogenEngine (one float, one double).
 
======================================================================================================================================================*/


/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        base class that wraps the Harmonizer class into a self-sufficient audio engine
 dependencies:       bv_Harmonizer
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_Harmonizer/bv_Harmonizer.h"



namespace bav
{


template<typename SampleType>
class ImogenEngine  :   public bav::dsp::FIFOWrappedEngine<SampleType>
{
    
    using FIFOEngine  = bav::dsp::FIFOWrappedEngine<SampleType>;
    using AudioBuffer = juce::AudioBuffer<SampleType>;
    using MidiBuffer  = juce::MidiBuffer;
    using uint32 = juce::uint32;
    
    
public:
    ImogenEngine();
    
    void killAllMidi();
    
    int reportLatency() const noexcept { return FIFOEngine::getLatency(); }
    
    int getCurrentNumVoices() const { return harmonizer.getNumVoices(); }
    
    void returnActivePitches (juce::Array<int>& outputArray) const;
    
    void recieveExternalPitchbend (const int bend);
    
    void updateDryWet      (float percentWet);
    void updateDryVoxPan   (int newMidiPan);
    void updateAdsrAttack  (float attack);
    void updateAdsrDecay   (float decay);
    void updateAdsrSustain (float sustain);
    void updateAdsrRelease (float release);
    
    void updateAdsr       (const float attack, const float decay, const float sustain, const float release);
    void updateStereoWidth(const int newStereoWidth, const int lowestPannedNote);
    void updateMidiVelocitySensitivity(const int newSensitivity);
    void updatePitchbendRange (const int rangeST);
    void updatePedalPitch  (const bool isOn, const int upperThresh, const int interval);
    void updateDescant     (const bool isOn, const int lowerThresh, const int interval);
    void updateNoteStealing(const bool shouldSteal);
    void updateLimiter     (const bool isOn);
    void updateNoiseGate (const float newThreshDB, const bool isOn);
    void updateDeEsser (const float deEssAmount, const float thresh_dB, const bool isOn);
    void updateCompressor (const float threshDB, const float ratio, const bool isOn);
    void updateDelay (int dryWet, int delayInSamples, bool isOn);
    void updateReverb (int wetPcnt, float decay, float duckAmount, float loCutFreq, float hiCutFreq, bool isOn);
    void updateInputGain  (const float newInGain);
    void updateOutputGain (const float newOutGain);
    void updateAftertouchGainOnOff (const bool shouldBeOn);
    void updateLeadBypass (bool leadIsBypassed) { leadBypass.store (leadIsBypassed); }
    void updateHarmonyBypass (bool harmoniesAreBypassed) { harmonyBypass.store (harmoniesAreBypassed); }
    
    int getModulatorSource() const noexcept { return modulatorInput.load(); }
    void setModulatorSource (const int newSource);
    
    void playChord (const juce::Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld);
    
    bool isMidiLatched() const { return harmonizer.isLatched(); }
    void updateMidiLatch (const bool isLatched);
    
    
    bool isConnectedToMtsEsp() const noexcept { return harmonizer.isConnectedToMtsEsp(); }
    juce::String getScaleName() const { return harmonizer.getScaleName(); }
    
    
private:
    
    // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    // 0 - left channel only
    // 1 - right channel only
    // 2 - mix all input channels to mono
    std::atomic<int> modulatorInput;
    
    static constexpr auto minSmoothedGain = SampleType(0.0000001);
    
    template<typename Type>
    inline Type smoothingZeroCheck (Type value)
    {
        return std::max (minSmoothedGain, static_cast<SampleType>(value));
    }
    
    void renderBlock (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages) override;
    
    void bypassedBlock (const AudioBuffer& input, MidiBuffer& midiMessages) override;
    
    void initialized (int newInternalBlocksize, double samplerate) override;
    
    void prepareToPlay (double samplerate) override;
    
    void resetTriggered() override;
    
    void latencyChanged (int newInternalBlocksize) override;
    
    void release() override;
    
    Harmonizer<SampleType> harmonizer;
    
    AudioBuffer monoBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer dryBuffer; // this buffer is used for panning & delaying the dry signal
    
    juce::dsp::ProcessSpec dspSpec;
    
    bav::dsp::FX::NoiseGate<SampleType> gate;
    std::atomic<bool> noiseGateIsOn;
    
    bav::dsp::FX::DeEsser<SampleType> deEsser;
    std::atomic<bool> deEsserIsOn;
    
    juce::dsp::DryWetMixer<SampleType> dryWetMixer;
    
    juce::dsp::IIR::Filter<SampleType> initialHiddenLoCut;
    
    bav::dsp::FX::Compressor<SampleType> compressor;
    std::atomic<bool> compressorIsOn;
    
    bav::dsp::FX::Reverb reverb;
    std::atomic<bool> reverbIsOn;
    
    bav::dsp::FX::Limiter<SampleType> limiter;
    std::atomic<bool> limiterIsOn;
    std::atomic<float> limiterThresh, limiterRelease;
    
    bav::dsp::Panner dryPanner;
    
    bav::dsp::FX::Delay<SampleType> delay;
    std::atomic<bool> delayIsOn;
    
    std::atomic<bool> leadBypass, harmonyBypass;
    
    juce::SmoothedValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> inputGain, outputGain, dryLgain, dryRgain;
    
    void resetSmoothedValues (int blocksize);
    
    static constexpr auto limiterThreshDb     = 0.0f;
    static constexpr auto limiterReleaseMs    = 35.0f;
    static constexpr auto noiseGateAttackMs   = 25.0f;
    static constexpr auto noiseGateReleaseMs  = 100.0f;
    static constexpr auto noiseGateFloorRatio = 10.0f; // ratio to one when the noise gate is activated
    static constexpr auto compressorAttackMs  = 4.0f;
    static constexpr auto compressorReleaseMs = 200.0f;
    
    static constexpr auto pitchDetectorMinHz = 80;
    static constexpr auto pitchDetectorMaxHz = 2400;
    
    static constexpr auto initialHiddenHiPassFreq = SampleType(65);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};


} // namespace
