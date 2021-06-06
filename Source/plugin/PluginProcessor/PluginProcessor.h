
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

    void prepareToPlay (double sampleRate, int samplesPerBlock) final;

    void releaseResources() final;

    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const final;

    double getTailLengthSeconds() const final;

    void getStateInformation (juce::MemoryBlock& destData) final;
    void setStateInformation (const void* data, int sizeInBytes) final;

    bav::BoolParameter* getMainBypass() const final;

    bool acceptsMidi() const final { return true; }
    bool producesMidi() const final { return true; }
    bool supportsMPE() const final { return false; }
    bool isMidiEffect() const final { return false; }

    const String      getName() const final { return "Imogen"; }
    juce::StringArray getAlternateDisplayNames() const final { return {"Imgn"}; }

    bool                        hasEditor() const final;
    juce::AudioProcessorEditor* createEditor() final;

    bool supportsDoublePrecisionProcessing() const final { return true; }

    /*=========================================================================================*/
    /* Initialization functions */

    BusesProperties createBusProperties() const final;

    template < typename SampleType >
    void initialize (Engine< SampleType >& activeEngine);

    template < typename SampleType >
    void initializeParameterFunctionPointers (Engine< SampleType >& engine);

    /*=========================================================================================*/

    template < typename SampleType1, typename SampleType2 >
    void prepareToPlayWrapped (const double sampleRate, Engine< SampleType1 >& activeEngine, Engine< SampleType2 >& idleEngine);

    template < typename SampleType >
    inline void processBlockInternal (juce::AudioBuffer< SampleType >& buffer,
                                      juce::MidiBuffer&                midiMessages);
    
    template < typename SampleType >
    void renderChunk (juce::AudioBuffer< SampleType >& audio, juce::MidiBuffer& midi, Engine<SampleType>& engine);

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
    
    /*=========================================================================================*/
    
    struct ParameterProcessor : ParameterProcessorBase
    {
        ParameterProcessor (Processor& p, ParameterList& l);
        
    private:
        void renderChunk (juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) final;
        void renderChunk (juce::AudioBuffer<double>& audio, juce::MidiBuffer& midi) final;
        
        Processor& processor;
    };
    
    ParameterProcessor parameterProcessor {*this, parameters};
    
    /*=========================================================================================*/

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};

}  // namespace Imogen
