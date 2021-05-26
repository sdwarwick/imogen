
#pragma once


namespace Imogen
{
namespace l = bav::ParameterValueConversionLambdas;

struct Parameters : bav::ParameterList
{
    using IntParam    = bav::IntParam;
    using FloatParam  = bav::FloatParam;
    using BoolParam   = bav::BoolParam;
    using GainParam   = bav::GainParam;
    using ToggleParam = bav::ToggleParam;

    Parameters()
        : ParameterList ("ImogenParameters")
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

    IntParam dryWet {"Dry/wet", "Main dry/wet", 0, 100, 100, l::pcnt_stringFromInt, l::pcnt_intFromString, "%"};

    GainParam inputGain { "In", "Input gain", 0.0f, juce::AudioProcessorParameter::inputGain };
    
    GainParam outputGain { "Out", "Output gain", -4.0f, juce::AudioProcessorParameter::outputGain };
    
    ToggleParam mainBypass { "Main", "Main bypass", false };

    ToggleParam leadBypass {"Lead", "Lead bypass", false};

    ToggleParam harmonyBypass {"Harmony", "Harmony bypass", false};

    IntParam stereoWidth {"Width", "Stereo width", 0, 100, 100, l::pcnt_stringFromInt, l::pcnt_intFromString, "%"};

    IntParam lowestPanned {"Lowest note", "Lowest panned note", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam leadPan {"Lead pan", "Lead pan", 0, 127, 64, l::midiPan_stringFromInt, l::midiPan_intFromString};

    IntParam pitchbendRange {"Pitchbend", "Pitchbend range", 0, 12, 2, l::st_stringFromInt, l::st_intFromString, st};

    IntParam velocitySens {"Velocity", "Velocity amount", 0, 100, 100, l::pcnt_stringFromInt, l::pcnt_intFromString, "%"};

    ToggleParam aftertouchToggle {"Aftertouch", "Aftertouch gain", true};

    ToggleParam voiceStealing {"Stealing", "Voice stealing", false};

    ToggleParam pedalToggle {"Toggle", "Pedal toggle", false};

    IntParam pedalThresh {"Thresh", "Pedal thresh", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam pedalInterval {"Interval", "Pedal interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString, st};

    ToggleParam descantToggle {"Toggle", "Descant toggle", false};

    IntParam descantThresh {"Thresh", "Descant thresh", 0, 127, 127, l::pcnt_stringFromInt, l::pitch_intFromString};

    IntParam descantInterval {"Interval", "Descant interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString, st};

    FloatParam adsrAttack {"Attack", "ADSR attack", msRange, 0.35f, generic, l::sec_stringFromFloat, l::sec_floatFromString, sec};

    FloatParam adsrDecay {"Decay", "ADSR decay", msRange, 0.06f, generic, l::sec_stringFromFloat, l::sec_floatFromString, sec};

    FloatParam adsrSustain {"Sustain", "ADSR sustain", zeroToOneRange, 0.8f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString, "%"};

    FloatParam adsrRelease {"Release", "ADSR release", msRange, 0.1f, generic, l::sec_stringFromFloat, l::sec_floatFromString, sec};

    ToggleParam noiseGateToggle {"Toggle", "Gate toggle", true};

    FloatParam noiseGateThresh {"Thresh", "Gate thresh", gainRange, -20.0f, generic, l::gain_stringFromFloat, l::gain_floatFromString, dB};

    ToggleParam deEsserToggle {"Toggle", "D-S toggle", true};

    FloatParam deEsserThresh {"Thresh", "D-S thresh", gainRange, -6.0f, generic, l::gain_stringFromFloat, l::gain_floatFromString, dB};

    FloatParam deEsserAmount {"Amount", "D-S amount", zeroToOneRange, 0.5f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString, dB};

    ToggleParam compToggle {"Toggle", "Compressor toggle", false};

    FloatParam compAmount {"Amount", "Compressor amount", zeroToOneRange, 0.35f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString, dB};

    ToggleParam delayToggle {"Toggle", "Delay toggle", false};

    IntParam delayDryWet {"Mix", "Delay mix", 0, 100, 35, l::pcnt_stringFromInt, l::pcnt_intFromString, "%"};

    ToggleParam reverbToggle {"Toggle", "Reverb toggle", false};

    IntParam reverbDryWet {"Mix", "Reverb mix", 0, 100, 35, l::pcnt_stringFromInt, l::pcnt_intFromString, "%"};

    FloatParam reverbDecay {"Decay", "Reverb decay", zeroToOneRange, 0.6f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString, "%"};

    FloatParam reverbDuck {"Duck", "Reverb duck", zeroToOneRange, 0.3f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString, "%"};

    FloatParam reverbLoCut {"Lo cut", "Reverb lo cut", hzRange, 80.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    FloatParam reverbHiCut {"Hi cut", "Reverb hi cut", hzRange, 5500.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    ToggleParam limiterToggle {"Toggle", "Limiter toggle", true};

    ToggleParam midiLatch {"Is latched", "MIDI is latched", false};

    /* */
    
    IntParam editorPitchbend {"Pitchbend", "GUI pitchbend", 0, 127, 64,
        [] (int value, int maximumStringLength)
        { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text)
        { return text.retainCharacters ("1234567890").getIntValue(); }};
    
private:
    const juce::NormalisableRange< float > gainRange {-60.0f, 0.0f, 0.01f};
    const juce::NormalisableRange< float > zeroToOneRange {0.0f, 1.0f, 0.01f};
    const juce::NormalisableRange< float > msRange {0.001f, 1.0f, 0.001f};
    const juce::NormalisableRange< float > hzRange {40.0f, 10000.0f, 1.0f};

    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;

    // labels for parameter units
    const juce::String dB {TRANS ("dB")};
    const juce::String st {TRANS ("st")};
    const juce::String sec {TRANS ("sec")};
};


}  // namespace Imogen
