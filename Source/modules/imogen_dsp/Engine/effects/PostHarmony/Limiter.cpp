
namespace Imogen
{
template <typename SampleType>
Limiter<SampleType>::Limiter (State& stateToUse) : state (stateToUse)
{
	//    static constexpr auto limiterThreshDb     = 0.0f;
	//    static constexpr auto limiterReleaseMs    = 35.0f;
}

template <typename SampleType>
void Limiter<SampleType>::process (AudioBuffer& audio)
{
	if (parameters.limiterToggle->get())
	{
		limiter.process (audio);
		meters.limRedux->set (static_cast<float> (limiter.getAverageGainReduction()));
	}
	else
	{
		meters.limRedux->set (0.f);
	}

	const auto numSamples = audio.getNumSamples();
	meters.outputLevelL->set (static_cast<float> (audio.getRMSLevel (0, 0, numSamples)));
	meters.outputLevelR->set (static_cast<float> (audio.getRMSLevel (1, 0, numSamples)));
}

template <typename SampleType>
void Limiter<SampleType>::prepare (double samplerate, int blocksize)
{
	limiter.prepare (samplerate, blocksize);
}

template struct Limiter<float>;
template struct Limiter<double>;

}  // namespace Imogen
