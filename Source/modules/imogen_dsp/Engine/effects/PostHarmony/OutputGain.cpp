
namespace Imogen
{
template <typename SampleType>
OutputGain<SampleType>::OutputGain (Parameters& params) : parameters (params)
{
}

template <typename SampleType>
void OutputGain<SampleType>::process (AudioBuffer& audio)
{
	gain.setGain (parameters.outputGain->get());
	gain.process (audio);
}

template <typename SampleType>
void OutputGain<SampleType>::prepare (double samplerate, int blocksize)
{
	gain.prepare (samplerate, blocksize);
}

template struct OutputGain<float>;
template struct OutputGain<double>;

}  // namespace Imogen
