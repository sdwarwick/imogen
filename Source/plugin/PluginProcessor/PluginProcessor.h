
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
    void renderChunk (juce::AudioBuffer< float >& audio, juce::MidiBuffer& midi) final;
    void renderChunk (juce::AudioBuffer< double >& audio, juce::MidiBuffer& midi) final;
    
    template < typename SampleType >
    void renderChunkInternal (Engine< SampleType >& engine, juce::AudioBuffer< SampleType >& audio, juce::MidiBuffer& midi);
    
    void updateMeters (ImogenMeterData meterData);
    void updateInternals (ImogenInternalsData internalsData);
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) final;
    
    template < typename SampleType1, typename SampleType2 >
    void prepareToPlayWrapped (const double sampleRate, Engine< SampleType1 >& activeEngine, Engine< SampleType2 >& idleEngine);
    
    template < typename SampleType >
    void initialize (Engine< SampleType >& activeEngine);
    
    template < typename SampleType >
    void initializeParameterFunctionPointers (Engine< SampleType >& engine);

    void releaseResources() final;

    BusesProperties createBusProperties() const final;
    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const final;

    double getTailLengthSeconds() const final;

    BoolParameter&    getMainBypass() const final;
    SerializableData& getStateData() final { return state; }
    
    bool acceptsMidi() const final { return true; }
    bool producesMidi() const final { return true; }
    bool supportsMPE() const final { return false; }
    bool isMidiEffect() const final { return false; }
    
    bool                        hasEditor() const final;
    juce::AudioProcessorEditor* createEditor() final;
    
    const String      getName() const final { return "Imogen"; }
    juce::StringArray getAlternateDisplayNames() const final { return {"Imgn"}; }

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

    /*=========================================================================================*/

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

}  // namespace Imogen
