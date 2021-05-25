
#pragma once


namespace Imogen
{


namespace l = bav::ParameterValueConversionLambdas;

struct Parameters :     bav::ParameterList
{
    using IntParam = bav::IntParam;
    using FloatParam = bav::FloatParam;
    using BoolParam = bav::BoolParam;
    
    Parameters()
        : ParameterList ("ImogenParameters")
    {
        add (inputMode, dryWet, inputGain, outputGain, mainBypass, leadBypass, harmonyBypass, stereoWidth, lowestPanned, leadPan, pitchbendRange, velocitySens, aftertouchToggle, voiceStealing, pedalToggle, pedalThresh, descantToggle, descantThresh, descantInterval, adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateToggle, noiseGateThresh, deEsserToggle, deEsserThresh, deEsserAmount, compToggle, compAmount, delayToggle, delayDryWet, reverbToggle, reverbDryWet, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut, limiterToggle, midiLatch);
        
        addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, editorPitchbend, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentCentsSharp, editorSizeX, editorSizeY);
    }
    
    IntParam inputMode { "Input source", "Input source", 1, 3, 1, juce::String(),
        [] (int value, int maxLength)
        {
            switch (value)
            {
                case (2): return TRANS ("Right").substring (0, maxLength);
                case (3): return TRANS ("Mix to mono").substring (0, maxLength);
                default: return TRANS ("Left").substring (0, maxLength);
            }
        },
        [] (const juce::String& text)
        {
            if (text.containsIgnoreCase (TRANS ("Right"))) return 2;
            if (text.containsIgnoreCase (TRANS ("mono")) || text.containsIgnoreCase (TRANS ("mix"))) return 3;
            return 1;
        } };
    
