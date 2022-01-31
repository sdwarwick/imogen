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

	plugin::GainMeterParameter& inputMeter { *state.meters.inputLevel };

	plugin::GainParameter& inputGain { *state.parameters.inputGain };
};

}  // namespace Imogen
