#pragma once

namespace Imogen
{
template <typename SampleType>
struct Delay
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	Delay (State& stateToUse);

	void process (AudioBuffer& audio);

	void prepare (double samplerate, int blocksize);

private:

	State&		state;
	Parameters& parameters { state.parameters };
	Meters&		meters { state.meters };

	dsp::FX::Delay<SampleType> delay;
};

}  // namespace Imogen
