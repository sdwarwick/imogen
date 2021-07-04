
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
    using Voice       = HarmonizerVoice< SampleType >;
    using Analyzer    = dsp::psola::Analyzer< SampleType >;

public:
    Harmonizer (State& stateToUse, Analyzer& analyzerToUse);

    void process (int numSamples,
                  MidiBuffer&        midiMessages,
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
    MidiState&  midi {parameters.midiState};
    Internals&  internals {state.internals};

    AudioBuffer wetBuffer;
    AudioBuffer alias;
    
    int lastBlocksize {0};
};


}  // namespace Imogen
