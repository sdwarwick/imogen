
#pragma once

#include <bv_synth/bv_synth.h>
#include <bv_psola/bv_psola.h>

#include "HarmonizerVoice.h"
#include "Lead/LeadProcessor.h"


namespace Imogen
{
template < typename SampleType >
class Harmonizer : public dsp::SynthBase< SampleType >
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;
    using Voice       = HarmonizerVoice< SampleType >;
    using Base        = dsp::SynthBase< SampleType >;

public:
    Harmonizer (State& stateToUse);

    void process (const AudioBuffer& input,
                  MidiBuffer&        midi,
                  bool               harmoniesBypassed,
                  bool               leadBypassed);

    int getLatencySamples() const;

    AudioBuffer& getHarmonySignal();
    AudioBuffer& getLeadSignal();

private:
    void prepared (double samplerate, int blocksize) final;

    void updateParameters();
    void updateInternals();

    Voice* createVoice() final;

    State&      state;
    Parameters& parameters {state.parameters};
    Internals&  internals {state.internals};

    AudioBuffer wetBuffer;

    dsp::psola::Analyzer< SampleType > analyzer;

    LeadProcessor< SampleType > leadProcessor {*this, state, analyzer};
};


}  // namespace Imogen
