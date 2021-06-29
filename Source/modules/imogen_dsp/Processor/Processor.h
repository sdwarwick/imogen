
#pragma once

#include <imogen_dsp/Engine/Engine.h>

namespace Imogen
{
class Processor : public dsp::ProcessorBase
{
public:
    Processor();
    
protected:
    State state;

private:
    bool canAddBus (bool isInput) const override final { return isInput; }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const final;

    double getTailLengthSeconds() const final;
    BoolParameter& getMainBypass() const final;

    bool acceptsMidi() const final { return true; }
    bool producesMidi() const final { return true; }
    bool supportsMPE() const final { return false; }
    bool isMidiEffect() const final { return false; }

    bool                        hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    const String      getName() const final { return "Imogen"; }
    juce::StringArray getAlternateDisplayNames() const final { return {"Imgn"}; }
    
    Parameters& parameters {state.parameters};
    
    Engine< float >  floatEngine {state};
    Engine< double > doubleEngine {state};
    
    PluginTransport transport;

    network::OscDataSynchronizer dataSync {state};
};

}  // namespace Imogen
