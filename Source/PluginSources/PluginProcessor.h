
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
 
 PluginProcessor.h: This file defines the core interface for Imogen's AudioProcessor as a whole. The class ImogenAudioProcessor is the top-level object that represents an instance of Imogen.
 
======================================================================================================================================================*/


#pragma once

#include "bv_ImogenEngine/bv_ImogenEngine.h"

#include "../../third-party/ableton-link/include/ableton/Link.hpp"


class ImogenAudioProcessorEditor; // forward declaration...

///////////

class ImogenAudioProcessor    : public juce::AudioProcessor
{
    using Parameter      = bav::Parameter;
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
    
    bool canAddBus (bool isInput) const override { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    
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
        voiceStealingID,
        inputGainID,
        outputGainID,
        limiterToggleID,
        noiseGateToggleID,
        noiseGateThresholdID,
        compressorToggleID,
        compressorAmountID,
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
#define IMGN_NUM_PARAMS reverbHiCutID + 1
    
    // IDs for events from the editor that are not parameters
    enum eventID
    {
        killAllMidi,
        midiLatch,
        pitchBendFromEditor
    };
    
    // tracks whether the processor has updated its parameter defaults since the last time this function was called
    bool hasUpdatedParamDefaults();
    
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
    
    void editorPitchbend (int wheelValue);
    
    
    inline bool isMidiLatched() const
    {
        return isUsingDoublePrecision() ? doubleEngine.isMidiLatched() : floatEngine.isMidiLatched();
    }
    
    
    static constexpr auto msgQueueSize = size_t(100);
    
    // this queue is SPSC; this is only for events flowing from the editor into the processor
    bav::MessageQueue<msgQueueSize> nonParamEvents;
    
    
    void setOscMessagesEnabled (bool shouldBeEnabled) { oscMapper.setEnabled (shouldBeEnabled); }
    bool areOscMessagesEnabled() const noexcept { return oscMapper.areOscMessagesEnabled(); }
    
    
    static constexpr int maxPossibleNumVoices = 20;
    
    
    bool isAbletonLinkEnabled() const { return abletonLink.isEnabled(); }
    
    int getNumAbletonLinkSessionPeers() const { return abletonLink.isEnabled() ? (int)abletonLink.numPeers() : 0; }
    
    
    bool isConnectedToMtsEsp() const noexcept
    {
        return isUsingDoublePrecision() ? doubleEngine.isConnectedToMtsEsp() : floatEngine.isConnectedToMtsEsp();
    }
    
    juce::String getScaleName() const
    {
        return isUsingDoublePrecision() ? doubleEngine.getScaleName() : floatEngine.getScaleName();
    }
    
    
    Parameter* getParameterPntr (const parameterID paramID) const;
    
    parameterID parameterPntrToID (const Parameter* const) const;
    
    
private:
    
    template <typename SampleType>
    void initialize (bav::ImogenEngine<SampleType>& activeEngine);
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
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
    
    void updateNumVoices (const int newNumVoices);
    
    template<typename SampleType>
    void updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                           bool compressorIsOn, float knobValue);
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    void initializeParameterListeners();
    void addParameterMessenger (parameterID paramID);
    void updateParameterDefaults();
    
    void initializeParameterOscMappings();
    
    bav::MessageQueue<msgQueueSize> paramChanges;
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine);
    
    
    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
    juce::Array< bav::MessageQueue<msgQueueSize>::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
    juce::AudioProcessorValueTreeState tree;
    
    // pointers to all the parameter objects
    BoolParamPtr  mainBypass, leadBypass, harmonyBypass, pedalPitchIsOn, descantIsOn, voiceStealing, limiterToggle, noiseGateToggle, compressorToggle, aftertouchGainToggle, deEsserToggle, reverbToggle;
    IntParamPtr   dryPan, dryWet, stereoWidth, lowestPanned, velocitySens, pitchBendRange, pedalPitchThresh, pedalPitchInterval, descantThresh, descantInterval, reverbDryWet, numVoices, inputSource;
    FloatParamPtr adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateThreshold, inputGain, outputGain, compressorAmount, deEsserThresh, deEsserAmount, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut;
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    // range object used to scale pitchbend values to and from the normalized 0.0-1.0 range
    juce::NormalisableRange<float> pitchbendNormalizedRange { 0.0f, 127.0f, 1.0f };
    
    
    
    /* simple attachment class that listens for chages in one specific parameter and pushes messages with the appropriate key value into the queue */
    class ParameterMessenger :  public juce::AudioProcessorValueTreeState::Listener
    {
        using MsgQ = bav::MessageQueue<msgQueueSize>;
        
    public:
        ParameterMessenger(MsgQ& queue, bav::Parameter* p, int paramIDtoListen)
            : q(queue), param(p), paramID(paramIDtoListen)
        {
            jassert (param != nullptr);
        }
        
        void parameterChanged (const juce::String& s, float value) override
        {
            juce::ignoreUnused (s);
            value = param->normalize(value);
            jassert (value >= 0.0f && value <= 1.0f);
            q.pushMessage (paramID, value);  // the message will store a normalized value
        }
        
    private:
        MsgQ& q;
        bav::Parameter* const param;
        const int paramID;
    };
    
    
    std::vector<ParameterMessenger> parameterMessengers; // all messengers are stored in here
    
    // this object manages all parameters' mappings to OSC messages
    bav::OSCMappingManager oscMapper;
    
    
    // this object represents the plugin as a participant in an Ableton Link session.
    ableton::Link abletonLink;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
