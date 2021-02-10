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

#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    
using namespace juce;


template<typename SampleType>
class ImogenEngine
{
    
public:
    ImogenEngine();
    
    ~ImogenEngine();
    
    void changeBlocksize (const int newBlocksize);
    
    void process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages,
                  const bool applyFadeIn = false, const bool applyFadeOut = false);
    
    void processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output);
    
    void initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void prepare (double sampleRate, int samplesPerBlock);
    
    void reset();
    
    void killAllMidi();
    
    void releaseResources();
    
    int reportLatency() const noexcept { return internalBlocksize; }
    
    void updateNumVoices (const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    
    void returnActivePitches (Array<int>& outputArray) const { return harmonizer.reportActiveNotes(outputArray); }
    
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
    void updatePitchDetectionConfidenceThresh (const float newUpperThresh, const float newLowerThresh);
    
    void clearBuffers();
    
    bool hasBeenReleased()    const noexcept { return resourcesReleased; }
    bool hasBeenInitialized() const noexcept { return initialized; }
    
    // determines how the plugin will select its mono input from the stereo buffer fed to it from the host
    enum ModulatorInputSource { left, right, mixToMono };
    
    ModulatorInputSource getModulatorSource() const noexcept { return modulatorInput; }
    
    void setModulatorSource (const ModulatorInputSource newSource) { modulatorInput = newSource; }
    
private:
    
    ModulatorInputSource modulatorInput; // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    
    int internalBlocksize; // the size of the processing blocks, in samples, that the algorithm will be processing at a time. This corresponds to the latency of the pitch detector, and, thus, the minimum possible Hz it can detect.
    
    void processWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                         MidiBuffer& midiMessages,
                         const bool applyFadeIn, const bool applyFadeOut);
    
    void processBypassedWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output);
    
    
    void renderBlock (const AudioBuffer<SampleType>& input, MidiBuffer& midiMessages);
    
    Harmonizer<SampleType> harmonizer;
    
    DelayBuffer<SampleType> inputBuffer;
    DelayBuffer<SampleType> outputBuffer;
    
    AudioBuffer<SampleType> inBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer<SampleType> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer<SampleType> dryBuffer; // this buffer is used for panning & delaying the dry signal
    
    dsp::ProcessSpec dspSpec;
    dsp::Limiter <SampleType> limiter;
    dsp::DryWetMixer<SampleType> dryWetMixer;
    bool limiterIsOn;
    
    bool resourcesReleased;
    bool initialized;
    
    std::atomic<float> dryGain, prevDryGain, wetGain, prevWetGain, inputGain, prevInputGain, outputGain, prevOutputGain;
    
    MidiBuffer midiChoppingBuffer;
    
    FancyMidiBuffer midiInputCollection;
    FancyMidiBuffer midiOutputCollection;
    FancyMidiBuffer chunkMidiBuffer;
    
    
    void copyRangeOfMidiBuffer (const MidiBuffer& readingBuffer, MidiBuffer& destBuffer,
                                const int startSampleOfInput,
                                const int startSampleOfOutput,
                                const int numSamples);
    
    
    void pushUpLeftoverSamples (AudioBuffer<SampleType>& targetBuffer,
                                const int numSamplesUsed,
                                const int numSamplesLeft);
    
    Panner dryPanner;
    
    CriticalSection lock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};


}; // namespace
