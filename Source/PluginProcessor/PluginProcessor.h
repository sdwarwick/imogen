
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

#include <../../third-party/ableton-link/include/ableton/Link.hpp>

#include "GUI/Holders/ImogenGuiHolder.h"

#include "../OSC/OSC_sender.h"
#include "../OSC/OSC_reciever.h"


using namespace Imogen;


class ImogenAudioProcessorEditor; // forward declaration...

/*
*/

class ImogenAudioProcessor    : public juce::AudioProcessor,
                                public ImogenEventReciever,
                                public ImogenEventSender,
                                private juce::Timer
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
    
    void timerCallback() override final;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override final;
    
    void releaseResources() override final;
    
    void reset() override final;
    
    void processBlock (juce::AudioBuffer<float>& buffer,  juce::MidiBuffer& midiMessages) override final;
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override final;
    
    void processBlockBypassed (juce::AudioBuffer<float>& buffer,  juce::MidiBuffer& midiMessages) override final;
    void processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override final;
    
    bool canAddBus (bool isInput) const override { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override final;
    
    /* ImogenEventReciever functions */
    void recieveParameterChange             (ParameterID paramID, float newValue) override final;
    void recieveParameterChangeGestureStart (ParameterID paramID) override final;
    void recieveParameterChangeGestureEnd   (ParameterID paramID) override final;
    
    void recievePresetNameChange (const juce::String& newPresetName) override final;
    
    void recieveAbletonLinkChange (bool isNowEnabled) override final;
    
    void recieveMidiLatchEvent (bool isNowLatched) override final;
    void recieveKillAllMidiEvent() override final;
    void recieveEditorPitchbendEvent (int wheelValue) override final;
    
    void recieveMTSconnectionChange (bool) override final { }
    void recieveMTSscaleChange (const juce::String&) override final { }
    
    void loadPreset   (const juce::String& presetName);
    void savePreset   (const juce::String& presetName);
    void deletePreset (const juce::String& presetName);
    
    
    /* ImogenEventSender functions */
    void sendParameterChange             (ParameterID paramID, float newValue) override final;
    void sendParameterChangeGestureStart (ParameterID paramID) override final;
    void sendParameterChangeGestureEnd   (ParameterID paramID) override final;
    
    void sendSavePreset   (const juce::String& presetName) override final { }
    void sendLoadPreset   (const juce::String& presetName) override final { }
    void sendDeletePreset (const juce::String& presetName) override final { }
    
    void sendEditorPitchbend (int) override final { }
    void sendMidiLatch (bool) override final { }
    void sendEnableAbletonLink (bool) override final { }
    
    /* */
    
    double getTailLengthSeconds() const override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    inline juce::File getPresetsFolder() const { return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen"); }
    
    juce::AudioProcessorParameter* getBypassParameter() const override { return tree.getParameter ("mainBypass"); }
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    int indexOfProgram (const juce::String& name) const;
    void changeProgramName (int index, const juce::String& newName) override;
    
    juce::String getActivePresetName();
    
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool supportsMPE()  const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    const juce::String getName() const override { return JucePlugin_Name; }
    juce::StringArray getAlternateDisplayNames() const override { return { "Imgn" }; }
    
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    inline bool isMidiLatched() const
    {
        return isUsingDoublePrecision() ? doubleEngine.isMidiLatched() : floatEngine.isMidiLatched();
    }
    
    
    static constexpr auto msgQueueSize = size_t(100);
    
    // this queue is SPSC; this is only for events flowing from the editor into the processor
    bav::MessageQueue<msgQueueSize> nonParamEvents;
    

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
    
    
    Parameter* getParameterPntr (const ParameterID paramID) const;
    
    ParameterID parameterPntrToID (const Parameter* const) const;
    
    
    // enables saving and loading of the editor's last saved size
    juce::Point<int> getLastEditorSize() const { return savedEditorSize; }
    void saveEditorSize (const juce::Point<int>& size) { savedEditorSize = size; }
    
    
    // rescans the presets folder for available presets
    void rescanPresetsFolder();
    
    
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
    
    template<typename SampleType>
    void updateCompressor (bav::ImogenEngine<SampleType>& activeEngine,
                           bool compressorIsOn, float knobValue);
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    void initializeParameterListeners();
    void addParameterMessenger (ParameterID paramID);
    void updateParameterDefaults();
    
    bav::MessageQueue<msgQueueSize> paramChanges;
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState,
                                    bav::ImogenEngine<SampleType>& activeEngine,
                                    bool isPresetChange);
    
    
    ImogenGuiHolder* getActiveGui() const;
    
    
    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
    juce::Array< bav::MessageQueue<msgQueueSize>::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
    juce::AudioProcessorValueTreeState tree;
    
    template<typename PointerType>
    PointerType makeParameterPointer (const juce::String& name);
    
    // pointers to all the parameter objects
    BoolParamPtr  mainBypass, leadBypass, harmonyBypass, pedalPitchIsOn, descantIsOn, voiceStealing, limiterToggle, noiseGateToggle, compressorToggle, aftertouchGainToggle, deEsserToggle, reverbToggle, delayToggle;
    IntParamPtr   dryPan, dryWet, stereoWidth, lowestPanned, velocitySens, pitchBendRange, pedalPitchThresh, pedalPitchInterval, descantThresh, descantInterval, reverbDryWet, inputSource, delayDryWet;
    FloatParamPtr adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateThreshold, inputGain, outputGain, compressorAmount, deEsserThresh, deEsserAmount, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut;
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    // range object used to scale pitchbend values to and from the normalized 0.0-1.0 range
    juce::NormalisableRange<float> pitchbendNormalizedRange { 0.0f, 127.0f, 1.0f };
    
    
    /* simple attachment class that listens for chages in one specific parameter and pushes messages with the appropriate key value into the queue */
    class ParameterMessenger :  public juce::AudioProcessorParameter::Listener
    {
        using MsgQ = bav::MessageQueue<msgQueueSize>;
        
    public:
        ParameterMessenger(ImogenEventReciever& r, MsgQ& queue, bav::Parameter* p, ParameterID paramIDtoListen)
            : reciever(r), q(queue), param(p), paramID(paramIDtoListen)
        {
            jassert (param != nullptr);
        }
        
        void parameterValueChanged (int parameterIndex, float newValue) override
        {
            juce::ignoreUnused (parameterIndex);
            
            //value = param->normalize(value);
            jassert (newValue >= 0.0f && newValue <= 1.0f);
            q.pushMessage (paramID, newValue);  // the message will store a normalized value
            reciever.recieveParameterChange (paramID, newValue);
        }
        
        void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override
        {
            juce::ignoreUnused (parameterIndex);
            
            if (gestureIsStarting)
                reciever.recieveParameterChangeGestureStart (paramID);
            else
                reciever.recieveParameterChangeGestureEnd (paramID);
        }
        
        bav::Parameter* parameter() const noexcept { return param; }
        
    private:
        ImogenEventReciever& reciever;
        MsgQ& q;
        bav::Parameter* const param;
        const ParameterID paramID;
    };
    
    
    void changeMidiLatchState (bool isNowLatched);
    
    
    std::vector<ParameterMessenger> parameterMessengers; // all messengers are stored in here
    
    // this object represents the plugin as a participant in an Ableton Link session.
    ableton::Link abletonLink;
    
    juce::Point<int> savedEditorSize;
    
    juce::Array<juce::File> availablePresets;
    
    std::atomic<int> currentProgram;  // stores the current "program" number. -1 if no preset is active.
    
    std::atomic<bool> mts_wasConnected;
    juce::String mts_lastScaleName;
    
    juce::String lastPresetName;
    
    std::atomic<bool> abletonLink_wasEnabled;
    
    ImogenOSCSender   oscSender;
    
    juce::OSCReceiver oscReceiver;
    ImogenOSCReciever oscListener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
