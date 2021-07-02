
#pragma once

#include "sublists/EQState.h"
#include "sublists/ReverbState.h"
#include "sublists/MidiState.h"

namespace Imogen
{

struct Parameters : ParameterList
{
    Parameters();

    IntParam inputMode {"Input source", "Input source", 1, 3, 1,
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

    PercentParam dryWet {"Main dry/wet", "Main dry/wet", 100};

    dbParam inputGain {"Input gain", "Input gain", 0.0f, juce::AudioProcessorParameter::inputGain};

    dbParam outputGain {"Output gain", "Output gain", -4.0f, juce::AudioProcessorParameter::outputGain};

    ToggleParam leadBypass {"Lead bypass", "Lead bypass", false};

    ToggleParam harmonyBypass {"Harmony bypass", "Harmony bypass", false};

    PercentParam stereoWidth {"Stereo width", "Stereo width", 100};

    IntParam lowestPanned {"Lowest panned note", "Lowest panned note", 0, 127, 0, pitch_stringFromInt, pitch_intFromString};

    IntParam leadPan {"Lead pan", "Lead pan", 0, 127, 64, midiPan_stringFromInt, midiPan_intFromString};

    ToggleParam noiseGateToggle {"Gate Toggle", "Gate toggle", true};
    dbParam     noiseGateThresh {"Gate Thresh", "Gate thresh", -20.0f, generic};

    ToggleParam  deEsserToggle {"D-S Toggle", "D-S toggle", true};
    dbParam      deEsserThresh {"D-S Thresh", "D-S thresh", -6.0f, generic};
    PercentParam deEsserAmount {"D-S Amount", "D-S amount", 50};

    ToggleParam  compToggle {"Compressor Toggle", "Compressor toggle", false};
    PercentParam compAmount {"Compressor Amount", "Compressor amount", 50};

    ToggleParam  delayToggle {"Delay Toggle", "Delay toggle", false};
    PercentParam delayDryWet {"Delay Mix", "Delay mix", 0};

    ToggleParam limiterToggle {"Limiter Toggle", "Limiter toggle", true};
    
    EQState eqState {*this};
    
    ReverbState reverbState {*this};
    
    MidiState midiState {*this};

private:
    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
};


}  // namespace Imogen
