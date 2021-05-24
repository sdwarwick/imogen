
#pragma once


namespace Imogen
{

enum ParameterID
{
    inputSourceID,
    mainBypassID,
    leadBypassID,
    harmonyBypassID,
    dryPanID,
    dryWetID,
    adsrAttackID,
    adsrDecayID,
    adsrSustainID,
    adsrReleaseID,
    stereoWidthID,
    lowestPannedID,
    velocitySensID,
    pitchBendRangeID,
    pedalPitchIsOnID,
    pedalPitchThreshID,
    pedalPitchIntervalID,
    descantIsOnID,
    descantThreshID,
    descantIntervalID,
    voiceStealingID,
    inputGainID,
    outputGainID,
    limiterToggleID,
    noiseGateToggleID,
    noiseGateThresholdID,
    compressorToggleID,
    compressorAmountID,
    aftertouchGainToggleID,
    deEsserToggleID,
    deEsserThreshID,
    deEsserAmountID,
    reverbToggleID,
    reverbDryWetID,
    reverbDecayID,
    reverbDuckID,
    reverbLoCutID,
    reverbHiCutID,
    delayToggleID,
    delayDryWetID,
    linkIsEnabledID,
    linkNumSessionPeersID,
    mtsEspIsConnectedID,
    mtsEspScaleNameID,
    midiLatchID,
    editorPitchbendID,
    lastMovedMidiCCnumberID,
    lastMovedMidiCCvalueID,
    currentInputNoteAsStringID,
    currentCentsSharpID,
    guiLightDarkModeID
};
static constexpr int numParams = guiLightDarkModeID + 1;


enum MeterID
{
    inputLevelID,
    outputLevelLID,
    outputLevelRID,
    gateReduxID,
    compReduxID,
    deEssGainReduxID,
    limiterGainReduxID,
    reverbLevelID,
    delayLevelID
};
static constexpr int numMeters = delayLevelID + 1;


/*=========================================================================================*/

namespace l = bav::ParameterValueConversionLambdas;

struct Parameters :     bav::ParameterList
{
    using IntParam = bav::IntParam;
    using FloatParam = bav::FloatParam;
    using BoolParam = bav::BoolParam;
    
    Parameters()
    {
        add (inputMode, dryWet, inputGain, outputGain, mainBypass, leadBypass, harmonyBypass, stereoWidth, lowestPanned, leadPan, pitchbendRange, velocitySens, aftertouchToggle, voiceStealing, pedalToggle, pedalThresh, descantToggle, descantThresh, descantInterval, adsrAttack, adsrDecay, adsrSustain, adsrRelease, noiseGateToggle, noiseGateThresh, deEsserToggle, deEsserThresh, deEsserAmount, compToggle, compAmount, delayToggle, delayDryWet, reverbToggle, reverbDryWet, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut, limiterToggle, midiLatch);
        
        addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, editorPitchbend, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentCentsSharp);
    }
    
