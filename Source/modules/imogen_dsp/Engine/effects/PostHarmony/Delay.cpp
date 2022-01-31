
namespace Imogen
{
template <typename SampleType>
Delay<SampleType>::Delay (State& stateToUse) : state (stateToUse)
{
}

template <typename SampleType>
void Delay<SampleType>::process (AudioBuffer& audio)
{
	if (parameters.delayToggle->get())
	{
		delay.setDryWet (parameters.delayDryWet->get());

		delay.process (audio);
		meters.delayLevel->set (static_cast<float> (delay.getAverageGainReduction()));
	}
	else
	{
		meters.delayLevel->set (-60.f);
	}
}

template <typename SampleType>
void Delay<SampleType>::prepare (double samplerate, int blocksize)
{
	delay.prepare (samplerate, blocksize);
}

template struct Delay<float>;
template struct Delay<double>;

}  // namespace Imogen
