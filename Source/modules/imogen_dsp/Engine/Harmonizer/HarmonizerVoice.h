
#pragma once


namespace Imogen
{
using namespace lemons;

template <typename SampleType>
class Harmonizer;  // forward declaration...


template <typename SampleType>
class HarmonizerVoice : public dsp::SynthVoiceBase<SampleType>
{
	using AudioBuffer = juce::AudioBuffer<SampleType>;

public:

	HarmonizerVoice (Harmonizer<SampleType>& h, dsp::psola::Analyzer<SampleType>& analyzerToUse);

private:

	void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate) final;

	dsp::psola::Shifter<SampleType> shifter;
};


}  // namespace Imogen