    IntParam inputMode { inputSourceID,
        "Input source",
        "Input source",
        1,
        3,
        1,
        juce::String(),
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
    
    IntParam dryWet { dryWetID, "Dry/wet", "Main dry/wet", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    FloatParam inputGain { inputGainID,
        "In",
        "Input gain",
        gainRange,
        0.0f,
        dB,
        juce::AudioProcessorParameter::inputGain,
        l::gain_stringFromFloat,
        l::gain_floatFromString };
    
    FloatParam outputGain { outputGainID,
        "Out",
        "Output gain",
        gainRange,
        -4.0f,
        dB,
        juce::AudioProcessorParameter::outputGain,
        l::gain_stringFromFloat,
        l::gain_floatFromString };
    
    BoolParam mainBypass { mainBypassID, "Main", "Main bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam leadBypass { leadBypassID, "Lead", "Lead bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam harmonyBypass { harmonyBypassID, "Harmony", "Harmony bypass", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam stereoWidth { stereoWidthID, "Width", "Stereo width", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    IntParam lowestPanned { lowestPannedID, "Lowest note", "Lowest panned note", 0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString };
    
    IntParam leadPan { dryPanID, "Lead pan", "Lead pan", 0, 127, 64, juce::String(), l::midiPan_stringFromInt, l::midiPan_intFromString };
    
    IntParam pitchbendRange { pitchBendRangeID, "Pitchbend", "Pitchbend range", 0, 12, 2, st, l::st_stringFromInt, l::st_intFromString };
    
    IntParam velocitySens { velocitySensID, "Velocity", "Velocity amount", 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    BoolParam aftertouchToggle { aftertouchGainToggleID, "Aftertouch", "Aftertouch gain", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam voiceStealing { voiceStealingID, "Stealing", "Voice stealing", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam pedalToggle { pedalPitchIsOnID, "Toggle", "Pedal toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam pedalThresh { pedalPitchThreshID, "Thresh", "Pedal thresh", 0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString };
    
    IntParam pedalInterval { pedalPitchIntervalID, "Interval", "Pedal interval", 1, 12, 12, st, l::st_stringFromInt, l::st_intFromString };
    
    BoolParam descantToggle { descantIsOnID, "Toggle", "Descant toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam descantThresh { descantThreshID, "Thresh", "Descant thresh", 0, 127, 127, juce::String(), l::pcnt_stringFromInt, l::pitch_intFromString };
    
    IntParam descantInterval { descantIntervalID, "Interval", "Descant interval", 1, 12, 12, st, l::st_stringFromInt, l::st_intFromString };
    
    FloatParam adsrAttack { adsrAttackID, "Attack", "ADSR attack", msRange, 0.35f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    FloatParam adsrDecay { adsrDecayID, "Decay", "ADSR decay", msRange, 0.06f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    FloatParam adsrSustain { adsrSustainID, "Sustain", "ADSR sustain", zeroToOneRange, 0.8f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam adsrRelease { adsrReleaseID, "Release", "ADSR release", msRange, 0.1f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString };
    
    BoolParam noiseGateToggle { noiseGateToggleID, "Toggle", "Gate toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam noiseGateThresh { noiseGateThresholdID, "Thresh", "Gate thresh", gainRange, -20.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString };
    
    BoolParam deEsserToggle { deEsserToggleID, "Toggle", "D-S toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam deEsserThresh { deEsserThreshID, "Thresh", "D-S thresh", gainRange, -6.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString };
    
    FloatParam deEsserAmount { deEsserAmountID, "Amount", "D-S amount", zeroToOneRange, 0.5f, dB, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    BoolParam compToggle { compressorToggleID, "Toggle", "Compressor toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    FloatParam compAmount { compressorAmountID,
        "Amount",
        "Compressor amount",
        zeroToOneRange,
        0.35f,
        dB,
        generic,
        l::normPcnt_stringFromInt,
        l::normPcnt_intFromString };
    
    BoolParam delayToggle { delayToggleID, "Toggle", "Delay toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam delayDryWet { delayDryWetID, "Mix", "Delay mix", 0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    BoolParam reverbToggle { reverbToggleID, "Toggle", "Reverb toggle", false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam reverbDryWet { reverbDryWetID, "Mix", "Reverb mix", 0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString };
    
    FloatParam reverbDecay { reverbDecayID, "Decay", "Reverb decay", zeroToOneRange, 0.6f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam reverbDuck { reverbDuckID, "Duck", "Reverb duck", zeroToOneRange, 0.3f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString };
    
    FloatParam reverbLoCut { reverbLoCutID, "Lo cut", "Reverb lo cut", hzRange, 80.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString };
    
    FloatParam reverbHiCut { reverbHiCutID, "Hi cut", "Reverb hi cut", hzRange, 5500.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString };
    
    BoolParam limiterToggle { limiterToggleID, "Toggle", "Limiter toggle", true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString };
    
    BoolParam midiLatch { midiLatchID, "Is latched", "MIDI is latched", false, l::toggle_stringFromBool, l::toggle_boolFromString };
    
    /* */
    
    BoolParam abletonLinkEnabled { linkIsEnabledID, "Toggle", "Ableton link toggle", false, l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam abletonLinkSessionPeers { linkNumSessionPeersID,
        "Num peers",
        "Ableton link num session peers",
        0,
        50,
        0,
        [] (int value, int maximumStringLength) { return juce::String (value).substring (0, maximumStringLength); },
        nullptr };
    
    BoolParam mtsEspIsConnected { mtsEspIsConnectedID, "Is connected", "MTS-ESP is connected", false, l::toggle_stringFromBool, l::toggle_boolFromString };
    
    IntParam editorPitchbend { editorPitchbendID,
        "Pitchbend",
        "GUI pitchbend",
        0,
        127,
        64,
        [] (int value, int maximumStringLength) { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text) { return text.retainCharacters ("1234567890").getIntValue(); } };
    
    IntParam lastMovedMidiController { lastMovedMidiCCnumberID, "Number", "Last moved MIDI controller number", 0, 127, 0, nullptr, nullptr };
    IntParam lastMovedCCValue { lastMovedMidiCCvalueID, "Value", "Last moved MIDI controller value", 0, 127, 0, nullptr, nullptr };
    
    BoolParam guiDarkMode { guiLightDarkModeID,
        "Dark mode",
        "GUI Dark mode",
        true,
        [] (bool val, int maxLength)
        {
            if (val) return TRANS ("Dark mode is on").substring (0, maxLength);
            
            return TRANS ("Dark mode is off").substring (0, maxLength);
        },
        nullptr };
    
    IntParam currentCentsSharp { currentCentsSharpID,
        "Cents sharp",
        "Current input cents sharp",
        -100,
        100,
        0,
        [] (int cents, int maxLength)
        {
            if (cents == 0) return TRANS ("Perfect!");
            
            if (cents > 0) return (juce::String (cents) + TRANS (" cents sharp")).substring (0, maxLength);
            
            return (juce::String (abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
        },
        nullptr };
    
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


    // auto scaleName = std::make_unique< StringNode > (mtsEspScaleNameID, "Scale name", "MTS-ESP scale name", "No active scale");
    
    // auto currentNote = std::make_unique< StringNode > (currentInputNoteAsStringID, "Current note", "Current input note as string", "-");
    

struct Meters :     bav::ParameterList
{
    using Gain = bav::ParameterHolder< bav::GainMeterParameter >;
    
    Meters()
    {
        add (inputLevel, outputLevelL, outputLevelR, gateRedux, compRedux, deEssRedux, limRedux, reverbLevel, delayLevel);
    }
    
    Gain inputLevel { inputLevelID, "In", "Input level", juce::AudioProcessorParameter::inputMeter };
    
    Gain outputLevelL { outputLevelLID, "OutL", "Output level (L)", juce::AudioProcessorParameter::outputMeter };
    Gain outputLevelR { outputLevelRID, "OutL", "Output level (R)", juce::AudioProcessorParameter::outputMeter };
    
    Gain gateRedux { gateReduxID, "Gate redux", "Noise gate gain reduction", compLimMeter };
    Gain compRedux { compReduxID, "Comp redux", "Compressor gain reduction", compLimMeter };
    Gain deEssRedux { deEssGainReduxID, "D-S redux", "De-esser gain reduction", compLimMeter };
    Gain limRedux { limiterGainReduxID, "Lim redux", "Limiter gain reduction", compLimMeter };
    
    Gain reverbLevel { reverbLevelID, "Reverb", "Reverb level", otherMeter };
    Gain delayLevel { delayLevelID, "Delay", "Delay level", otherMeter };
    
private:
    static constexpr auto compLimMeter = juce::AudioProcessorParameter::compressorLimiterGainReductionMeter;
    static constexpr auto otherMeter   = juce::AudioProcessorParameter::otherMeter;
};


} // namespace Imogen
