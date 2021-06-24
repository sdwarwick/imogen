
namespace Imogen
{
template < typename SampleType >
NoiseGate< SampleType >::NoiseGate (State& stateToUse) : state (stateToUse)
{
    //    static constexpr auto noiseGateAttackMs   = 25.0f;
    //    static constexpr auto noiseGateReleaseMs  = 100.0f;
    //    static constexpr auto noiseGateFloorRatio = 10.0f;  // ratio to one when the noise gate is activated
}

template < typename SampleType >
void NoiseGate< SampleType >::process (AudioBuffer& audio)
{
    if (parameters.noiseGateToggle->get())
    {
        gate.setThreshold (parameters.noiseGateThresh->get());

        gate.process (audio);

        meters.gateRedux->set (static_cast< float > (gate.getAverageGainReduction()));
    }
    else
    {
        meters.gateRedux->set (0.f);
    }
}

template < typename SampleType >
void NoiseGate< SampleType >::prepare (double samplerate, int blocksize)
{
    gate.prepare (samplerate, blocksize);
}

template struct NoiseGate< float >;
template struct NoiseGate< double >;

}  // namespace Imogen
