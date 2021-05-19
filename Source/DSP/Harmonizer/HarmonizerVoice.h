/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 bv_HarmonizerVoice.h: This file defines the interface for the HarmonizerVoice class. The Harmonizer owns and manages a collection of HarmonizerVoices; a single Voice plays a single repitched note at a time.
 
======================================================================================================================================================*/


namespace bav
{
template < typename SampleType >
class Harmonizer; // forward declaration...


template < typename SampleType >
class HarmonizerVoice : public dsp::SynthVoiceBase< SampleType >
{
    using AudioBuffer     = juce::AudioBuffer< SampleType >;
    using Synthesis_Grain = SynthesisGrain< SampleType >;


public:
    HarmonizerVoice (Harmonizer< SampleType >* h);


private:
    friend class Harmonizer< SampleType >;

    void newBlockComing (int previousBlocksize, int upcomingBlocksize) override;

    void renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate, int startSampleOfOrigBuffer) override;

    void bypassedBlockRecieved (float voicesLastOutputFreq, double currentSamplerate, int numSamples) override;

    void prepared (const int blocksize) override;

    void released() override;

    void noteCleared() override;

    /**/

    Harmonizer< SampleType >* parent;

    PsolaShifter< SampleType > shifter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


} // namespace bav
