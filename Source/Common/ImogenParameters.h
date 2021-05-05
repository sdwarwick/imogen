
#pragma once

#include "ImogenCommon.h"

#include <juce_audio_processors/juce_audio_processors.h>



namespace Imogen
{


struct ImogenFloatParameter :   public bav::FloatParameter
{
    ImogenFloatParameter (ParameterID paramtrID, const juce::NormalisableRange<float>& nRange, float defaultVal,
                          juce::AudioProcessorParameter::Category parameterCategory = juce::AudioProcessorParameter::genericParameter,
                          std::function<juce::String(float value, int maximumStringLength)> stringFromFloat = nullptr,
                          std::function<float(const juce::String& text)> floatFromString = nullptr)
    
        :  bav::FloatParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                                nRange, defaultVal, juce::String(), parameterCategory, stringFromFloat, floatFromString)
    {
        orig()->setValueNotifyingHost (defaultVal);
    }
};


struct ImogenIntParameter   :   public bav::IntParameter
{
    ImogenIntParameter (ParameterID paramtrID, int min, int max, int defaultVal,
                        std::function<juce::String(int value, int maximumStringLength)> stringFromInt = nullptr,
                        std::function<int(const juce::String& text)> intFromString = nullptr)
    
    :  bav::IntParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                          min, max, defaultVal, juce::String(), stringFromInt, intFromString)
    {
        orig()->setValueNotifyingHost (defaultVal);
    }
};


struct ImogenBoolParameter  :   public bav::BoolParameter
{
    ImogenBoolParameter (ParameterID paramtrID, bool defaultVal,
                         std::function<juce::String(bool value, int maximumStringLength)> stringFromBool = nullptr,
                         std::function<bool(const juce::String& text)> boolFromString = nullptr)
    
    :  bav::BoolParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                           defaultVal, juce::String(), stringFromBool, boolFromString)
    {
        orig()->setValueNotifyingHost (defaultVal);
    }
};



