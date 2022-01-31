
namespace Imogen
{
template <typename SampleType>
StereoReducer<SampleType>::StereoReducer (Parameters& params) : parameters (params)
{
}

template <typename SampleType>
void StereoReducer<SampleType>::process (const AudioBuffer& stereoInput, AudioBuffer& monoOutput)
{
	using Mode = typename dsp::FX::MonoStereoConverter<SampleType>::StereoReductionMode;

	switch (parameters.inputMode->get())
	{
		case (1) : reducer.setStereoReductionMode (Mode::rightOnly); break;
		case (2) : reducer.setStereoReductionMode (Mode::mixToMono); break;
		default : reducer.setStereoReductionMode (Mode::leftOnly); break;
	}

	reducer.convertStereoToMono (stereoInput, monoOutput);
}

template <typename SampleType>
void StereoReducer<SampleType>::prepare (double, int blocksize)
{
	reducer.prepare (blocksize);
}

template struct StereoReducer<float>;
template struct StereoReducer<double>;

}  // namespace Imogen
