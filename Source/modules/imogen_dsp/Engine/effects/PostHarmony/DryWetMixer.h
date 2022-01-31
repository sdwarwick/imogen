#pragma once

namespace Imogen
{
template <typename SampleType>
struct DryWetMixer
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

	DryWetMixer (Parameters& params);

	void process (AudioBuffer& dry, AudioBuffer& wet);

	void prepare (double samplerate, int blocksize);

private:

	Parameters& parameters;

	dsp::FX::DryWetMixer<SampleType> mixer;
};

}  // namespace Imogen
