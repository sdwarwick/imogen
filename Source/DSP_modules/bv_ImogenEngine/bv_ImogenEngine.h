/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        base class that wraps the Harmonizer class into a self-sufficient audio processor
 dependencies:       bv_Harmonizer, juce_dsp
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "juce_dsp/juce_dsp.h"

#include "bv_Harmonizer/bv_Harmonizer.h"  // this file includes the bv_SharedCode header



namespace bav

{
    
using namespace juce;


template<typename SampleType>
class ImogenEngine  :   public bav::dsp::FIFOWrappedEngineWithMidi<SampleType>
{
    
    using FIFOEngine = bav::dsp::FIFOWrappedEngineWithMidi<SampleType>;
    
public:
    ImogenEngine();
    
    
    void initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void reset();
    
    void killAllMidi();
    
    int reportLatency() const noexcept { return FIFOEngine::getLatency(); }
    
    void updateNumVoices (const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    int getCurrentNumVoices() const { return harmonizer.getNumVoices(); }
    
    void returnActivePitches (Array<int>& outputArray) const;
    
    void recieveExternalPitchbend (const int bend);
    
    void updateDryWet     (const int percentWet);
    void updateDryVoxPan  (const int newMidiPan);
    void updateAdsr       (const float attack, const float decay, const float sustain, const float release, const bool isOn);
    void updateQuickKill  (const int newMs);
    void updateQuickAttack(const int newMs);
    void updateStereoWidth(const int newStereoWidth, const int lowestPannedNote);
    void updateMidiVelocitySensitivity(const int newSensitivity);
    void updatePitchbendSettings(const int rangeUp, const int rangeDown);
    void updatePedalPitch  (const bool isOn, const int upperThresh, const int interval);
    void updateDescant     (const bool isOn, const int lowerThresh, const int interval);
    void updateConcertPitch(const int newConcertPitchHz);
    void updateNoteStealing(const bool shouldSteal);
    void updateMidiLatch   (const bool isLatched);
    void updateLimiter     (const bool isOn);
    void updateInputGain  (const float newInGain);
    void updateOutputGain (const float newOutGain);
    void updateSoftPedalGain (const float newGain);
    void updateAftertouchGainOnOff (const bool shouldBeOn);
    void updateUsingChannelPressure (const bool useChannelPressure);
    void updatePlayingButReleasedGain (const float newGainMult);
    void updatePitchDetectionHzRange (const int minHz, const int maxHz);
    void updatePitchDetectionConfidenceThresh (const float newUpperThresh, const float newLowerThresh);
    
    bool hasBeenReleased()    const noexcept { return resourcesReleased; }
    bool hasBeenInitialized() const noexcept { return initialized; }
    
    int getModulatorSource() const noexcept { return modulatorInput.load(); }
    void setModulatorSource (const int newSource);
    
    void playChord (const Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld);
    
    
private:
    
    // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    // 0 - left channel only
    // 1 - right channel only
    // 2 - mix all input channels to mono
    std::atomic<int> modulatorInput;
    
    void renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages) override;
    
    void prepareToPlay (double samplerate, int blocksize) override;
    
    void latencyChanged (int newInternalBlocksize) override;
    
    void release() override;
    
    Harmonizer<SampleType> harmonizer;
    
    AudioBuffer<SampleType> monoBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer<SampleType> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer<SampleType> dryBuffer; // this buffer is used for panning & delaying the dry signal
    
    juce::dsp::ProcessSpec dspSpec;
    
    juce::dsp::DryWetMixer<SampleType> dryWetMixer;
    std::atomic<SampleType> wetMixPercent;
    
    juce::dsp::Limiter <SampleType> limiter;
    std::atomic<bool> limiterIsOn;
    std::atomic<float> limiterThresh, limiterRelease;
    
    bool resourcesReleased;
    bool initialized;
    
    std::atomic<float> inputGain, outputGain;
    std::atomic<float> prevInputGain, prevOutputGain;
    
    bav::dsp::Panner dryPanner;
    
    CriticalSection lock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};


} // namespace
