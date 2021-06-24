
namespace Imogen
{
template < typename SampleType >
Reverb< SampleType >::Reverb (State& stateToUse) : state (stateToUse)
{
}

template < typename SampleType >
void Reverb< SampleType >::process (AudioBuffer& audio)
{
    if (parameters.reverbToggle->get())
    {
        reverb.setDryWet (parameters.reverbDryWet->get());
        reverb.setDuckAmount (parameters.reverbDuck->get());
        reverb.setLoCutFrequency (parameters.reverbLoCut->get());
        reverb.setHiCutFrequency (parameters.reverbHiCut->get());

        const auto d = static_cast< float > (parameters.reverbDecay->get()) * 0.01f;
        reverb.setDamping (1.f - d);
        reverb.setRoomSize (d);

        SampleType level;
        reverb.process (audio, &level);
        meters.reverbLevel->set (static_cast< float > (level));
    }
    else
    {
        meters.reverbLevel->set (-60.f);
    }
}

template < typename SampleType >
void Reverb< SampleType >::prepare (double samplerate, int blocksize)
{
    reverb.prepare (blocksize, samplerate, 2);
}

template < typename SampleType >
void Reverb< SampleType >::setWidth (float width)
{
    reverb.setWidth (width);
}

template struct Reverb< float >;
template struct Reverb< double >;

}  // namespace Imogen
