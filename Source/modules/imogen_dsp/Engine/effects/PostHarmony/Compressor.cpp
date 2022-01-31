
namespace Imogen
{
template <typename SampleType>
Compressor<SampleType>::Compressor (State& stateToUse)
	: state (stateToUse)
{
	//    static constexpr auto compressorAttackMs  = 4.0f;
	//    static constexpr auto compressorReleaseMs = 200.0f;
}

template <typename SampleType>
void Compressor<SampleType>::process (AudioBuffer& dry, AudioBuffer& wet)
{
	if (parameters.compToggle->get())
	{
		updateCompressorAmount (parameters.compAmount->get());

		dryComp.process (dry);
		wetComp.process (wet);

		meters.compRedux->set (static_cast<float> (dryComp.getAverageGainReduction() + wetComp.getAverageGainReduction()) * 0.5f);
	}
	else
	{
		meters.compRedux->set (0.f);
	}
}

template <typename SampleType>
void Compressor<SampleType>::updateCompressorAmount (int amount)
{
	const auto a = static_cast<float> (amount) * 0.01f;

	const auto thresh = juce::jmap (a, 0.f, -60.f);
	const auto ratio  = juce::jmap (a, 1.f, 10.f);

	dryComp.setThreshold (thresh);
	dryComp.setRatio (ratio);

	wetComp.setThreshold (thresh);
	wetComp.setRatio (ratio);
}

template <typename SampleType>
void Compressor<SampleType>::prepare (double samplerate, int blocksize)
{
	dryComp.prepare (samplerate, blocksize);
	wetComp.prepare (samplerate, blocksize);
}

template struct Compressor<float>;
template struct Compressor<double>;

}  // namespace Imogen
