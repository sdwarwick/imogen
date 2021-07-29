
#pragma once

#include "sublists/EQState.h"
#include "sublists/ReverbState.h"
#include "sublists/MidiState.h"

namespace Imogen
{
struct Parameters : plugin::ParameterList
{
    Parameters();

    IntParam inputMode {1, 3, 1, "Input source",
                        [] (int value, int maxLength)
                        {
                            switch (value)
                            {
                                case (2) : return TRANS ("Right").substring (0, maxLength);
                                case (3) : return TRANS ("Mix to mono").substring (0, maxLength);
                                default : return TRANS ("Left").substring (0, maxLength);
                            }
                        },
                        [] (const juce::String& text)
                        {
                            if (text.containsIgnoreCase (TRANS ("Right"))) return 2;
                            if (text.containsIgnoreCase (TRANS ("mono")) || text.containsIgnoreCase (TRANS ("mix"))) return 3;
                            return 1;
                        }};

    PercentParam dryWet {"Main dry/wet", 100};

    dbParam inputGain {"Input gain", 0.0f, juce::AudioProcessorParameter::inputGain};

    dbParam outputGain {"Output gain", -4.0f, juce::AudioProcessorParameter::outputGain};

    ToggleParam leadBypass {"Lead bypass", false};

    ToggleParam harmonyBypass {"Harmony bypass", false};

    PercentParam stereoWidth {"Stereo width", 100};

    IntParam lowestPanned {0, 127, 0, "Lowest panned note", pitch_stringFromInt, pitch_intFromString};

    IntParam leadPan {0, 127, 64, "Lead pan", midiPan_stringFromInt, midiPan_intFromString};

    ToggleParam noiseGateToggle {"Gate toggle", true};
    dbParam     noiseGateThresh {"Gate thresh", -20.0f};

    ToggleParam  deEsserToggle {"D-S toggle", true};
    dbParam      deEsserThresh {"D-S thresh", -6.0f};
    PercentParam deEsserAmount {"D-S amount", 50};

    ToggleParam  compToggle {"Compressor toggle", false};
    PercentParam compAmount {"Compressor amount", 50};

    ToggleParam  delayToggle {"Delay toggle", false};
    PercentParam delayDryWet {"Delay mix", 0};

    ToggleParam limiterToggle {"Limiter toggle", true};

    EQState eqState {*this};

    ReverbState reverbState {*this};

    MidiState midiState {*this};
};


}  // namespace Imogen
