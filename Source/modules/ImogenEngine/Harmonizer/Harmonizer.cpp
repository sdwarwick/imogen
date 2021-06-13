
namespace Imogen
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer()
{
    Base::setConcertPitchHz (440);

    Base::updateQuickReleaseMs (adsrQuickReleaseMs);

    Base::setPlayingButReleasedMultiplier (playingButReleasedGainMultiplier);
    Base::setSoftPedalMultiplier (softPedalGainMultiplier);
}


template < typename SampleType >
void Harmonizer< SampleType >::initialized (const double, const int)
{
    //analyzer.initialize();
}


template < typename SampleType >
void Harmonizer< SampleType >::prepared (int)
{
}


template < typename SampleType >
void Harmonizer< SampleType >::resetTriggered()
{
   // analyzer.reset();
}


template < typename SampleType >
void Harmonizer< SampleType >::samplerateChanged (double newSamplerate)
{
   // analyzer.setSamplerate (newSamplerate);
}


template < typename SampleType >
void Harmonizer< SampleType >::release()
{
   // analyzer.releaseResources();
}


template < typename SampleType >
void Harmonizer< SampleType >::render (const AudioBuffer& input, AudioBuffer& output, juce::MidiBuffer& midiMessages)
{
    jassert (input.getNumSamples() == output.getNumSamples());
    jassert (output.getNumChannels() == 2);

//    if (Base::getNumActiveVoices() == 0)
//        analyzer.reset();
//    else
//        analyzer.clearUnusedGrains();
//
//    analyzer.analyzeInput (input.getReadPointer (0), input.getNumSamples());

    Base::renderVoices (midiMessages, output);
    
    //TODO: update intonation info...
  //  intonationInfo.pitch = juce::roundToInt (math::freqToMidi (analyzer.getLastFrequency()));
    //intonationInfo.centsSharp = ...
}


template class Harmonizer< float >;
template class Harmonizer< double >;


}  // namespace bav
