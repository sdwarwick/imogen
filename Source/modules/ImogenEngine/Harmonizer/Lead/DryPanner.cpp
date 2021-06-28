
namespace Imogen
{
template < typename SampleType >
DryPanner< SampleType >::DryPanner (Parameters& params) : parameters (params)
{
}

template < typename SampleType >
void DryPanner< SampleType >::process (const AudioBuffer& monoIn, AudioBuffer& stereoOut, bool bypassed)
{
    if (bypassed)
    {
        stereoOut.clear();
    }
    else
    {
        panner.setMidiPan (parameters.leadPan->get());
        panner.process (monoIn, stereoOut);
    }
}

template < typename SampleType >
void DryPanner< SampleType >::prepare (double samplerate, int blocksize)
{
    panner.prepare (samplerate, blocksize);
}

template struct DryPanner< float >;
template struct DryPanner< double >;

}  // namespace Imogen
