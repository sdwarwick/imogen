
#pragma once


namespace Imogen
{
namespace l = bav::ParameterValueConversionLambdas;

struct Parameters : bav::ParameterList
{
    using IntParam     = bav::IntParam;
    using FloatParam   = bav::FloatParam;
    using BoolParam    = bav::BoolParam;
    using GainParam    = bav::GainParam;
    using ToggleParam  = bav::ToggleParam;
    using PercentParam = bav::PercentParam;

    Parameters(): ParameterList ("ImogenParameters")
    {
        add (inputMode, dryWet, inputGain, outputGain, mainBypass, leadBypass, harmonyBypass, stereoWidth, lowestPanned, leadPan, pitchbendRange, velocitySens, aftertouchToggle, voiceStealing, pedalToggle, pedalThresh, descantToggle, descantThresh, descantInterval, adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateToggle, noiseGateThresh, deEsserToggle, deEsserThresh, deEsserAmount, compToggle, compAmount, delayToggle, delayDryWet, reverbToggle, reverbDryWet, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut, limiterToggle, midiLatch);
        
        addInternal (editorPitchbend);
    }

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

    PercentParam dryWet {"Dry/wet", "Main dry/wet", 100};

    GainParam inputGain { "In", "Input gain", 0.0f, juce::AudioProcessorParameter::inputGain };
    
    GainParam outputGain { "Out", "Output gain", -4.0f, juce::AudioProcessorParameter::outputGain };
    
    ToggleParam mainBypass { "Main", "Main bypass", false };

    ToggleParam leadBypass {"Lead", "Lead bypass", false};

    ToggleParam harmonyBypass {"Harmony", "Harmony bypass", false};

    PercentParam stereoWidth {"Width", "Stereo width", 100};

    IntParam lowestPanned {"Lowest note", "Lowest panned note", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam leadPan {"Lead pan", "Lead pan", 0, 127, 64, l::midiPan_stringFromInt, l::midiPan_intFromString};

    IntParam pitchbendRange {"Pitchbend", "Pitchbend range", 0, 12, 2, l::st_stringFromInt, l::st_intFromString, TRANS ("st")};

    PercentParam velocitySens {"Velocity", "Velocity amount", 100};

    ToggleParam aftertouchToggle {"Aftertouch", "Aftertouch gain", true};

    ToggleParam voiceStealing {"Stealing", "Voice stealing", false};

    ToggleParam pedalToggle {"Toggle", "Pedal toggle", false};

    IntParam pedalThresh {"Thresh", "Pedal thresh", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam pedalInterval {"Interval", "Pedal interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString};

    ToggleParam descantToggle {"Toggle", "Descant toggle", false};

    IntParam descantThresh {"Thresh", "Descant thresh", 0, 127, 127, l::pcnt_stringFromInt, l::pitch_intFromString};

    IntParam descantInterval {"Interval", "Descant interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString};

    FloatParam adsrAttack {"Attack", "ADSR attack",
        juce::NormalisableRange< float >(0.0f, 1.0f, 0.01f),
        0.35f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};

    FloatParam adsrDecay {"Decay", "ADSR decay",
        juce::NormalisableRange< float >(0.0f, 1.0f, 0.01f),
        0.06f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};
    
    PercentParam adsrSustain {"Sustain", "ADSR sustain", 80};

    FloatParam adsrRelease {"Release", "ADSR release",
        juce::NormalisableRange< float >(0.0f, 1.0f, 0.01f),
        0.1f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};

    ToggleParam noiseGateToggle {"Toggle", "Gate toggle", true};

    GainParam noiseGateThresh {"Thresh", "Gate thresh", -20.0f, generic};

    ToggleParam deEsserToggle {"Toggle", "D-S toggle", true};

    GainParam deEsserThresh {"Thresh", "D-S thresh", -6.0f, generic};

    PercentParam deEsserAmount {"Amount", "D-S amount", 50};

    ToggleParam compToggle {"Toggle", "Compressor toggle", false};

    PercentParam compAmount {"Amount", "Compressor amount", 50};

    ToggleParam delayToggle {"Toggle", "Delay toggle", false};

    PercentParam delayDryWet {"Mix", "Delay mix", 0};

    ToggleParam reverbToggle {"Toggle", "Reverb toggle", false};

    PercentParam reverbDryWet {"Mix", "Reverb mix", 15};

    PercentParam reverbDecay {"Decay", "Reverb decay", 60};

    PercentParam reverbDuck {"Duck", "Reverb duck", 30};

    FloatParam reverbLoCut {"Lo cut", "Reverb lo cut",
        juce::NormalisableRange< float >(40.0f, 10000.0f, 1.0f),
        80.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    FloatParam reverbHiCut {"Hi cut", "Reverb hi cut",
        juce::NormalisableRange< float >(40.0f, 10000.0f, 1.0f),
        5500.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    ToggleParam limiterToggle {"Toggle", "Limiter toggle", true};

    ToggleParam midiLatch {"Is latched", "MIDI is latched", false};

    /* */
    
    IntParam editorPitchbend {"Pitchbend", "GUI pitchbend", 0, 127, 64,
        [] (int value, int maximumStringLength)
        { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text)
        { return text.retainCharacters ("1234567890").getIntValue(); }};
    
private:
    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
};


}  // namespace Imogen
