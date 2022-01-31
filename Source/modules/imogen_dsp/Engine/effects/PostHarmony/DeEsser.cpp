
namespace Imogen
{
template <typename SampleType>
DeEsser<SampleType>::DeEsser (State& stateToUse) : state (stateToUse)
{
}

template <typename SampleType>
void DeEsser<SampleType>::process (AudioBuffer& dry, AudioBuffer& wet)
{
	if (parameters.deEsserToggle->get())
	{
		const auto thresh = parameters.deEsserThresh->get();
		const auto amount = parameters.deEsserAmount->get();

		dryDS.setThresh (thresh);
		dryDS.setDeEssAmount (amount);

		wetDS.setThresh (thresh);
		wetDS.setDeEssAmount (amount);

		dryDS.process (dry);
		wetDS.process (wet);

		meters.compRedux->set (static_cast<float> (dryDS.getAverageGainReduction() + wetDS.getAverageGainReduction()) * 0.5f);
	}
	else
	{
		meters.deEssRedux->set (0.f);
	}
}

template <typename SampleType>
void DeEsser<SampleType>::prepare (double samplerate, int blocksize)
{
	dryDS.prepare (samplerate, blocksize);
	wetDS.prepare (samplerate, blocksize);
}

template struct DeEsser<float>;
template struct DeEsser<double>;

}  // namespace Imogen
