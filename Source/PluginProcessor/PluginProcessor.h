
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

#include "../OSC/OSC.h"


using namespace Imogen;


class ImogenAudioProcessorEditor; // forward declaration...

/*
*/

class ImogenAudioProcessor    : public juce::AudioProcessor,
                                public ProcessorStateChangeReciever,
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
    
    /*=========================================================================================*/
    /* ProcessorStateChangeReciever functions -- for external sources (GUI, OSC) to update the processor's state */
    
    void recieveExternalParameterChange  (ParameterID param, float newValue) override final;
    void recieveExternalParameterGesture (ParameterID param, bool gestureStart) override final;
    
    /*=========================================================================================*/
    /* juce::AudioProcessor functions */

    void prepareToPlay (double sampleRate, int samplesPerBlock) override final;
    
    void releaseResources() override final;
    
    void reset() override final;
    
    void processBlock (juce::AudioBuffer<float>& buffer,  juce::MidiBuffer& midiMessages) override final;
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override final;
    
    void processBlockBypassed (juce::AudioBuffer<float>& buffer,  juce::MidiBuffer& midiMessages) override final;
    void processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override final;
    
    bool canAddBus (bool isInput) const override { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override final;
    
    double getTailLengthSeconds() const override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorParameter* getBypassParameter() const override { return tree.getParameter ("mainBypass"); }
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    int indexOfProgram (const juce::String& name) const;
    void changeProgramName (int index, const juce::String& newName) override;
    
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool supportsMPE()  const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    const juce::String getName() const override { return JucePlugin_Name; }
    juce::StringArray getAlternateDisplayNames() const override { return { "Imgn" }; }
    
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    /*=========================================================================================*/
    
    inline bool isMidiLatched() const;
    
    bool isAbletonLinkEnabled() const { return abletonLink.isEnabled(); }
    
    int getNumAbletonLinkSessionPeers() const;
    
    bool isConnectedToMtsEsp() const noexcept;
    
    juce::String getScaleName() const;
    
    /*=========================================================================================*/
    
    juce::Point<int> getLastEditorSize() const { return savedEditorSize; }
    
    void saveEditorSize (int width, int height);
    
    /*=========================================================================================*/
    
private:
    
    /*=========================================================================================*/
    /* Initialization functions */
    
    template <typename SampleType>
    void initialize (bav::ImogenEngine<SampleType>& activeEngine);
    
    BusesProperties makeBusProperties() const;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    
    /*=========================================================================================*/
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate,
                               bav::ImogenEngine<SampleType1>& activeEngine,
                               bav::ImogenEngine<SampleType2>& idleEngine);
    
    template <typename SampleType>
    inline void processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                     juce::MidiBuffer& midiMessages,
                                     bav::ImogenEngine<SampleType>& engine,
                                     const bool isBypassedThisCallback);
    
    /*=========================================================================================*/
    
    template<typename SampleType>
    void processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine);
    
    void changeMidiLatchState (bool isNowLatched);
    
    /*=========================================================================================*/

    ImogenGuiHolder* getActiveGui() const;
    
    Parameter* getParameterPntr (const ParameterID paramID) const;
    
    /*=========================================================================================*/
    
    // simple hack to force the translation system to initialize before anything else in our initialization list
    bav::TranslationInitializer translationInitializer;
    
    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
    juce::AudioProcessorValueTreeState tree;
    
    std::vector< Parameter* > parameterPointers;
    
    // range object used to scale pitchbend values to and from the normalized 0.0-1.0 range
    juce::NormalisableRange<float> pitchbendNormalizedRange { 0.0f, 127.0f, 1.0f };
    
    ableton::Link abletonLink;  // this object represents the plugin as a participant in an Ableton Link session.
    
    juce::Point<int> savedEditorSize;
    
    static constexpr auto msgQueueSize = size_t(100);
    bav::MessageQueue<msgQueueSize> nonParamEvents;  // this queue is SPSC; this is only for events flowing from the editor into the processor
    
    juce::Array< bav::MessageQueue<msgQueueSize>::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
    ImogenOSCSender   oscSender;
    juce::OSCReceiver oscReceiver;
    // ImogenOSCReciever<juce::OSCReceiver::RealtimeCallback> oscListener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
