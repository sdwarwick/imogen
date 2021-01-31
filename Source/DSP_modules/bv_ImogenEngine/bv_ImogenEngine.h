/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        base class that wraps the Harmonizer & pitch detector classes into one processor
 website:            http://www.benvining.com
 license:            GPL
 dependencies:       bv_Harmonizer, bv_PitchDetector, juce_dsp
 OSXFrameworks:
 iOSFrameworks:
 linuxLibs:
 mingwLibs:
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/

#pragma once

#include "juce_dsp/juce_dsp.h"

#include "bv_Harmonizer/bv_Harmonizer.h"
#include "bv_PitchDetector/bv_PitchDetector.h"



template<typename SampleType>
class ImogenEngine
{
public:
    ImogenEngine();
    
    ~ImogenEngine();
    
    void changeBlocksize (const int newBlocksize);
    
    void process (juce::AudioBuffer<SampleType>& inBus, juce::AudioBuffer<SampleType>& output, juce::MidiBuffer& midiMessages,
                  const bool applyFadeIn = false, const bool applyFadeOut = false);
    
    void processBypassed (juce::AudioBuffer<SampleType>& inBus, juce::AudioBuffer<SampleType>& output);
    
    void initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void prepare (double sampleRate, int samplesPerBlock);
    
    void reset();
    
    void releaseResources();
    
    int reportLatency() const noexcept { return internalBlocksize; }
    
    void updateNumVoices(const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    
    void returnActivePitches(juce::Array<int>& outputArray) const { return harmonizer.reportActiveNotes(outputArray); }
    
    void updateSamplerate (const int newSamplerate);
    void updateDryWet     (const float newWetMixProportion);
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
    void updateIntervalLock(const bool isLocked);
    void updateLimiter     (const float thresh, const float release, const bool isOn);
    void updateInputGain  (const float newInGain);
    void updateOutputGain (const float newOutGain);
    void updateDryGain (const float newDryGain);
    void updateWetGain (const float newWetGain);
    void updateSoftPedalGain (const float newGain);
    void updatePitchDetectionHzRange (const int minHz, const int maxHz);
    void updatePitchDetectionConfidenceThresh (const float newThresh);
    
    void clearBuffers();
    
    bool hasBeenReleased()    const noexcept { return resourcesReleased; }
    bool hasBeenInitialized() const noexcept { return initialized; }
    
    enum ModulatorInputSource { left, right, mixToMono }; // determines how the plugin will take input from the stereo buffer fed to it from the host
    
    ModulatorInputSource getModulatorSource() const noexcept { return modulatorInput; }
    
    void setModulatorSource (const ModulatorInputSource newSource) { modulatorInput = newSource; }
    
private:
    
    ModulatorInputSource modulatorInput; // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    
    int internalBlocksize; // the size of the processing blocks, in samples, that the algorithm will be processing at a time. This corresponds to the latency of the pitch detector, and, thus, the minimum possible Hz it can detect.
    
    void processWrapped (juce::AudioBuffer<SampleType>& inBus, juce::AudioBuffer<SampleType>& output,
                         juce::MidiBuffer& midiMessages,
                         const bool applyFadeIn, const bool applyFadeOut);
    
    void processBypassedWrapped (juce::AudioBuffer<SampleType>& inBus, juce::AudioBuffer<SampleType>& output);
    
    
    void renderBlock (const juce::AudioBuffer<SampleType>& input, juce::MidiBuffer& midiMessages);
    
    PitchDetector<SampleType> pitchDetector;
    Harmonizer<SampleType> harmonizer;
    
    DelayBuffer<SampleType> inputBuffer;
    DelayBuffer<SampleType> outputBuffer;
    
    juce::AudioBuffer<SampleType> inBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    juce::AudioBuffer<SampleType> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    juce::AudioBuffer<SampleType> dryBuffer; // this buffer is used for panning & delaying the dry signal
    
    juce::dsp::ProcessSpec dspSpec;
    juce::dsp::Limiter <SampleType> limiter;
    juce::dsp::DryWetMixer<SampleType> dryWetMixer;
    bool limiterIsOn;
    
    bool resourcesReleased;
    bool initialized;
    
    float dryGain, prevDryGain;
    float wetGain, prevWetGain;
    
    float inputGain, prevInputGain;
    float outputGain, prevOutputGain;
    
    juce::MidiBuffer midiChoppingBuffer;
    
    FancyMidiBuffer midiInputCollection;
    FancyMidiBuffer midiOutputCollection;
    FancyMidiBuffer chunkMidiBuffer;
    
    
    void copyRangeOfMidiBuffer (const juce::MidiBuffer& readingBuffer, juce::MidiBuffer& destBuffer,
                                const int startSampleOfInput,
                                const int startSampleOfOutput,
                                const int numSamples);
    
    
    void pushUpLeftoverSamples (juce::AudioBuffer<SampleType>& targetBuffer,
                                const int numSamplesUsed,
                                const int numSamplesLeft);
    
    Panner dryPanner;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};
