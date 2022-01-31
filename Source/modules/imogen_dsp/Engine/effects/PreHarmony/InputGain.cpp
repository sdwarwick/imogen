
namespace Imogen
{
template <typename SampleType>
InputGain<SampleType>::InputGain (State& stateToUse) : state (stateToUse)
{
}

template <typename SampleType>
void InputGain<SampleType>::process (AudioBuffer& audio)
{
	gain.setGain (parameters.inputGain->get());
	gain.process (audio);

	meters.inputLevel->set (static_cast<float> (audio.getRMSLevel (0, 0, audio.getNumSamples())));
}

template <typename SampleType>
void InputGain<SampleType>::prepare (double samplerate, int blocksize)
{
	gain.prepare (samplerate, blocksize);
}

template struct InputGain<float>;
template struct InputGain<double>;

}  // namespace Imogen
