
#pragma once

#include "PitchCorrector.h"
#include "DryPanner.h"

namespace Imogen
{
template < typename SampleType >
class LeadProcessor
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using Analyzer    = dsp::psola::Analyzer< SampleType >;
    using Synth       = dsp::SynthBase< SampleType >;

    LeadProcessor (Harmonizer< SampleType >& harm, State& stateToUse);

    void prepare (double samplerate, int blocksize);

    void process (bool leadIsBypassed);

    AudioBuffer& getProcessedSignal();

private:
    PitchCorrection< SampleType > pitchCorrector;
    DryPanner< SampleType >       dryPanner;

    AudioBuffer pannedLeadBuffer;
};

}  // namespace Imogen
