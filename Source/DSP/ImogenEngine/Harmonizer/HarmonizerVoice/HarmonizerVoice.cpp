
namespace bav
{
template < typename SampleType >
HarmonizerVoice< SampleType >::HarmonizerVoice (Harm& h)
    : dsp::SynthVoiceBase< SampleType > (&h)
    , parent (h)
    , shifter (parent.analyzer)
{
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::prepared (const int)
{
    shifter.prepare();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::released()
{
    shifter.releaseResources();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::noteCleared()
{
    shifter.reset();
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::newBlockComing (int previousBlocksize, int)
{
    shifter.newBlockComing (previousBlocksize);
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int)
{
    jassert (desiredFrequency > 0 && currentSamplerate > 0);

    shifter.getSamples (output.getWritePointer (0),
                        output.getNumSamples(),
                        juce::roundToInt (currentSamplerate / desiredFrequency)); // desired period
}


template < typename SampleType >
void HarmonizerVoice< SampleType >::bypassedBlockRecieved (float, double, int numSamples)
{
    shifter.bypassedBlockRecieved (numSamples);
}


template class HarmonizerVoice< float >;
template class HarmonizerVoice< double >;


} // namespace bav
