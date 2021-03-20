/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
*/

#pragma once

#include "bv_ImogenEngine/bv_ImogenEngine.h"


#ifndef IMOGEN_ONLY_BUILDING_STANDALONE
  #define IMOGEN_ONLY_BUILDING_STANDALONE 0
#endif


#define bvie_MAX_POSSIBLE_NUM_VOICES 20


class ImogenAudioProcessorEditor; // forward declaration...

///////////

class ImogenAudioProcessor    : public juce::AudioProcessor
{
    
    using FloatParameter = bav::FloatParameter;
    using IntParameter   = bav::IntParameter;
    using BoolParameter  = bav::BoolParameter;
    
    using FloatParamPtr = FloatParameter*;
    using IntParamPtr   = IntParameter*;
    using BoolParamPtr  = BoolParameter*;
    
    
public:
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    void releaseResources() override;
    
    void reset() override;
    
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;
    
    bool canAddBus (bool isInput) const override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    bool shouldWarnUserToEnableSidechain() const;  // warns the user to enable sidechain, if it's disabled (only needed for Logic & Garageband)
#endif
    
    // key values by which parameters are accessed from the editor:
    enum parameterID
    {
        numVoicesID,
        inputSourceID,
        mainBypassID,
        leadBypassID,
        harmonyBypassID,
        dryPanID,
        dryWetID,
        adsrAttackID,
        adsrDecayID,
        adsrSustainID,
        adsrReleaseID,
        adsrToggleID,
        stereoWidthID,
        lowestPannedID,
        velocitySensID,
        pitchBendRangeID,
        pedalPitchIsOnID,
        pedalPitchThreshID,
        pedalPitchIntervalID,
        descantIsOnID,
        descantThreshID,
        descantIntervalID,
        concertPitchHzID,
        voiceStealingID,
        inputGainID,
        outputGainID,
        limiterToggleID,
        noiseGateToggleID,
        noiseGateThresholdID,
        compressorToggleID,
        compressorAmountID,
        vocalRangeTypeID,
        aftertouchGainToggleID,
        deEsserToggleID,
        deEsserThreshID,
        deEsserAmountID,
        reverbToggleID,
        reverbDryWetID,
        reverbDecayID,
        reverbDuckID,
        reverbLoCutID,
        reverbHiCutID
    };
#define IMGN_FIRST_PARAM numVoicesID
#define IMGN_LAST_PARAM reverbHiCutID
    
    enum eventID  // IDs for events that are not parameters
    {
        killAllMidi,
        midiLatch,
        pitchBendFromEditor
    };
    
    // returns any parameter's current value as a float in the range 0.0 to 1.0
    float getCurrentParameterValue (const parameterID paramID) const;
    
    // returns any parameter's default value as a float in the normalised range 0.0 to 1.0
    float getDefaultParameterValue (const parameterID paramID) const;
    
    // tracks whether the processor has updated its parameter defaults since the last time this function was called
    bool hasUpdatedParamDefaults();
    
    // returns the normalisable range associated with the given parameter.
    const juce::NormalisableRange<float>& getParameterRange (const parameterID paramID) const;
    
    // returns a string descripttion of the currently selected vocal range type
    juce::String getCurrentVocalRange() const;
    
    double getTailLengthSeconds() const override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void savePreset  (juce::String presetName);
    bool loadPreset  (juce::String presetName);
    void deletePreset(juce::String presetName);
    juce::File getPresetsFolder() const { return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen"); }
    
    juce::AudioProcessorParameter* getBypassParameter() const override { return tree.getParameter ("mainBypass"); }
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool supportsMPE()  const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    const juce::String getName() const override { return JucePlugin_Name; }
    
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    // these queues are public because the editor needs to read from paramChangesForEditor and write to paramChangesForProcessor
    bav::MessageQueue paramChangesForEditor;
    bav::MessageQueue paramChangesForProcessor;
    
    // this queue is SPSC; this is only for events flowing from the editor into the processor
    bav::MessageQueue nonParamEvents;
    
    
private:
    template <typename SampleType>
    void initialize (bav::ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate,
                               bav::ImogenEngine<SampleType1>& activeEngine,
                               bav::ImogenEngine<SampleType2>& idleEngine);
    
    
    template <typename SampleType>
    inline void processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                     juce::MidiBuffer& midiMessages,
                                     bav::ImogenEngine<SampleType>& engine,
                                     const bool isBypassedThisCallback);
    
    template<typename SampleType>
    void updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    void processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    void processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine);
    
    void updateVocalRangeType (int newRangeType);
    
    void updateNumVoices (const int newNumVoices);
    
    template<typename SampleType>
    void updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                           bool compressorIsOn, float knobValue);
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    void initializeParameterListeners();
    void addParameterMessenger (juce::String stringID, int paramID);
    void updateParameterDefaults();
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine);
    
    
    inline bool isMidiLatched() const
    {
        if (isUsingDoublePrecision())
            return doubleEngine.isMidiLatched();
        
        return floatEngine.isMidiLatched();
    }
    
    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    juce::PluginHostType host;
    bool needsSidechain;
#endif
    
    juce::Array< bav::MessageQueue::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
    juce::AudioProcessorValueTreeState tree;
    
    // pointers to all the parameter objects
    BoolParamPtr  mainBypass, leadBypass, harmonyBypass, adsrToggle, pedalPitchIsOn, descantIsOn, voiceStealing, limiterToggle, noiseGateToggle, compressorToggle, aftertouchGainToggle, deEsserToggle, reverbToggle;
    IntParamPtr   vocalRangeType, dryPan, dryWet, stereoWidth, lowestPanned, velocitySens, pitchBendRange, pedalPitchThresh, pedalPitchInterval, descantThresh, descantInterval, concertPitchHz, reverbDryWet, numVoices, inputSource;
    FloatParamPtr adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateThreshold, inputGain, outputGain, compressorAmount, deEsserThresh, deEsserAmount, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut;
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    std::atomic<int> defaultVocalRangeIndex;
    
    juce::NormalisableRange<float> pitchBendNormRange { 0, 127 };  // because there's a parameterID for this, I need a dummy NormalisableRange for it
    
    
    /* attachment class that listens for changes in one specific parameter and pushes appropriate messages for each value change to both message FIFOs */
    class ParameterMessenger :  public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        ParameterMessenger(bav::MessageQueue& queue1, bav::MessageQueue& queue2, int paramIDtoListen):
            q1(queue1), q2(queue2), paramID(paramIDtoListen)
        { }
        
        void parameterChanged (const juce::String& s, float value) override
        {
            juce::ignoreUnused (s);
            q1.pushMessage (paramID, value);
            q2.pushMessage (paramID, value);
        }
        
    private:
        bav::MessageQueue& q1;
        bav::MessageQueue& q2;
        const int paramID;
    };
    
    std::vector<ParameterMessenger> parameterMessengers; // all messengers are stored in here
    
    
#define imgn_VOCAL_RANGE_TYPES juce::StringArray { "Soprano","Alto","Tenor","Bass" }
    
    juce::StringArray vocalRangeTypes = imgn_VOCAL_RANGE_TYPES;
    
#if IMOGEN_ONLY_BUILDING_STANDALONE
    const bool denormalsWereDisabledWhenTheAppStarted;  // simpe hacky way to attempt to leave the CPU as we found it in standalone app mode
#endif
    
    bav::Parameter* getParameterPntr (const parameterID paramID) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
