#pragma once

#include <imogen_state/imogen_state.h>

#include "Lead/LeadProcessor.h"
#include "effects/EffectsManager.h"


namespace Imogen
{
template < typename SampleType >
class Engine : public dsp::LatencyEngine< SampleType >
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;

    Engine (State& stateToUse) : state (stateToUse) { }

private:
    void renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool isBypassed) final;

    void onPrepare (int blocksize, double samplerate) final;

    void updateStereoWidth (int width);

    State&      state;
    Parameters& parameters {state.parameters};

    dsp::psola::Analyzer< SampleType > analyzer;

    Harmonizer< SampleType > harmonizer {state, analyzer};

    LeadProcessor< SampleType > leadProcessor {harmonizer, state};

    EffectsManager< SampleType > effects {state};
};

}  // namespace Imogen
