#pragma once

namespace Imogen
{
template <typename SampleType>
struct OutputGain
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	OutputGain (Parameters& params);

	void process (AudioBuffer& audio);

	void prepare (double samplerate, int blocksize);

private:

	Parameters& parameters;

	dsp::FX::SmoothedGain<SampleType, 2> gain;
};

}  // namespace Imogen
