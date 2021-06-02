
#pragma once


namespace bav
{
template < typename SampleType >
class Harmonizer; // forward declaration...


template < typename SampleType >
class HarmonizerVoice : public dsp::SynthVoiceBase< SampleType >
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using Harm = Harmonizer< SampleType >;

public:
    HarmonizerVoice (Harm& h);


private:
    void newBlockComing (int previousBlocksize, int upcomingBlocksize) override;

    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int startSampleOfOrigBuffer) override;

    void bypassedBlockRecieved (float voicesLastOutputFreq, double currentSamplerate, int numSamples) override;

    void prepared (const int blocksize) override;

    void released() override;

    void noteCleared() override;

    /**/

    Harm& parent;

    dsp::PsolaShifter< SampleType > shifter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


} // namespace bav
