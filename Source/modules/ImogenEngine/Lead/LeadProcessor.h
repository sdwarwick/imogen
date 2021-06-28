
#pragma once

#include <ImogenEngine/Harmonizer/Harmonizer.h>

#include "DryPanner.h"
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
    using Harmonizer  = Harmonizer< SampleType >;

    LeadProcessor (Harmonizer& harm, State& stateToUse);

    void prepare (double samplerate, int blocksize);

    void process (bool leadIsBypassed);

    AudioBuffer& getProcessedSignal();

private:
    PitchCorrection< SampleType > pitchCorrector;
    DryPanner< SampleType >       dryPanner;

    AudioBuffer pannedLeadBuffer;
};

}  // namespace Imogen
