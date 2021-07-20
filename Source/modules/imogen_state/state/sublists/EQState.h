#pragma once

namespace Imogen
{
using namespace ParameterValueConversionLambdas;

struct EQState
{
    EQState (plugin::ParameterList& list);

    ToggleParam eqToggle {"EQ Toggle", "EQ toggle", false};

    HzParam    eqLowShelfFreq {"EQ low shelf freq", "EQ low shelf freq", 80.f};
    FloatParam eqLowShelfQ {0.01f, 10.f, 0.707f, "EQ low shelf Q", "EQ low shelf Q"};
    FloatParam eqLowShelfGain {0.f, 4.f, 1.f, "EQ low shelf gain", "EQ low shelf gain"};

    HzParam    eqHighShelfFreq {"EQ high shelf freq", "EQ high shelf freq", 80.f};
    FloatParam eqHighShelfQ {0.01f, 10.f, 0.707f, "EQ high shelf Q", "EQ high shelf Q"};
    FloatParam eqHighShelfGain {0.f, 4.f, 1.f, "EQ high shelf gain", "EQ high shelf gain"};

    HzParam    eqHighPassFreq {"EQ high pass freq", "EQ high pass freq", 80.f};
    FloatParam eqHighPassQ {0.01f, 10.f, 0.707f, "EQ high pass Q", "EQ high pass Q"};

    HzParam    eqPeakFreq {"EQ peak freq", "EQ peak freq", 80.f};
    FloatParam eqPeakQ {0.01f, 10.f, 0.707f, "EQ peak Q", "EQ peak Q"};
    FloatParam eqPeakGain {0.f, 4.f, 1.f, "EQ peak gain", "EQ peak gain"};
};

}  // namespace Imogen
