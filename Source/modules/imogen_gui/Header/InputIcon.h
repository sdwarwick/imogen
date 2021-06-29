#pragma once

namespace Imogen
{

class InputIcon : public juce::Component
{
public:
    InputIcon (State& stateToUse);
    
private:
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    State& state;
    
    GainMeterParameter& inputMeter {*state.meters.inputLevel.get()};
    
    GainParameter& inputGain {*state.parameters.inputGain.get()};
};

}
