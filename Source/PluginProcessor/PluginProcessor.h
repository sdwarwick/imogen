
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

#include "GUI/Holders/ImogenGuiHolder.h"

#include "../ImogenParameters.h"

#if IMOGEN_USE_ABLETON_LINK
  #include <../../third-party/ableton-link/include/ableton/Link.hpp>
#endif

#if IMOGEN_USE_OSC
  #include "../OSC/OSC.h"
#endif


#undef IMOGEN_PROCESSOR_TIMER
#if IMOGEN_REMOTE_APP || ! IMOGEN_HEADLESS
  #define IMOGEN_PROCESSOR_TIMER 1
#else
  #define IMOGEN_PROCESSOR_TIMER 0
#endif


using namespace Imogen;


class ImogenAudioProcessorEditor; // forward declaration...

/*
*/

class ImogenAudioProcessor    : private bav::TranslationInitializer,
                                public  juce::AudioProcessor,
                                public  ProcessorStateChangeReciever
#if IMOGEN_PROCESSOR_TIMER
                              , private juce::Timer
#endif
{
    using Parameter      = bav::Parameter;
    using FloatParameter = ImogenFloatParameter;
    using IntParameter   = ImogenIntParameter;
    using BoolParameter  = ImogenBoolParameter;
    
    using FloatParamPtr = FloatParameter*;
    using IntParamPtr   = IntParameter*;
    using BoolParamPtr  = BoolParameter*;

    
public:
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;
    
#if IMOGEN_PROCESSOR_TIMER
    void timerCallback() override final;
#endif
    
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
    
    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override final;
    
    double getTailLengthSeconds() const override final;
    
    void getStateInformation (juce::MemoryBlock& destData) override final;
    void setStateInformation (const void* data, int sizeInBytes) override final;
    
    juce::AudioProcessorParameter* getBypassParameter() const override final { return tree.getParameter ("mainBypass"); }
    
    int getNumPrograms() override final { return 1; }
    int getCurrentProgram() override final { return 0; }
    void setCurrentProgram (int) override final { }
    const juce::String getProgramName (int) override final { return {}; }
    void changeProgramName (int, const juce::String&) override final { }
    
    bool acceptsMidi()  const override final { return true;  }
    bool producesMidi() const override final { return true;  }
    bool supportsMPE()  const override final { return false; }
    bool isMidiEffect() const override final { return false; }
    
    const juce::String getName() const override final { return JucePlugin_Name; }
    juce::StringArray getAlternateDisplayNames() const override final { return { "Imgn" }; }
    
    bool hasEditor() const override final;
    
    juce::AudioProcessorEditor* createEditor() override final;
    
    bool supportsDoublePrecisionProcessing() const override final { return true; }
    
    /*=========================================================================================*/
    
    inline bool isMidiLatched() const;
    
    bool isAbletonLinkEnabled() const;
    
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
    
    template <typename SampleType>
    void initializeParameterFunctionPointers (bav::ImogenEngine<SampleType>& engine);
    
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
    inline void processQueuedNonParamEvents (bav::ImogenEngine<SampleType>& activeEngine);
    
    void changeMidiLatchState (bool isNowLatched);
    
    /*=========================================================================================*/

    void updateEditorSizeFromAPVTS();
    
    inline ImogenGuiHolder* getActiveGui() const;
    
    inline Parameter* getParameterPntr (const ParameterID paramID) const;
    
    /*=========================================================================================*/
    
    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
    juce::AudioProcessorValueTreeState tree;
    
    std::vector< Parameter* > parameterPointers;
    Parameter* mainBypassPntr;
    
    // range object used to scale pitchbend values to and from the normalized 0.0-1.0 range
    juce::NormalisableRange<float> pitchbendNormalizedRange { 0.0f, 127.0f, 1.0f };
    
#if IMOGEN_USE_ABLETON_LINK
    ableton::Link abletonLink;  // this object represents the plugin as a participant in an Ableton Link session.
#endif
    
    juce::Point<int> savedEditorSize;
    
    static constexpr auto msgQueueSize = size_t(100);
    bav::MessageQueue<msgQueueSize> nonParamEvents;  // this queue is SPSC; this is only for events flowing from the editor into the processor
    juce::Array< bav::MessageQueue<msgQueueSize>::Message >  currentMessages;  // this array stores the current messages from the message FIFO
    
#if IMOGEN_USE_OSC
    ImogenOSCSender   oscSender;
    juce::OSCReceiver oscReceiver;
    // ImogenOSCReciever<juce::OSCReceiver::RealtimeCallback> oscListener;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