    IntParam dryWet { "Dry/wet", "Main dry/wet", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    FloatParam inputGain { "In", "Input gain", gainRange, 0.0f, dB,
        juce::AudioProcessorParameter::inputGain,
        l::gain_stringFromFloat,
        l::gain_floatFromString };
    
    FloatParam outputGain { "Out", "Output gain", gainRange, -4.0f, dB,
        juce::AudioProcessorParameter::outputGain,
        l::gain_stringFromFloat,
        l::gain_floatFromString };
    
    BoolParam mainBypass { "Main", "Main bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam leadBypass { "Lead", "Lead bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam harmonyBypass { "Harmony", "Harmony bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam stereoWidth { "Width", "Stereo width", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    IntParam lowestPanned { "Lowest note", "Lowest panned note", 0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString };
    
    IntParam leadPan { "Lead pan", "Lead pan", 0, 127, 64, juce::String(), l::midiPan_stringFromInt, l::midiPan_intFromString };
    
    IntParam pitchbendRange { "Pitchbend", "Pitchbend range", 0, 12, 2, st, l::st_stringFromInt, l::st_intFromString };
    
    IntParam velocitySens { "Velocity", "Velocity amount", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    BoolParam aftertouchToggle { "Aftertouch", "Aftertouch gain", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam voiceStealing { "Stealing", "Voice stealing", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam pedalToggle { "Toggle", "Pedal toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam pedalThresh { "Thresh", "Pedal thresh", 0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString };
    
    IntParam pedalInterval { "Interval", "Pedal interval", 1, 12, 12, st, l::st_stringFromInt, l::st_intFromString };
    
    BoolParam descantToggle { "Toggle", "Descant toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam descantThresh { "Thresh", "Descant thresh", 0, 127, 127, juce::String(), l::pcnt_stringFromInt, l::pitch_intFromString };
    
    IntParam descantInterval { "Interval", "Descant interval", 1, 12, 12, st, l::st_stringFromInt, l::st_intFromString };
    
    FloatParam adsrAttack { "Attack", "ADSR attack", msRange, 0.35f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    FloatParam adsrDecay { "Decay", "ADSR decay", msRange, 0.06f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    FloatParam adsrSustain { "Sustain", "ADSR sustain", zeroToOneRange, 0.8f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam adsrRelease { "Release", "ADSR release", msRange, 0.1f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    BoolParam noiseGateToggle { "Toggle", "Gate toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam noiseGateThresh { "Thresh", "Gate thresh", gainRange, -20.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString };
    
    BoolParam deEsserToggle { "Toggle", "D-S toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam deEsserThresh { "Thresh", "D-S thresh", gainRange, -6.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString };
    
    FloatParam deEsserAmount { "Amount", "D-S amount", zeroToOneRange, 0.5f, dB, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    BoolParam compToggle { "Toggle", "Compressor toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam compAmount { "Amount", "Compressor amount", zeroToOneRange, 0.35f, dB, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    BoolParam delayToggle { "Toggle", "Delay toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam delayDryWet { "Mix", "Delay mix", 0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    BoolParam reverbToggle { "Toggle", "Reverb toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam reverbDryWet { "Mix", "Reverb mix", 0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    FloatParam reverbDecay { "Decay", "Reverb decay", zeroToOneRange, 0.6f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam reverbDuck { "Duck", "Reverb duck", zeroToOneRange, 0.3f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam reverbLoCut { "Lo cut", "Reverb lo cut", hzRange, 80.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString };
    
    FloatParam reverbHiCut { "Hi cut", "Reverb hi cut", hzRange, 5500.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString };
    
    BoolParam limiterToggle { "Toggle", "Limiter toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam midiLatch { "Is latched", "MIDI is latched", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    /* */
    
    BoolParam abletonLinkEnabled { "Toggle", "Ableton link toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam abletonLinkSessionPeers { "Num peers", "Ableton link num session peers", 0, 50, 0, juce::String(),
        [] (int value, int maximumStringLength) { return juce::String (value).substring (0, maximumStringLength); },
        nullptr };
    
    BoolParam mtsEspIsConnected { "Is connected", "MTS-ESP is connected", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam editorPitchbend { "Pitchbend", "GUI pitchbend", 0, 127, 64, juce::String(),
        [] (int value, int maximumStringLength) { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text) { return text.retainCharacters ("1234567890").getIntValue(); } };
    
    IntParam lastMovedMidiController { "Number", "Last moved MIDI controller number", 0, 127, 0 };
    
    IntParam lastMovedCCValue { "Value", "Last moved MIDI controller value", 0, 127, 0 };
    
    BoolParam guiDarkMode { "Dark mode", "GUI Dark mode", true, juce::String(),
        [] (bool val, int maxLength)
        {
            if (val) return TRANS ("Dark mode is on").substring (0, maxLength);
            
            return TRANS ("Dark mode is off").substring (0, maxLength);
        },
        nullptr };
    
    IntParam currentCentsSharp { "Cents sharp", "Current input cents sharp", -100, 100, 0, juce::String(),
        [] (int cents, int maxLength)
        {
            if (cents == 0) return TRANS ("Perfect!");
            
            if (cents > 0) return (juce::String (cents) + TRANS (" cents sharp")).substring (0, maxLength);
            
            return (juce::String (abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
        },
        nullptr };
    
    IntParam editorSizeX { "editorSizeX", "editor size X", 0, 10000, 900 };
    IntParam editorSizeY { "editorSizeY", "editor size Y", 0, 10000, 400 };
    
private:
    const juce::NormalisableRange< float > gainRange {-60.0f, 0.0f, 0.01f};
    const juce::NormalisableRange< float > zeroToOneRange {0.0f, 1.0f, 0.01f};
    const juce::NormalisableRange< float > msRange {0.001f, 1.0f, 0.001f};
    const juce::NormalisableRange< float > hzRange {40.0f, 10000.0f, 1.0f};
    
    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
    
    // labels for parameter units
    const juce::String dB { TRANS ("dB") };
    const juce::String st { TRANS ("st") };
    const juce::String sec { TRANS ("sec") };
};

    // auto scaleName = std::make_unique< StringNode > ("Scale name", "MTS-ESP scale name", "No active scale");
    // auto currentNote = std::make_unique< StringNode > ("Current note", "Current input note as string", "-");
    

struct Meters :     bav::ParameterList
{
    using Gain = bav::ParameterHolder< bav::GainMeterParameter >;
    using Param = juce::AudioProcessorParameter;
    
    Meters()
        : ParameterList ("ImogenMeters")
    {
        add (inputLevel, outputLevelL, outputLevelR, gateRedux, compRedux, deEssRedux, limRedux, reverbLevel, delayLevel);
    }
    
    Gain inputLevel { "In", "Input level", Param::inputMeter };
    
    Gain outputLevelL { "OutL", "Output level (L)", Param::outputMeter };
    Gain outputLevelR { "OutR", "Output level (R)", Param::outputMeter };
    
    Gain gateRedux { "Gate redux", "Noise gate gain reduction", compLimMeter };
    Gain compRedux { "Comp redux", "Compressor gain reduction", compLimMeter };
    Gain deEssRedux { "D-S redux", "De-esser gain reduction", compLimMeter };
    Gain limRedux { "Lim redux", "Limiter gain reduction", compLimMeter };
    
    Gain reverbLevel { "Reverb", "Reverb level", otherMeter };
    Gain delayLevel { "Delay", "Delay level", otherMeter };
    
private:
    static constexpr auto compLimMeter = Param::compressorLimiterGainReductionMeter;
    static constexpr auto otherMeter   = Param::otherMeter;
};


} // namespace Imogen
