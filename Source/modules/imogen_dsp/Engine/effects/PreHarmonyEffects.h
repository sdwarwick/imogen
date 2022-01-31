#pragma once

namespace Imogen
{
template <typename SampleType>
class PreHarmonyEffects
{
public:

	using AudioBuffer = juce::AudioBuffer<SampleType>;

	PreHarmonyEffects (State& stateToUse);

	void prepare (double samplerate, int blocksize);

	void process (const AudioBuffer& input);

	const SampleType* getProcessedInputSignal() const;

private:

	AudioBuffer processedMonoBuffer;

	State& state;

	StereoReducer<SampleType>	stereoReducer { state.parameters };
	dsp::FX::Filter<SampleType> initialLoCut { dsp::FX::FilterType::HighPass, 65.f };
	InputGain<SampleType>		inputGain { state };
	NoiseGate<SampleType>		gate { state };
};

}  // namespace Imogen
