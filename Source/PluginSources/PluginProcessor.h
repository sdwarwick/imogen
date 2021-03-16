/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
*/

#pragma once

#include "bv_ImogenEngine/bv_ImogenEngine.h"


#ifndef IMOGEN_ONLY_BUILDING_STANDALONE
  #define IMOGEN_ONLY_BUILDING_STANDALONE 0
#endif



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
    juce::File getPresetsFolder() const;
    
    void updateModulatorInputSource (const int newSource);
    int getCurrentModulatorInputSource() const;
    
    void returnActivePitches(juce::Array<int>& outputArray) const;
    
    void killAllMidi();
    
    void pitchbendFromEditor (const int pitchbend);
    
    juce::AudioProcessorValueTreeState tree;
    
    juce::AudioProcessorParameter* getBypassParameter() const override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    void updateNumVoices (const int newNumVoices);
    
    void setMidiLatch (const bool isOn);
    
    // these functions return parameters' current state/value
    int getDryPan() const { return dryPan->get(); }
    int getDryWet() const { return dryWet->get(); }
    float getInputGain() const { return inputGain->get(); }
    float getOutputGain() const { return outputGain->get(); }
    bool getIsLimiterOn() const { return limiterToggle->get(); }
    bool getIsNoiseGateOn() const { return noiseGateToggle->get(); }
    float getNoiseGateThresh() const { return noiseGateThreshold->get(); }
    bool getIsCompressorOn() const { return compressorToggle->get(); }
    float getCompressorAmount() const { return compressorAmount->get(); }
    float getAdsrAttack() const { return adsrAttack->get(); }
    float getAdsrDecay() const { return adsrDecay->get(); }
    float getAdsrSustain() const { return adsrSustain->get(); }
    float getAdsrRelease() const { return adsrRelease->get(); }
    bool getIsAdsrOn() const { return adsrToggle->get(); }
    bool getIsMidiLatchOn() const { return latchIsOn.load(); }
    bool getIsIntervalLockOn() const { return intervalLockIsOn.load(); }
    int getStereoWidth() const { return stereoWidth->get(); }
    int getLowestPannedNote() const { return lowestPanned->get(); }
    int getMidiVelocitySensitivity() const { return velocitySens->get(); }
    int getPitchbendRange() const { return pitchBendRange->get(); }
    bool getIsVoiceStealingEnabled() const { return voiceStealing->get(); }
    int getConcertPitchHz() const { return concertPitchHz->get(); }
    bool getIsPedalPitchOn() const { return pedalPitchIsOn->get(); }
    int getPedalPitchThresh() const { return pedalPitchThresh->get(); }
    int getPedalPitchInterval() const { return pedalPitchInterval->get(); }
    bool getIsDescantOn() const { return descantIsOn->get(); }
    int getDescantThresh() const { return descantThresh->get(); }
    int getDescantInterval() const { return descantInterval->get(); }
    bool getIsLeadBypassed() const { return leadBypass->get(); }
    bool getIsHarmonyBypassed() const { return harmonyBypass->get(); }
    bool getIsDeEsserOn() const { return deEsserToggle->get(); }
    float getDeEsserThresh() const { return deEsserThresh->get(); }
    float getDeEsserAmount() const { return deEsserAmount->get(); }
    bool getIsReverbOn() const { return reverbToggle->get(); }
    
    
    // these functions return the default values for each parameter, according to the most recently loaded state from the host, or user-selected preset.
    int getDefaultDryPan() const { return defaultDryPan.load(); }
    int getDefaultDryWet() const { return defaultDryWet.load(); }
    float getDefaultInputGain() const { return defaultInputGain.load(); }
    float getDefaultOutputGain() const { return defaultOutputGain.load(); }
    float getDefaultAdsrAttack() const { return defaultAdsrAttack.load(); }
    float getDefaultAdsrDecay() const { return defaultAdsrDecay.load(); }
    float getDefaultAdsrSustain() const { return defaultAdsrSustain.load(); }
    float getDefaultAdsrRelease() const { return defaultAdsrRelease.load(); }
    int getDefaultStereoWidth() const { return defaultStereoWidth.load(); }
    int getDefaultLowestPannedNote() const { return defaultLowestPannedNote.load(); }
    int getDefaultMidiVelocitySensitivity() const { return defaultVelocitySensitivity.load(); }
    int getDefaultConcertPitchHz() const { return defaultConcertPitchHz.load(); }
    int getDefaultPedalPitchThresh() const { return defaultPedalPitchThresh.load(); }
    int getDefaultDescantThresh() const { return defaultDescantThresh.load(); }
    float getDefaultNoiseGateThresh() const { return defaultNoiseGateThresh.load(); }
    int getDefaultPitchbendRange() const { return defaultPitchbendRange.load(); }
    float getDefaultCompressorAmount() const { return defaultCompressorAmount.load(); }
    float getDefaultDeEsserThresh() const { return defaultDeEsserThresh.load(); }
    float getDefaultDeEsserAmount() const { return defaultDeEsserAmount.load(); }
    
    bool hasUpdatedParamDefaults()
    {
        const bool hasUpdated = parameterDefaultsAreDirty.load();
        parameterDefaultsAreDirty.store (false);
        return hasUpdated;
    }
    
    
    bav::MessageQueue paramChangesForEditor;
    
    enum parameterIDs
    {
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
        reverbToggleID
    };
    
    
private:
    
    bav::MessageQueue paramChangesForProcessor;
    
    template<typename SampleType>
    void updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    void processQueuedParameterChanges (bav::ImogenEngine<SampleType>& activeEngine);
    
    void updateVocalRangeType (int rangeTypeIndex);
    
    void updatePitchDetectionHzRange (int minHz, int maxHz);
    
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
    
    std::atomic<bool> latchIsOn, intervalLockIsOn;
    
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
    
    void updateParameterDefaults();
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    std::atomic<int> defaultDryPan, defaultDryWet, defaultStereoWidth, defaultLowestPannedNote, defaultVelocitySensitivity, defaultPitchbendRange, defaultPedalPitchThresh, defaultPedalPitchInterval, defaultDescantThresh, defaultDescantInterval, defaultConcertPitchHz;
    std::atomic<float> defaultAdsrAttack, defaultAdsrDecay, defaultAdsrSustain, defaultAdsrRelease, defaultInputGain, defaultOutputGain, defaultNoiseGateThresh, defaultCompressorAmount, defaultDeEsserThresh, defaultDeEsserAmount;
    
    int prevRangeTypeIndex;
    
    
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
