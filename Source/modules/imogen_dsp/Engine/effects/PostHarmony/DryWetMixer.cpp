
namespace Imogen
{
template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer (Parameters& params) : parameters (params)
{
}

template <typename SampleType>
void DryWetMixer<SampleType>::process (AudioBuffer& dry, AudioBuffer& wet)
{
	mixer.setWetMix (parameters.dryWet->get());
	mixer.process (dry, wet);
}

template <typename SampleType>
void DryWetMixer<SampleType>::prepare (double samplerate, int blocksize)
{
	mixer.prepare (2, blocksize, samplerate);
}

template struct DryWetMixer<float>;
template struct DryWetMixer<double>;

}  // namespace Imogen
