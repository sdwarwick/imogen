#pragma once

namespace Imogen
{
template <typename SampleType>
struct Reverb
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	Reverb (State& stateToUse);

	void process (AudioBuffer& audio);

	void prepare (double samplerate, int blocksize);

	void setWidth (float width);

private:

	State&		 state;
	ReverbState& parameters { state.parameters.reverbState };
	Meters&		 meters { state.meters };

	dsp::FX::Reverb reverb;
};

}  // namespace Imogen
