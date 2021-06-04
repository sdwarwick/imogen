
#pragma once


namespace Imogen
{
namespace l = bav::ParameterValueConversionLambdas;

struct Parameters : bav::ParameterList
{
    Parameters() : ParameterList ("ImogenParameters")
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

    PercentParam dryWet {"Main dry/wet", "Main dry/wet", 100};

    GainParam inputGain {"Input gain", "Input gain", 0.0f, juce::AudioProcessorParameter::inputGain};

    GainParam outputGain {"Output gain", "Output gain", -4.0f, juce::AudioProcessorParameter::outputGain};

    ToggleParam mainBypass {"Main bypass", "Main bypass", false};

    ToggleParam leadBypass {"Lead bypass", "Lead bypass", false};

    ToggleParam harmonyBypass {"Harmony bypass", "Harmony bypass", false};

    PercentParam stereoWidth {"Stereo width", "Stereo width", 100};

    IntParam lowestPanned {"Lowest panned note", "Lowest panned note", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam leadPan {"Lead pan", "Lead pan", 0, 127, 64, l::midiPan_stringFromInt, l::midiPan_intFromString};

    IntParam pitchbendRange {"Pitchbend range", "Pitchbend range", 0, 12, 2, l::st_stringFromInt, l::st_intFromString, TRANS ("st")};

    PercentParam velocitySens {"Velocity amount", "Velocity amount", 100};

    ToggleParam aftertouchToggle {"Aftertouch gain", "Aftertouch gain", true};

    ToggleParam voiceStealing {"Voice stealing", "Voice stealing", false};

    ToggleParam pedalToggle {" Pedal toggle", "Pedal toggle", false};

    IntParam pedalThresh {"Pedal thresh", "Pedal thresh", 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString};

    IntParam pedalInterval {"Pedal interval", "Pedal interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString};

    ToggleParam descantToggle {"Descant toggle", "Descant toggle", false};

    IntParam descantThresh {"Descant thresh", "Descant thresh", 0, 127, 127, l::pcnt_stringFromInt, l::pitch_intFromString};

    IntParam descantInterval {"Descant interval", "Descant interval", 1, 12, 12, l::st_stringFromInt, l::st_intFromString};

    FloatParam adsrAttack {"ADSR Attack", "ADSR attack",
                           juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
                           0.35f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};

    FloatParam adsrDecay {"ADSR Decay", "ADSR decay",
                          juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
                          0.06f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};

    PercentParam adsrSustain {"ADSR Sustain", "ADSR sustain", 80};

    FloatParam adsrRelease {"ADSR Release", "ADSR release",
                            juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
                            0.1f, generic, l::sec_stringFromFloat, l::sec_floatFromString, TRANS ("sec")};

    ToggleParam noiseGateToggle {"Gate Toggle", "Gate toggle", true};

    GainParam noiseGateThresh {"Gate Thresh", "Gate thresh", -20.0f, generic};

    ToggleParam deEsserToggle {"D-S Toggle", "D-S toggle", true};

    GainParam deEsserThresh {"D-S Thresh", "D-S thresh", -6.0f, generic};

    PercentParam deEsserAmount {"D-S Amount", "D-S amount", 50};

    ToggleParam compToggle {"Compressor Toggle", "Compressor toggle", false};

    PercentParam compAmount {"Compressor Amount", "Compressor amount", 50};

    ToggleParam delayToggle {"Delay Toggle", "Delay toggle", false};

    PercentParam delayDryWet {"Delay Mix", "Delay mix", 0};

    ToggleParam reverbToggle {"Reverb Toggle", "Reverb toggle", false};

    PercentParam reverbDryWet {"Reverb Mix", "Reverb mix", 15};

    PercentParam reverbDecay {"Reverb Decay", "Reverb decay", 60};

    PercentParam reverbDuck {"Reverb Duck", "Reverb duck", 30};

    FloatParam reverbLoCut {"Reverb Lo cut", "Reverb lo cut",
                            juce::NormalisableRange< float > (40.0f, 10000.0f, 1.0f),
                            80.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    FloatParam reverbHiCut {"Reverb Hi cut", "Reverb hi cut",
                            juce::NormalisableRange< float > (40.0f, 10000.0f, 1.0f),
                            5500.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString, TRANS ("Hz")};

    ToggleParam limiterToggle {"Limiter Toggle", "Limiter toggle", true};

    ToggleParam midiLatch {"MIDI latch", "MIDI latch", false};

    /* */

    IntParam editorPitchbend {"GUI Pitchbend", "GUI Pitchbend", 0, 127, 64,
                              [] (int value, int maximumStringLength)
                              { return juce::String (value).substring (0, maximumStringLength); },
                              [] (const juce::String& text)
                              { return text.retainCharacters ("1234567890").getIntValue(); }};

private:
    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
};


}  // namespace Imogen
