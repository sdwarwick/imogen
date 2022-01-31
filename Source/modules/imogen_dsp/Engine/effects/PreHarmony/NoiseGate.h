#pragma once

namespace Imogen
{
template <typename SampleType>
struct NoiseGate
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	NoiseGate (State& stateToUse);

	void process (AudioBuffer& audio);

	void prepare (double samplerate, int blocksize);

private:

	State&		state;
	Parameters& parameters { state.parameters };
	Meters&		meters { state.meters };

	dsp::FX::NoiseGate<SampleType> gate;
};

}  // namespace Imogen
