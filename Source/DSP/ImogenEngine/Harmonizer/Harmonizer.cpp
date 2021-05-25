
namespace bav
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer()
{
    Base::setConcertPitchHz (440);

    Base::updateQuickReleaseMs (adsrQuickReleaseMs);

    Base::playingButReleasedMultiplier = playingButReleasedGainMultiplier;
    Base::softPedalMultiplier          = softPedalGainMultiplier;
}


template < typename SampleType >
void Harmonizer< SampleType >::initialized (const double, const int)
{
    analyzer.initialize();
}


template < typename SampleType >
void Harmonizer< SampleType >::prepared (int)
{

}


template < typename SampleType >
void Harmonizer< SampleType >::resetTriggered()
{
    analyzer.reset();
}


template < typename SampleType >
void Harmonizer< SampleType >::samplerateChanged (double newSamplerate)
{
    analyzer.setSamplerate (newSamplerate);
}


template < typename SampleType >
void Harmonizer< SampleType >::release()
{
    analyzer.releaseResources();
}


template < typename SampleType >
void Harmonizer< SampleType >::render (const AudioBuffer& input, AudioBuffer& output, juce::MidiBuffer& midiMessages)
{
    jassert (input.getNumSamples() == output.getNumSamples());
    jassert (output.getNumChannels() == 2);

    if (Base::getNumActiveVoices() == 0)
        analyzer.reset();
    else
        analyzer.clearUnusedGrains();

    analyzer.analyzeInput(input.getReadPointer(0), input.getNumSamples());
    
    Base::renderVoices (midiMessages, output);
}


template < typename SampleType >
void Harmonizer< SampleType >::addNumVoices (const int voicesToAdd)
{
    if (voicesToAdd == 0) return;

    for (int i = 0; i < voicesToAdd; ++i)
        Base::voices.add (new Voice (*this));

    jassert (Base::voices.size() >= voicesToAdd);

    Base::numVoicesChanged();
}


template class Harmonizer< float >;
template class Harmonizer< double >;


} // namespace bav
