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
    
    juce::AudioProcessorValueTreeState tree;
    
    juce::AudioProcessorParameter* getBypassParameter() const override { return tree.getParameter ("mainBypass"); }
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    
    bav::MessageQueue paramChangesForEditor;
    bav::MessageQueue paramChangesForProcessor;
    
    
    enum parameterIDs
    {
        midiLatchID,  // midi latch is not an automatable parameter, but needs an ID here so that evenrs in the message FIFO can represent latch on/off
        modulatorSourceID,  // this is also not automatable
        killAllMidiID,  // not automatable
        pitchbendFromEditorID, // not automatable
        numVoicesID,  // definitely not automatable!!
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
    
    
    
    inline float modulatorSourceToFloatParam (int newModSource) const
    {
        switch (newModSource)
        {
            case (2):  return 0.6f;
            case (3):  return 0.9f;
            default:   return 0.3f;
        }
    }
    
    inline int floatParamToModulatorSource (float modSourceFloatParam) const
    {
        if (modSourceFloatParam == 0.3f)
            return 1;
        
        if (modSourceFloatParam == 0.6f)
            return 2;
        
        return 3;
    }
    
    inline float numVoicesToFloatParam (int newNumVoices) const
    {
        jassert (newNumVoices > 0 && newNumVoices <= bvie_MAX_POSSIBLE_NUM_VOICES);
        return float(newNumVoices / bvie_MAX_POSSIBLE_NUM_VOICES);
    }
    
    inline int floatParamToNumVoices (float numVoicesFloatParam) const
    {
        return juce::roundToInt (juce::jmap (numVoicesFloatParam, 1.0f, float(bvie_MAX_POSSIBLE_NUM_VOICES)));
    }
    
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
    
    
    // listener variables linked to AudioProcessorValueTreeState parameters:
    juce::AudioParameterBool*  mainBypass;
    juce::AudioParameterBool*  leadBypass;
    juce::AudioParameterBool*  harmonyBypass;
    juce::AudioParameterInt*   dryPan;
    juce::AudioParameterInt*   dryWet;
    juce::AudioParameterFloat* adsrAttack;
    juce::AudioParameterFloat* adsrDecay;
    juce::AudioParameterFloat* adsrSustain;
    juce::AudioParameterFloat* adsrRelease;
    juce::AudioParameterBool*  adsrToggle;
    juce::AudioParameterInt*   stereoWidth;
    juce::AudioParameterInt*   lowestPanned;
    juce::AudioParameterInt*   velocitySens;
    juce::AudioParameterInt*   pitchBendRange;
    juce::AudioParameterBool*  pedalPitchIsOn;
    juce::AudioParameterInt*   pedalPitchThresh;
    juce::AudioParameterInt*   pedalPitchInterval;
    juce::AudioParameterBool*  descantIsOn;
    juce::AudioParameterInt*   descantThresh;
    juce::AudioParameterInt*   descantInterval;
    juce::AudioParameterInt*   concertPitchHz;
    juce::AudioParameterBool*  voiceStealing;
    juce::AudioParameterFloat* inputGain;
    juce::AudioParameterFloat* outputGain;
    juce::AudioParameterBool*  limiterToggle;
    juce::AudioParameterBool*  noiseGateToggle;
    juce::AudioParameterFloat* noiseGateThreshold;
    juce::AudioParameterBool*  compressorToggle;
    juce::AudioParameterFloat* compressorAmount;
    juce::AudioParameterChoice* vocalRangeType;
    juce::AudioParameterBool*  aftertouchGainToggle;
    juce::AudioParameterBool* deEsserToggle;
    juce::AudioParameterFloat* deEsserThresh;
    juce::AudioParameterFloat* deEsserAmount;
    juce::AudioParameterBool*  reverbToggle;
    juce::AudioParameterInt*   reverbDryWet;
    juce::AudioParameterFloat* reverbDecay;
    juce::AudioParameterFloat* reverbDuck;
    juce::AudioParameterFloat* reverbLoCut;
    juce::AudioParameterFloat* reverbHiCut;
    
    void updateParameterDefaults();
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    std::atomic<int> defaultDryPan, defaultDryWet, defaultStereoWidth, defaultLowestPannedNote, defaultVelocitySensitivity, defaultPitchbendRange, defaultPedalPitchThresh, defaultPedalPitchInterval, defaultDescantThresh, defaultDescantInterval, defaultConcertPitchHz, defaultReverbDryWet, defaultNumVoices, defaultModulatorSource;
    std::atomic<float> defaultAdsrAttack, defaultAdsrDecay, defaultAdsrSustain, defaultAdsrRelease, defaultInputGain, defaultOutputGain, defaultNoiseGateThresh, defaultCompressorAmount, defaultDeEsserThresh, defaultDeEsserAmount, defaultReverbDecay, defaultReverbDuck, defaultReverbLoCut, defaultReverbHiCut;
    std::atomic<bool> defaultLeadBypass, defaultHarmonyBypass, defaultAdsrToggle, defaultPedalPitchToggle, defaultDescantToggle, defaultVoiceStealingToggle, defaultLimiterToggle, defaultNoiseGateToggle, defaultCompressorToggle, defaultAftertouchGainToggle, defaultDeEsserToggle, defaultReverbToggle;
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
