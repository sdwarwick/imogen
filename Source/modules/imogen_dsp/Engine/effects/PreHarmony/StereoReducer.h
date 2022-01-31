#pragma once

namespace Imogen
{
template <typename SampleType>
struct StereoReducer
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	StereoReducer (Parameters& params);

	void process (const AudioBuffer& stereoInput, AudioBuffer& monoOutput);

	void prepare (double samplerate, int blocksize);

private:

	Parameters& parameters;

	dsp::FX::MonoStereoConverter<SampleType> reducer;
};

}  // namespace Imogen
