
#pragma once

#include "ImogenEngine/ImogenEngine.h"
#include "ImogenCommon/ImogenCommon.h"

#ifndef IMOGEN_HEADLESS
#    define IMOGEN_HEADLESS 0
#endif

namespace Imogen
{
class Processor : public dsp::ProcessorBase
{
public:
    Processor();
    ~Processor() override;

    String getScaleName() const;

    State& getState() { return state; }

private:
    /*=========================================================================================*/
    /* juce::AudioProcessor functions */

    void prepareToPlay (double sampleRate, int samplesPerBlock) override final;

    void releaseResources() override final;

    void processBlock (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages) override final;
    void processBlock (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages) override final;

    void processBlockBypassed (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages) override final;
    void processBlockBypassed (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages) override final;

    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override final;

    double getTailLengthSeconds() const override final;

    void getStateInformation (juce::MemoryBlock& destData) override final;
    void setStateInformation (const void* data, int sizeInBytes) override final;

    juce::AudioProcessorParameter* getBypassParameter() const override final;

    bool acceptsMidi() const override final { return true; }
    bool producesMidi() const override final { return true; }
    bool supportsMPE() const override final { return false; }
    bool isMidiEffect() const override final { return false; }

    const String      getName() const override final { return "Imogen"; }
    juce::StringArray getAlternateDisplayNames() const override final { return {"Imgn"}; }

    bool                        hasEditor() const override final;
    juce::AudioProcessorEditor* createEditor() override final;

    bool supportsDoublePrecisionProcessing() const override final { return true; }

    /*=========================================================================================*/
    /* Initialization functions */

    BusesProperties createBusProperties() const override final;

    template < typename SampleType >
    void initialize (Engine< SampleType >& activeEngine);

    template < typename SampleType >
    void initializeParameterFunctionPointers (Engine< SampleType >& engine);

    /*=========================================================================================*/

    template < typename SampleType1, typename SampleType2 >
    void prepareToPlayWrapped (const double sampleRate, Engine< SampleType1 >& activeEngine, Engine< SampleType2 >& idleEngine);

    template < typename SampleType >
    inline void processBlockWrapped (juce::AudioBuffer< SampleType >& buffer,
                                     juce::MidiBuffer&                midiMessages,
                                     Engine< SampleType >&            engine,
                                     const bool                       isBypassedThisCallback);

    void updateMeters (ImogenMeterData meterData);
    void updateInternals (ImogenInternalsData internalsData);

    /*=========================================================================================*/

    // one engine of each type. The idle one isn't destroyed, but takes up few resources.
    Engine< float >  floatEngine;
    Engine< double > doubleEngine;

    State       state;
    Parameters& parameters {state.parameters};
    Internals&  internals {state.internals};
    Meters&     meters {state.meters};

    network::SelfOwnedOscDataSynchronizer dataSync {state};

    PluginTransport transport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

}  // namespace Imogen
