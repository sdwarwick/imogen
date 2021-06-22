
#pragma once

#include "ImogenEngine/ImogenEngine.h"

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

    IntParameter& getPitchbendParam();

private:
    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const final;

    double getTailLengthSeconds() const final;
    BoolParameter& getMainBypass() const final;

    bool acceptsMidi() const final { return true; }
    bool producesMidi() const final { return true; }
    bool supportsMPE() const final { return false; }
    bool isMidiEffect() const final { return false; }

    bool                        hasEditor() const final;
    juce::AudioProcessorEditor* createEditor() final;

    const String      getName() const final { return "Imogen"; }
    juce::StringArray getAlternateDisplayNames() const final { return {"Imgn"}; }

    
    State       state;
    Parameters& parameters {state.parameters};
    Internals&  internals {state.internals};
    Meters&     meters {state.meters};

    Engine< float >  floatEngine {state};
    Engine< double > doubleEngine {state};
    
    PluginTransport transport;

    network::OscDataSynchronizer dataSync {state};
};

}  // namespace Imogen
