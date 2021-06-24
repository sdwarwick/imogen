
namespace Imogen
{
template < typename SampleType >
EQ< SampleType >::EQ (Parameters& params)
    : parameters (params)
{
    using FT = dsp::FX::FilterType;

    dryEQ.addBand (FT::LowPass, 10000.f);
    dryEQ.addBand (FT::HighPass, 80.f);
    dryEQ.addBand (FT::Peak, 2500.f);

    wetEQ.addBand (FT::LowPass, 10000.f);
    wetEQ.addBand (FT::HighPass, 80.f);
    wetEQ.addBand (FT::Peak, 2500.f);
}

template < typename SampleType >
void EQ< SampleType >::process (AudioBuffer& dry, AudioBuffer& wet)
{
    using FT = dsp::FX::FilterType;

    if (parameters.eqToggle->get())
    {
        const auto lowPassFreq = parameters.eqLowPassFreq->get();
        const auto lowPassQ    = parameters.eqLowPassQ->get();

        if (auto* dryLowPass = dryEQ.getBandOfType (FT::LowPass))
        {
            dryLowPass->setFilterFrequency (lowPassFreq);
            dryLowPass->setQfactor (lowPassQ);
        }
        if (auto* wetLowPass = wetEQ.getBandOfType (FT::LowPass))
        {
            wetLowPass->setFilterFrequency (lowPassFreq);
            wetLowPass->setQfactor (lowPassQ);
        }

        const auto highPassFreq = parameters.eqHiPassFreq->get();
        const auto highPassQ    = parameters.eqHiPassQ->get();

        if (auto* dryHighPass = dryEQ.getBandOfType (FT::HighPass))
        {
            dryHighPass->setFilterFrequency (highPassFreq);
            dryHighPass->setQfactor (highPassQ);
        }
        if (auto* wetHighPass = wetEQ.getBandOfType (FT::HighPass))
        {
            wetHighPass->setFilterFrequency (highPassFreq);
            wetHighPass->setQfactor (highPassQ);
        }

        const auto peakFreq = parameters.eqPeakFreq->get();
        const auto peakQ    = parameters.eqPeakQ->get();
        const auto peakGain = parameters.eqPeakGain->get();

        if (auto* dryPeak = dryEQ.getBandOfType (FT::Peak))
        {
            dryPeak->setFilterFrequency (peakFreq);
            dryPeak->setQfactor (peakQ);
            dryPeak->setGain (peakGain);
        }
        if (auto* wetPeak = wetEQ.getBandOfType (FT::Peak))
        {
            wetPeak->setFilterFrequency (peakFreq);
            wetPeak->setQfactor (peakQ);
            wetPeak->setGain (peakGain);
        }

        dryEQ.process (dry);
        wetEQ.process (wet);
    }
}

template < typename SampleType >
void EQ< SampleType >::prepare (double samplerate, int blocksize)
{
    dryEQ.prepare (samplerate, blocksize);
    wetEQ.prepare (samplerate, blocksize);
}

template struct EQ< float >;
template struct EQ< double >;

}  // namespace Imogen
