
#pragma once

#include <bv_synth/bv_synth.h>
#include <bv_psola/bv_psola.h>

#include "HarmonizerVoice.h"


namespace Imogen
{
template < typename SampleType >
class Harmonizer : public dsp::SynthBase< SampleType >
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;
    using Voice       = HarmonizerVoice< SampleType >;
    using Analyzer    = dsp::psola::Analyzer< SampleType >;

public:
    Harmonizer (State& stateToUse, Analyzer& analyzerToUse);

    void process (int numSamples,
                  MidiBuffer&        midi,
                  bool               harmoniesBypassed);

    AudioBuffer& getHarmonySignal();
    
    Analyzer& analyzer;
    
private:
    void prepared (double samplerate, int blocksize) final;

    void updateParameters();
    void updateInternals();

    Voice* createVoice() final;

    State&      state;
    Parameters& parameters {state.parameters};
    Internals&  internals {state.internals};

    AudioBuffer wetBuffer;
};


}  // namespace Imogen
