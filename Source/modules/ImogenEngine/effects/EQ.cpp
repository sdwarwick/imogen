
namespace Imogen
{
template < typename SampleType >
EQ< SampleType >::EQ (Parameters& params)
    : parameters (params)
{
    dryEQ.addBand (FT::LowShelf, 80.f);
    dryEQ.addBand (FT::HighShelf, 10000.f);
    dryEQ.addBand (FT::HighPass, 80.f);
    dryEQ.addBand (FT::Peak, 2500.f);

    wetEQ.addBand (FT::LowShelf, 80.f);
    wetEQ.addBand (FT::HighShelf, 10000.f);
    wetEQ.addBand (FT::HighPass, 80.f);
    wetEQ.addBand (FT::Peak, 2500.f);
}

template < typename SampleType >
void EQ< SampleType >::process (AudioBuffer& dry, AudioBuffer& wet)
{
    if (! parameters.eqToggle->get())
        return;

    updateLowShelf (parameters.eqLowShelfFreq->get(), parameters.eqLowShelfQ->get(), parameters.eqLowShelfGain->get());
    updateHighShelf (parameters.eqHighShelfFreq->get(), parameters.eqHighShelfQ->get(), parameters.eqHighShelfGain->get());
    updatePeak (parameters.eqPeakFreq->get(), parameters.eqPeakQ->get(), parameters.eqPeakGain->get());
    updateHighPass (parameters.eqHighPassFreq->get(), parameters.eqHighPassQ->get());

    dryEQ.process (dry);
    wetEQ.process (wet);
}

template < typename SampleType >
void EQ< SampleType >::updateLowShelf (float freq, float Q, float gain)
{
    if (auto* dry = dryEQ.getBandOfType (FT::LowShelf))
    {
        dry->setFilterFrequency (freq);
        dry->setQfactor (Q);
        dry->setGain (gain);
    }

    if (auto* wet = wetEQ.getBandOfType (FT::LowShelf))
    {
        wet->setFilterFrequency (freq);
        wet->setQfactor (Q);
        wet->setGain (gain);
    }
}

template < typename SampleType >
void EQ< SampleType >::updateHighShelf (float freq, float Q, float gain)
{
    if (auto* dry = dryEQ.getBandOfType (FT::HighShelf))
    {
        dry->setFilterFrequency (freq);
        dry->setQfactor (Q);
        dry->setGain (gain);
    }

    if (auto* wet = wetEQ.getBandOfType (FT::HighShelf))
    {
        wet->setFilterFrequency (freq);
        wet->setQfactor (Q);
        wet->setGain (gain);
    }
}

template < typename SampleType >
void EQ< SampleType >::updatePeak (float freq, float Q, float gain)
{
    if (auto* dry = dryEQ.getBandOfType (FT::Peak))
    {
        dry->setFilterFrequency (freq);
        dry->setQfactor (Q);
        dry->setGain (gain);
    }

    if (auto* wet = wetEQ.getBandOfType (FT::Peak))
    {
        wet->setFilterFrequency (freq);
        wet->setQfactor (Q);
        wet->setGain (gain);
    }
}

template < typename SampleType >
void EQ< SampleType >::updateHighPass (float freq, float Q)
{
    if (auto* dry = dryEQ.getBandOfType (FT::HighPass))
    {
        dry->setFilterFrequency (freq);
        dry->setQfactor (Q);
    }

    if (auto* wet = wetEQ.getBandOfType (FT::HighPass))
    {
        wet->setFilterFrequency (freq);
        wet->setQfactor (Q);
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
