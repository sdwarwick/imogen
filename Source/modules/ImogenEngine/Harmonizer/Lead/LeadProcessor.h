
#pragma once

#include <ImogenEngine/effects/DryPanner.h>
#include "PitchCorrector.h"

namespace Imogen
{
template < typename SampleType >
class LeadProcessor
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using Analyzer    = dsp::psola::Analyzer< SampleType >;
    using Synth       = dsp::SynthBase< SampleType >;

    LeadProcessor (Synth& harm, State& stateToUse, Analyzer& analyzerToUse);

    void prepare (double samplerate, int blocksize);

    void process (bool leadIsBypassed);

    AudioBuffer& getProcessedSignal();

private:
    State&      state;
    Parameters& parameters {state.parameters};

    PitchCorrection< SampleType > pitchCorrector;
    DryPanner< SampleType >       dryPanner {parameters};

    AudioBuffer pannedLeadBuffer;
};

}  // namespace Imogen
