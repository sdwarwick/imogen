
#pragma once

#include "ImogenCommon.h"

#include <juce_audio_processors/juce_audio_processors.h>



namespace Imogen
{


static inline auto createMeterParameterTree()
{
    using Group = juce::AudioProcessorParameterGroup;
    using Gain  = bav::GainMeterParameter;
    
    constexpr auto compLimMeter = juce::AudioProcessorParameter::compressorLimiterGainReductionMeter;
    constexpr auto otherMeter   = juce::AudioProcessorParameter::otherMeter;
    
    
    auto inputLevel  = std::make_unique<Gain> (inputLevelID,  "In",  "Input level",  juce::AudioProcessorParameter::inputMeter);
    auto outputLevelL = std::make_unique<Gain> (outputLevelLID, "OutL", "Output level (L)", juce::AudioProcessorParameter::outputMeter);
    auto outputLevelR = std::make_unique<Gain> (outputLevelRID, "OutL", "Output level (R)", juce::AudioProcessorParameter::outputMeter);
    
    auto gateGainRedux  = std::make_unique<Gain> (gateReduxID, "Gate redux", "Noise gate gain reduction", compLimMeter);
    auto compGainRedux  = std::make_unique<Gain> (compReduxID, "Comp redux", "Compressor gain reduction", compLimMeter);
    auto deEssGainRedux = std::make_unique<Gain> (deEssGainReduxID, "D-S redux", "De-esser gain reduction", compLimMeter);
    auto limtrGainRedux = std::make_unique<Gain> (limiterGainReduxID, "Lim redux", "Limiter gain reduction", compLimMeter);
    
    auto reverbLevel = std::make_unique<Gain> (reverbLevelID,  "Reverb",  "Reverb level",  otherMeter);
    auto delayLevel  = std::make_unique<Gain> (delayLevelID,   "Delay",   "Delay level",   otherMeter);
    
    return std::make_unique<Group> (meterTreeID(), meterTreeName(), parameterTreeSeparatorString(),
                                    std::move (inputLevel), std::move (outputLevelL), std::move (outputLevelR), std::move (gateGainRedux),
                                    std::move (compGainRedux), std::move (deEssGainRedux), std::move (limtrGainRedux),
                                    std::move (reverbLevel), std::move (delayLevel));
}


static inline auto createAutomatableParameterTree()
{
    using Group = juce::AudioProcessorParameterGroup;
    
    using FloatParameter = bav::FloatParameter;
    using IntParameter   = bav::IntParameter;
    using BoolParameter  = bav::BoolParameter;
    
    namespace l = bav::ParameterValueConversionLambdas;
    
    std::vector<std::unique_ptr<Group>> groups;
    
    const juce::NormalisableRange<float> gainRange      (-60.0f, 0.0f, 0.01f);
    const juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    const juce::NormalisableRange<float> msRange        (0.001f, 1.0f, 0.001f);
    const juce::NormalisableRange<float> hzRange        (40.0f, 10000.0f, 1.0f);
    
    constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
    
    // labels for parameter units
    const auto dB  = TRANS ("dB");
    const auto st  = TRANS ("st");
    const auto sec = TRANS ("sec");
    
    const auto div = parameterTreeSeparatorString();
    
    {   /* MIXING */
        auto inputMode = std::make_unique<IntParameter> (inputSourceID, "Input source", "Input source",
                                                         1, 3, 1, juce::String(),
                                                         [](int value, int maxLength)
                                                         {
            switch (value)
            {
                case (2): return TRANS("Right").substring(0, maxLength);
                case (3): return TRANS("Mix to mono").substring(0, maxLength);
                default:  return TRANS("Left").substring(0, maxLength);
            }
        },
                                                         [](const juce::String& text)
                                                         {
            if (text.containsIgnoreCase (TRANS("Right"))) return 2;
            if (text.containsIgnoreCase (TRANS("mono")) || text.containsIgnoreCase (TRANS("mix"))) return 3;
            return 1;
        });
        
        auto dryWet = std::make_unique<IntParameter> (dryWetID, "Dry/wet", "Main dry/wet",
                                                      0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto inGain = std::make_unique<FloatParameter> (inputGainID, "In", "Input gain",
                                                        gainRange, 0.0f, dB, juce::AudioProcessorParameter::inputGain,
                                                        l::gain_stringFromFloat, l::gain_floatFromString);
        
        auto outGain = std::make_unique<FloatParameter> (outputGainID, "Out", "Output gain",
                                                         gainRange, -4.0f, dB, juce::AudioProcessorParameter::outputGain,
                                                         l::gain_stringFromFloat, l::gain_floatFromString);
        //  subgroup: bypasses
        auto mainBypass    = std::make_unique<BoolParameter> (mainBypassID, "Main", "Main bypass",
                                                              false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto leadBypass    = std::make_unique<BoolParameter> (leadBypassID, "Lead", "Lead bypass",
                                                              false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto harmonyBypass = std::make_unique<BoolParameter> (harmonyBypassID, "Harmony", "Harmony bypass",
                                                              false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), div,
                                                 std::move (mainBypass), std::move (leadBypass), std::move (harmonyBypass));
        //  subgroup: stereo image
        auto stereo_width   = std::make_unique<IntParameter> (stereoWidthID, "Width", "Stereo width",
                                                              0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto stereo_lowest  = std::make_unique<IntParameter> (lowestPannedID, "Lowest note", "Lowest panned note",
                                                              0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString);
        
        auto stereo_leadPan = std::make_unique<IntParameter> (dryPanID, "Lead pan", "Lead pan",
                                                              0, 127, 64, juce::String(), l::midiPan_stringFromInt, l::midiPan_intFromString);
        
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), div,
                                               std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));
        
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), div,
                                                      std::move (inputMode), std::move (dryWet), std::move (inGain), std::move (outGain),
                                                      std::move (bypasses), std::move (stereo)));
    }
    {   /* MIDI */
        auto pitchbendRange   = std::make_unique<IntParameter>  (pitchBendRangeID, "Pitchbend", "Pitchbend range",
                                                                 0, 12, 2, st, l::st_stringFromInt, l::st_intFromString);
        
        auto velocitySens     = std::make_unique<IntParameter>  (velocitySensID, "Velocity", "Velocity amount",
                                                                 0, 100, 100, "%", l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto aftertouchToggle = std::make_unique<BoolParameter> (aftertouchGainToggleID, "Aftertouch", "Aftertouch gain",
                                                                 true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto voiceStealing    = std::make_unique<BoolParameter> (voiceStealingID, "Stealing", "Voice stealing",
                                                                 false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        //  subgroup: pedal pitch
        auto pedal_toggle   = std::make_unique<BoolParameter> (pedalPitchIsOnID, "Toggle", "Pedal toggle",
                                                               false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto pedal_thresh   = std::make_unique<IntParameter>  (pedalPitchThreshID, "Thresh", "Pedal thresh",
                                                               0, 127, 0, juce::String(), l::pitch_stringFromInt, l::pitch_intFromString);
        
        auto pedal_interval = std::make_unique<IntParameter>  (pedalPitchIntervalID, "Interval", "Pedal interval",
                                                               1, 12, 12, st, l::st_stringFromInt, l::st_intFromString);
        
        auto pedal = std::make_unique<Group> ("Pedal pitch", TRANS ("Pedal pitch"), div,
                                              std::move (pedal_toggle), std::move (pedal_thresh), std::move (pedal_interval));
        
        //  subgroup: descant
        auto descant_toggle   = std::make_unique<BoolParameter> (descantIsOnID, "Toggle", "Descant toggle",
                                                                 false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto descant_thresh   = std::make_unique<IntParameter>  (descantThreshID, "Thresh", "Descant thresh",
                                                                 0, 127, 127, juce::String(), l::pcnt_stringFromInt, l::pitch_intFromString);
        
        auto descant_interval = std::make_unique<IntParameter>  (descantIntervalID, "Interval", "Descant interval",
                                                                 1, 12, 12, st, l::st_stringFromInt, l::st_intFromString);
        
        auto descant = std::make_unique<Group> ("Descant", TRANS ("Descant"), div,
                                                std::move (descant_toggle), std::move (descant_thresh), std::move (descant_interval));
        
        groups.emplace_back (std::make_unique<Group> ("MIDI", TRANS ("MIDI"), div,
                                                      std::move (pitchbendRange), std::move (velocitySens), std::move (aftertouchToggle),
                                                      std::move (voiceStealing), std::move (pedal), std::move (descant)));
    }
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> (adsrAttackID, "Attack", "ADSR attack",
                                                         msRange, 0.35f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString);
        
        auto decay   = std::make_unique<FloatParameter> (adsrDecayID, "Decay", "ADSR decay",
                                                         msRange, 0.06f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString);
        
        auto sustain = std::make_unique<FloatParameter> (adsrSustainID, "Sustain", "ADSR sustain",
                                                         zeroToOneRange, 0.8f, "%", generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto release = std::make_unique<FloatParameter> (adsrReleaseID, "Release", "ADSR release",
                                                         msRange, 0.1f, sec, generic, l::sec_stringFromFloat, l::sec_floatFromString);
        
        groups.emplace_back (std::make_unique<Group> ("ADSR", TRANS ("ADSR"), div,
                                                      std::move (attack), std::move (decay), std::move (sustain), std::move (release)));
    }
    {    /* EFFECTS */
        //  subgroup: noise gate
        auto gate_toggle = std::make_unique<BoolParameter>  (noiseGateToggleID, "Toggle", "Gate toggle",
                                                             true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto gate_thresh = std::make_unique<FloatParameter> (noiseGateThresholdID, "Thresh", "Gate thresh",
                                                             gainRange, -20.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString);
        
        auto gate = std::make_unique<Group> ("Noise gate", TRANS ("Noise gate"), div, std::move (gate_toggle), std::move (gate_thresh));
        
        //  subgroup: de-esser
        auto ess_toggle = std::make_unique<BoolParameter>  (deEsserToggleID, "Toggle", "D-S toggle",
                                                            true, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto ess_thresh = std::make_unique<FloatParameter> (deEsserThreshID, "Thresh", "D-S thresh",
                                                            gainRange, -6.0f, dB, generic, l::gain_stringFromFloat, l::gain_floatFromString);
        
        auto ess_amount = std::make_unique<FloatParameter> (deEsserAmountID, "Amount", "D-S amount",
                                                            zeroToOneRange, 0.5f, dB, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), div,
                                              std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));
        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  (compressorToggleID, "Toggle", "Compressor toggle",
                                                             false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto comp_amount = std::make_unique<FloatParameter> (compressorAmountID, "Amount", "Compressor amount",
                                                             zeroToOneRange, 0.35f, dB, generic,
                                                             l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), div, std::move (comp_toggle), std::move (comp_amount));
        
        //  subgroup: delay
        auto delay_toggle = std::make_unique<BoolParameter> (delayToggleID, "Toggle", "Delay toggle",
                                                             false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto delay_mix    = std::make_unique<IntParameter>  (delayDryWetID, "Mix", "Delay mix",
                                                             0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto delay = std::make_unique<Group> ("Delay", TRANS ("Delay"), div, std::move (delay_toggle), std::move (delay_mix));
        
        //  subgroup: reverb
        auto verb_toggle = std::make_unique<BoolParameter>  (reverbToggleID, "Toggle", "Reverb toggle",
                                                             false, juce::String(), l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto verb_dryWet = std::make_unique<IntParameter>   (reverbDryWetID, "Mix", "Reverb mix",
                                                             0, 100, 35, "%", l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto verb_decay  = std::make_unique<FloatParameter> (reverbDecayID, "Decay", "Reverb decay",
                                                             zeroToOneRange, 0.6f, "%", generic,
                                                             l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto verb_duck   = std::make_unique<FloatParameter> (reverbDuckID, "Duck", "Reverb duck",
                                                             zeroToOneRange, 0.3f, "%", generic,
                                                             l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto verb_loCut  = std::make_unique<FloatParameter> (reverbLoCutID, "Lo cut", "Reverb lo cut",
                                                             hzRange, 80.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString);
        
        auto verb_hiCut  = std::make_unique<FloatParameter> (reverbHiCutID, "Hi cut", "Reverb hi cut",
                                                             hzRange, 5500.0f, TRANS ("Hz"), generic, l::hz_stringFromFloat, l::hz_floatFromString);
        
        auto reverb = std::make_unique<Group> ("Reverb", TRANS ("Reverb"), div,
                                               std::move (verb_toggle), std::move (verb_dryWet), std::move (verb_decay),
                                               std::move (verb_duck), std::move (verb_loCut), std::move (verb_hiCut));
        //  limiter
        auto limiter = std::make_unique<Group> ("Limiter", TRANS ("Limiter"), div,
                                                std::make_unique<BoolParameter>  (limiterToggleID,
                                                                                  "Toggle", "Limiter toggle",
                                                                                  true, juce::String(),
                                                                                  l::toggle_stringFromBool, l::toggle_boolFromString));
        
        groups.emplace_back (std::make_unique<Group> ("Effects", TRANS ("Effects"), div,
                                                      std::move (gate),  std::move (deEss), std::move (compressor),
                                                      std::move (delay), std::move (reverb), std::move (limiter)));
    }
    
    auto mainGroup = std::make_unique<Group> (parameterTreeID(), parameterTreeName(), div);
    
    for (auto& group : groups)
        mainGroup->addChild (std::move (group));
    
    return mainGroup;
}


static inline auto createParameterTree()
{
    return std::make_unique<juce::AudioProcessorParameterGroup> ("Imogen", "Imogen", parameterTreeSeparatorString(),
                                                                 createAutomatableParameterTree(),
                                                                 createMeterParameterTree());
}



static inline auto createNonAutomatableParametersTree()
{
    using Group = bav::NonParamValueTreeNodeGroup;
    
    using IntNode    = bav::IntValueTreeNode;
    using BoolNode   = bav::BoolValueTreeNode;
    using StringNode = bav::StringValueTreeNode;
    
    namespace l = bav::ParameterValueConversionLambdas;
    
    std::vector<std::unique_ptr<Group>> groups;
    
    { /* Ableton Link */
        auto isEnabled = std::make_unique<BoolNode> (linkIsEnabledID, "Toggle", "Ableton link toggle",
                                                     false, l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto numSessionPeers = std::make_unique<IntNode> (linkNumSessionPeersID, "Num peers",
                                                          "Ableton link num session peers",
                                                          0, 50, 0,
                                                          [](int value, int maximumStringLength)
                                                          { return juce::String(value).substring (0, maximumStringLength); },
                                                          nullptr);
        
        groups.emplace_back (std::make_unique<Group> ("Ableton Link",
                                                      std::move (isEnabled), std::move (numSessionPeers)));
    }
    { /* MTS-ESP */
        auto isConnected = std::make_unique<BoolNode> (mtsEspIsConnectedID, "Is connected", "MTS-ESP is connected",
                                                       false, l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto scaleName = std::make_unique<StringNode> (mtsEspScaleNameID, "Scale name", "MTS-ESP scale name", "No active scale");
        
        groups.emplace_back (std::make_unique<Group> ("MTS-ESP",
                                                      std::move (isConnected), std::move (scaleName)));
    }
    { /* MIDI */
        auto isLatched = std::make_unique<BoolNode> (midiLatchID, "Is latched", "MIDI is latched",
                                                     false, l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto editorPitchbend = std::make_unique<IntNode> (editorPitchbendID, "Pitchbend", "GUI pitchbend", 0, 127, 64,
                                                          [](int value, int maximumStringLength)
                                                          { return juce::String(value).substring (0, maximumStringLength); },
                                                          [](const juce::String& text)
                                                          { return text.retainCharacters ("1234567890").getIntValue(); });
        
        // subgroup: last moved MIDI controller
        auto number = std::make_unique<IntNode> (lastMovedMidiCCnumberID, "Number", "Last moved MIDI controller number", 0, 127, 0,
                                                 nullptr,
                                                 nullptr);
        
        auto value = std::make_unique<IntNode> (lastMovedMidiCCvalueID, "Value", "Last moved MIDI controller value", 0, 127, 0,
                                                nullptr,
                                                nullptr);
        
        auto lastMovedController = std::make_unique<Group> ("Last moved MIDI controller",
                                                            std::move (number), std::move (value));
        
        groups.emplace_back (std::make_unique<Group> ("MIDI",
                                                      std::move (isLatched), std::move (editorPitchbend), std::move (lastMovedController)));
    }
    
    { /* GUI state */
        auto lightDarkMode = std::make_unique<BoolNode> (guiLightDarkModeID, "Dark mode", "GUI Dark mode",
                                                         true,
                                                         [](bool val, int maxLength)
                                                         {
                                                             if (val)
                                                                 return juce::String("Dark mode is on").substring (0, maxLength);
            
                                                             return juce::String("Dark mode is off").substring (0, maxLength);
                                                         },
                                                         nullptr);
        
        groups.emplace_back (std::make_unique<Group> ("GUI state",
                                                      std::move (lightDarkMode)));
    }
    
    { /* Current intonation information */
        auto currentNote = std::make_unique<StringNode> (currentInputNoteAsStringID,
                                                         "Current note", "Current input note as string",
                                                         "-");
        
        auto currentCentsSharp = std::make_unique<IntNode> (currentCentsSharpID, "Cents sharp", "Current input cents sharp",
                                                            -100, 100, 0,
                                                            [](int cents, int maxLength)
                                                            {
                                                                if (cents == 0)
                                                                    return TRANS ("Perfect!");
                                                  
                                                                if (cents > 0)
                                                                    return (juce::String(cents) + TRANS (" cents sharp")).substring (0, maxLength);
            
                                                                return (juce::String(abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
                                                            },
                                                            nullptr);
        
        groups.emplace_back (std::make_unique<Group> ("Intonation information",
                                                      std::move (currentNote), std::move (currentCentsSharp)));
    }
    
    auto mainGroup = std::make_unique<Group> ("Non-parameter properties");
    
    for (auto& group : groups)
        mainGroup->addChild (std::move (group));
    
    return mainGroup;
}


}  // namespace