static inline std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterTree()
{
    using Group = juce::AudioProcessorParameterGroup;
    
    using FloatParameter = ImogenFloatParameter;
    using IntParameter   = ImogenIntParameter;
    using BoolParameter  = ImogenBoolParameter;
    
    std::vector<std::unique_ptr<Group>> groups;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    juce::NormalisableRange<float> hzRange (40.0f, 10000.0f, 1.0f);
    
    constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
    
    std::function< juce::String (bool value, int maximumStringLength) >  toggle_stringFromBool = [](bool value, int maxLength) { return value ? TRANS("On").substring(0, maxLength) : TRANS("Off").substring(0, maxLength); };
    std::function< bool (const juce::String& text) >                     toggle_boolFromString = [](const juce::String& text) { return (text.containsIgnoreCase (TRANS("On")) || text.containsIgnoreCase (TRANS("Yes")) || text.containsIgnoreCase (TRANS("True"))); };
    
    std::function< juce::String (float value, int maximumStringLength) > gain_stringFromFloat = [](float value, int maxLength) { return (juce::String(value) + " " + TRANS("dB")).substring(0, maxLength); };
    std::function< float (const juce::String& text) >                    gain_floatFromString = [](const juce::String& text)
    {
        const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("dB"));
        
        if (token_location > -1)
            return text.substring (0, token_location).trim().getFloatValue();
        
        return text.trim().getFloatValue();
    };
    
    std::function< juce::String (int value, int maximumStringLength) >   pcnt_stringFromInt  = [](int value, int maxLength) { return (juce::String(value) + "%").substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      pcnt_intFromString  = [](const juce::String& text)
    {
        const auto token_location = text.indexOf ("%");
        
        if (token_location > -1)
            return text.substring (0, token_location).trim().getIntValue();
        
        return text.trim().getIntValue();
    };
    
    std::function< juce::String (float value, int maximumStringLength) > sec_stringFromFloat = [](float value, int maxLength) { return (juce::String(value) + " " + TRANS("sec")).substring(0, maxLength); };
    std::function< float (const juce::String& text) >                    sec_floatFromString = [](const juce::String& text)
    {
        const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("sec"));
        
        if (token_location > -1)
            return text.substring (0, token_location).trim().getFloatValue();
        
        return text.trim().getFloatValue();
    };
    
    std::function< juce::String (float value, int maximumStringLength) > hz_stringFromFloat  = [](float value, int maxLength) { auto string = (value < 1000.0f) ? juce::String (value) + " " + TRANS("Hz") : juce::String (value * 0.001f) + " " + TRANS("kHz"); return string.substring(0, maxLength); };
    std::function< float (const juce::String& text) >                    hz_floatFromString  = [](juce::String text)
    {
        const auto kHz_token_location = text.indexOfWholeWordIgnoreCase (TRANS("kHz"));
        
        if (kHz_token_location > -1)
            return text.substring (0, kHz_token_location).trim().getFloatValue() * 1000.0f;
        
        const auto hz_token_location = text.indexOfWholeWordIgnoreCase (TRANS("Hz"));
        
        if (hz_token_location > -1)
            return text.substring (0, hz_token_location).trim().getFloatValue();
        
        return text.trim().getFloatValue();
    };
    
    std::function< juce::String (int value, int maximumStringLength) >   st_stringFromInt    = [](int value, int maxLength) { return (juce::String(value) + " " + TRANS("st")).substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      st_intFromString    = [](const juce::String& text)
    {
        const auto token_location = text.indexOfWholeWordIgnoreCase (TRANS("st"));
        
        if (token_location > -1)
            return text.substring (0, token_location).trim().getIntValue();
        
        return text.trim().getIntValue();
    };
    
    std::function< juce::String (int value, int maximumStringLength) >   pitch_stringFromInt = [](int value, int maxLength) { return bav::midi::pitchToString (value, true).substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      pitch_intFromString = [](const juce::String& text)
    {
        const auto pitchClassTokens = juce::String("AaBbCcDdEeFfGg#") + bav::gui::getSharpSymbol() + bav::gui::getFlatSymbol();
        
        if (text.containsAnyOf (pitchClassTokens))
            return bav::midi::stringToPitch (text.trim());
        
        return text.trim().getIntValue();
    };
    
    std::function< juce::String (int value, int maximumStringLength) >   normPcnt_stringFromInt = [](int value, int maxLength) { return (juce::String(value * 100.0f) + "%").substring(0, maxLength); };
    std::function< int (const juce::String& text) >                      normPcnt_intFromString = [](const juce::String& text)
    {
        const auto token_location = text.indexOf ("%");
        
        if (token_location > -1)
            return text.substring (0, token_location).trim().getFloatValue() * 0.01f;
        
        return text.trim().getFloatValue();
    };
    
    
    {   /* MIXING */
        auto inputMode = std::make_unique<IntParameter> (inputSourceID, 1, 3, 1,
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
        
        auto dryWetP = std::make_unique<IntParameter>   (dryWetID, 0, 100, 100, pcnt_stringFromInt, pcnt_intFromString);
        
        auto inGain = std::make_unique<FloatParameter>  (inputGainID, gainRange, 0.0f, juce::AudioProcessorParameter::inputGain, gain_stringFromFloat, gain_floatFromString);
        
        auto outGain = std::make_unique<FloatParameter> (outputGainID, gainRange, -4.0f, juce::AudioProcessorParameter::outputGain, gain_stringFromFloat, gain_floatFromString);
        
        //  subgroup: bypasses
        auto mainBypassP = std::make_unique<BoolParameter> (mainBypassID, false, toggle_stringFromBool, toggle_boolFromString);
        auto leadBypassP = std::make_unique<BoolParameter> (leadBypassID, false, toggle_stringFromBool, toggle_boolFromString);
        auto harmonyBypassP = std::make_unique<BoolParameter> (harmonyBypassID, false, toggle_stringFromBool, toggle_boolFromString);
        
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), "|", std::move (mainBypassP), std::move (leadBypassP), std::move (harmonyBypassP));
        
        //  subgroup: stereo image
        auto stereo_width  = std::make_unique<IntParameter> (stereoWidthID, 0, 100, 100, pcnt_stringFromInt, pcnt_intFromString);
        auto stereo_lowest = std::make_unique<IntParameter> (lowestPannedID, 0, 127, 0, pitch_stringFromInt, pitch_intFromString);
        
        auto stereo_leadPan = std::make_unique<IntParameter>  (dryPanID, 0, 127, 64,
                                                               [](int value, int maxLength) { return bav::midiPanIntToString (value).substring(0, maxLength); },
                                                               [](const juce::String& text) { return bav::midiPanStringToInt (text); });
        
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), "|", std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));
        
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), "|",
                                                      std::move (inputMode), std::move (dryWetP), std::move (inGain), std::move (outGain), std::move (bypasses), std::move (stereo)));
    }
    {   /* MIDI */
        auto pitchbendRange = std::make_unique<IntParameter>   (pitchBendRangeID, 0, 12, 2, st_stringFromInt, st_intFromString);
        auto velocitySensP = std::make_unique<IntParameter>    (velocitySensID, 0, 100, 100, pcnt_stringFromInt, pcnt_intFromString);
        auto aftertouchToggle = std::make_unique<BoolParameter>(aftertouchGainToggleID, true, toggle_stringFromBool, toggle_boolFromString);
        auto voiceStealingP = std::make_unique<BoolParameter>  (voiceStealingID, false, toggle_stringFromBool, toggle_boolFromString);
        
        //  subgroup: pedal pitch
        auto pedal_toggle = std::make_unique<BoolParameter>  (pedalPitchIsOnID, false, toggle_stringFromBool, toggle_boolFromString);
        auto pedal_thresh = std::make_unique<IntParameter>   (pedalPitchThreshID, 0, 127, 0, pitch_stringFromInt, pitch_intFromString);
        auto pedal_interval = std::make_unique<IntParameter> (pedalPitchIntervalID, 1, 12, 12, st_stringFromInt, st_intFromString);
        
        auto pedal = std::make_unique<Group> ("Pedal pitch", TRANS ("Pedal pitch"), "|", std::move (pedal_toggle), std::move (pedal_thresh), std::move (pedal_interval));
        
        //  subgroup: descant
        auto descant_toggle = std::make_unique<BoolParameter>  (descantIsOnID, false, toggle_stringFromBool, toggle_boolFromString);
        auto descant_thresh = std::make_unique<IntParameter>   (descantThreshID, 0, 127, 127, pcnt_stringFromInt, pitch_intFromString);
        auto descant_interval = std::make_unique<IntParameter> (descantIntervalID, 1, 12, 12, st_stringFromInt, st_intFromString);
        
        auto descant = std::make_unique<Group> ("Descant", TRANS ("Descant"), "|", std::move (descant_toggle), std::move (descant_thresh), std::move (descant_interval));
        
        groups.emplace_back (std::make_unique<Group> ("MIDI", TRANS ("MIDI"), "|",
                                                      std::move(pitchbendRange), std::move (velocitySensP), std::move (aftertouchToggle), std::move (voiceStealingP), std::move (pedal), std::move (descant)));
    }
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> (adsrAttackID, msRange, 0.35f, generic, sec_stringFromFloat, sec_floatFromString);
        auto decay   = std::make_unique<FloatParameter> (adsrDecayID, msRange, 0.06f, generic, sec_stringFromFloat, sec_floatFromString);
        auto sustain = std::make_unique<FloatParameter> (adsrSustainID, zeroToOneRange, 0.8f, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto release = std::make_unique<FloatParameter> (adsrReleaseID, msRange, 0.1f, generic, sec_stringFromFloat, sec_floatFromString);
        
        groups.emplace_back (std::make_unique<Group> ("ADSR", TRANS ("ADSR"), "|",
                                                      std::move (attack), std::move (decay), std::move (sustain), std::move (release)));
    }
    {    /* EFFECTS */
        //  subgroup: noise gate
        auto gate_toggle = std::make_unique<BoolParameter>  (noiseGateToggleID, true, toggle_stringFromBool, toggle_boolFromString);
        auto gate_thresh = std::make_unique<FloatParameter> (noiseGateThresholdID, gainRange, -20.0f, generic, gain_stringFromFloat, gain_floatFromString);
        
        auto gate = std::make_unique<Group> ("Noise gate", TRANS ("Noise gate"), "|", std::move (gate_toggle), std::move (gate_thresh));
        
        //  subgroup: de-esser
        auto ess_toggle = std::make_unique<BoolParameter>  (deEsserToggleID, true, toggle_stringFromBool, toggle_boolFromString);
        auto ess_thresh = std::make_unique<FloatParameter> (deEsserThreshID, gainRange, -6.0f, generic, gain_stringFromFloat, gain_floatFromString);
        auto ess_amount = std::make_unique<FloatParameter> (deEsserAmountID, zeroToOneRange, 0.5f, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), "|", std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));
        
        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  (compressorToggleID, false, toggle_stringFromBool, toggle_boolFromString);
        auto comp_amount = std::make_unique<FloatParameter> (compressorAmountID, zeroToOneRange, 0.35f, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), "|", std::move (comp_toggle), std::move (comp_amount));
        
        //  subgroup: delay
        auto delay_toggle = std::make_unique<BoolParameter> (delayToggleID, false, toggle_stringFromBool, toggle_boolFromString);
        auto delay_mix = std::make_unique<IntParameter>     (delayDryWetID, 0, 100, 35, pcnt_stringFromInt, pcnt_intFromString);
        
        auto delay = std::make_unique<Group> ("Delay", TRANS ("Delay"), "|", std::move (delay_toggle), std::move (delay_mix));
        
        //  subgroup: reverb
        auto verb_toggle = std::make_unique<BoolParameter>  (reverbToggleID, false, toggle_stringFromBool, toggle_boolFromString);
        auto verb_dryWet = std::make_unique<IntParameter>   (reverbDryWetID, 0, 100, 35, pcnt_stringFromInt, pcnt_intFromString);
        auto verb_decay  = std::make_unique<FloatParameter> (reverbDecayID, zeroToOneRange, 0.6f, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto verb_duck   = std::make_unique<FloatParameter> (reverbDuckID, zeroToOneRange, 0.3f, generic, normPcnt_stringFromInt, normPcnt_intFromString);
        auto verb_loCut  = std::make_unique<FloatParameter> (reverbLoCutID, hzRange, 80.0f, generic, hz_stringFromFloat, hz_floatFromString);
        auto verb_hiCut  = std::make_unique<FloatParameter> (reverbHiCutID, hzRange, 5500.0f, generic, hz_stringFromFloat, hz_floatFromString);
        
        auto reverb = std::make_unique<Group> ("Reverb", TRANS ("Reverb"), "|", std::move (verb_toggle), std::move (verb_dryWet), std::move (verb_decay), std::move (verb_duck), std::move (verb_loCut), std::move (verb_hiCut));
        
        //  limiter
        auto limiter = std::make_unique<Group> ("Limiter", TRANS ("Limiter"), "|",
                                                std::make_unique<BoolParameter>  (limiterToggleID, true, toggle_stringFromBool, toggle_boolFromString));
        
        groups.emplace_back (std::make_unique<Group> ("Effects", TRANS ("Effects"), "|",
                                                      std::move (gate),  std::move (deEss), std::move (compressor),
                                                      std::move (delay), std::move (reverb), std::move (limiter)));
    }
    
    auto mainGroup = std::make_unique<Group> ("ImogenParameters", TRANS ("Imogen Parameters"), "|");
    
    for (auto& group : groups)
        mainGroup->addChild (std::move (group));
    
    return mainGroup;
}



static inline void createValueTree (juce::ValueTree& tree,
                                    const juce::AudioProcessorParameterGroup& parameterTree)
{
    for (auto* node : parameterTree)
    {
        if (auto* param = node->getParameter())
        {
            if (auto* parameter = dynamic_cast<bav::Parameter*>(param))
            {
                const auto paramID = static_cast<Imogen::ParameterID> (parameter->key());
                const auto key = juce::Identifier { getParameterIdentifier (paramID) };
                
                tree.setProperty (key, parameter->getCurrentDenormalizedValue(), nullptr);
            }
            else
            {
                jassertfalse;
            }
        }
        else if (auto* thisGroup = node->getGroup())
        {
            createValueTree (tree, *thisGroup);
        }
    }
}


static inline juce::Identifier imogenValueTreeType()
{
    static juce::Identifier type { "ImogenParameters" };
    return type;
}


}  // namespace
