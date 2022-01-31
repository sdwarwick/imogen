#pragma once

#include <imogen_dsp/Engine/Harmonizer/Harmonizer.h>

namespace Imogen
{
template <typename SampleType>
class PitchCorrection : public dsp::psola::PitchCorrectorBase<SampleType>
{
public:

	using AudioBuffer = juce::AudioBuffer<SampleType>;
	using Base		  = dsp::psola::PitchCorrectorBase<SampleType>;

	PitchCorrection (Harmonizer<SampleType>& harm, Internals& internalsToUse);

	void renderNextFrame (int numSamples);

	void prepare (double samplerate, int blocksize);

	const AudioBuffer& getCorrectedSignal() const;

private:

	Internals& internals;

	AudioBuffer correctedBuffer;
	AudioBuffer alias;
};

}  // namespace Imogen
