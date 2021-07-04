#pragma once

#include <imogen_state/imogen_state.h>

#include "Lead/LeadProcessor.h"
#include "effects/PostHarmonyEffects.h"
#include "effects/PreHarmonyEffects.h"

namespace Imogen
{
template < typename SampleType >
class Engine : public dsp::LatencyEngine< SampleType >
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    Engine (State& stateToUse);

private:
    void renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool isBypassed) final;

    void onPrepare (int blocksize, double samplerate) final;

    void updateStereoWidth (int width);

    State&      state;
    Parameters& parameters {state.parameters};

    dsp::psola::Analyzer< SampleType > analyzer;
    
    PreHarmonyEffects<SampleType> preHarmonyEffects {state};

    Harmonizer< SampleType > harmonizer {state, analyzer};

    LeadProcessor< SampleType > leadProcessor {harmonizer, state};

    PostHarmonyEffects< SampleType > postHarmonyEffects {state};
};

}  // namespace Imogen
