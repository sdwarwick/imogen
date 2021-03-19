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
    
    // standard & general-purpose functions ---------------------------------------------------------------------------------------------------------
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
    bool shouldWarnUserToEnableSidechain() const;
#endif
    
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    const juce::String getName() const override { return JucePlugin_Name; }
    
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool supportsMPE()  const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void savePreset  (juce::String presetName);
    bool loadPreset  (juce::String presetName);
    void deletePreset(juce::String presetName);
    juce::File getPresetsFolder() const { return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen"); }
    
    
    juce::AudioProcessorParameter* getBypassParameter() const override { return tree.getParameter ("mainBypass"); }
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    
    bav::MessageQueue paramChangesForEditor;
    bav::MessageQueue paramChangesForProcessor;
    
    
    enum parameterIDs
    {
        midiLatchID,  // midi latch is not an automatable parameter, but needs an ID here so that evenrs in the message FIFO can represent latch on/off
        killAllMidiID,  // not automatable
        pitchbendFromEditorID, // not automatable
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
    
    
    // returns any parameter's current value as a float in the range 0.0 to 1.0
    float getCurrentParameterValue (const parameterIDs paramID) const;
    
    
    // returns any parameter's default value as a float in the normalised range 0.0 to 1.0
    float getDefaultParameterValue (const parameterIDs paramID) const;
    
    
    bool hasUpdatedParamDefaults()
    {
        const bool hasUpdated = parameterDefaultsAreDirty.load();
        parameterDefaultsAreDirty.store (false);
        return hasUpdated;
    }
    
    
    // returns the normalisable range associated with the given parameter.
    const juce::NormalisableRange<float>& getParameterRange (const parameterIDs paramID) const;
    
    
    inline float vocalRangeTypeToFloatParam (juce::String name) const
    {
        if (name.equalsIgnoreCase ("Soprano")) return 0.0f;
        if (name.equalsIgnoreCase ("Alto")) return 0.25f;
        if (name.equalsIgnoreCase ("Tenor")) return 0.5f;
        jassert (name.equalsIgnoreCase ("Bass")); return 0.75f;
    }
    
    inline juce::String floatParamToVocalRangeType (float vocalRangeParam) const
    {
        if (vocalRangeParam == 0.0f) return juce::String ("Soprano");
        if (vocalRangeParam == 0.25f) return juce::String ("Alto");
        if (vocalRangeParam == 0.5f) return juce::String ("Tenor");
        jassert (vocalRangeParam == 0.75f); return juce::String ("Bass");
    }
    
    
private:
    
    juce::Array< bav::MessageQueue::Message >  currentMessages;  // this array stores the current messages from the queue
    
    juce::AudioProcessorValueTreeState tree;
    
    template<typename SampleType>
    void updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    void processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine);
    
    void updateVocalRangeType (juce::String newRangeType);
    
    void updateNumVoices (const int newNumVoices);
    
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
                                     const bool masterBypass);
    
    
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    juce::PluginHostType host;
    bool needsSidechain;
#endif
    
    template<typename SampleType>
    void updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                           bool compressorIsOn, float knobValue);
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    void initializeParameterListeners();
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
    template<typename SampleType>
    std::unique_ptr<juce::XmlElement> pluginStateToXml (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine);
    
    
    juce::AudioParameterBool*  mainBypass;
    juce::AudioParameterChoice* vocalRangeType;
    BoolParamPtr  leadBypass, harmonyBypass, adsrToggle, pedalPitchIsOn, descantIsOn, voiceStealing, limiterToggle, noiseGateToggle, compressorToggle, aftertouchGainToggle, deEsserToggle, reverbToggle;
    IntParamPtr   dryPan, dryWet, stereoWidth, lowestPanned, velocitySens, pitchBendRange, pedalPitchThresh, pedalPitchInterval, descantThresh, descantInterval, concertPitchHz, reverbDryWet, numVoices, inputSource;
    FloatParamPtr adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateThreshold, inputGain, outputGain, compressorAmount, deEsserThresh, deEsserAmount, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut;
    
    
    void updateParameterDefaults();
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    std::atomic<int> defaultVocalRangeIndex;
    
    
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
    
    void addParameterMessenger (juce::String stringID, int paramID);
    
    
    
#define imgn_VOCAL_RANGE_TYPES juce::StringArray { "Soprano","Alto","Tenor","Bass" }
    
    juce::StringArray vocalRangeTypes = imgn_VOCAL_RANGE_TYPES;
    
    juce::NormalisableRange<float> pitchBendNormRange { 0, 127 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
