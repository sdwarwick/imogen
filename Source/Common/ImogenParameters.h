
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
    
        :  bav::FloatParameter (paramtrID, getParameterNameVerboseNoSpaces (paramtrID), getParameterNameVerbose (paramtrID),
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
    
    :  bav::IntParameter (paramtrID, getParameterNameVerboseNoSpaces (paramtrID), getParameterNameVerbose (paramtrID),
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
    
    :  bav::BoolParameter (paramtrID, getParameterNameVerboseNoSpaces (paramtrID), getParameterNameVerbose (paramtrID),
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
    
    namespace l = bav::ParameterValueConversionLambdas;
    
    std::vector<std::unique_ptr<Group>> groups;
    
    static const juce::NormalisableRange<float> gainRange      (-60.0f, 0.0f, 0.01f);
    static const juce::NormalisableRange<float> zeroToOneRange (0.0f, 1.0f, 0.01f);
    static const juce::NormalisableRange<float> msRange        (0.001f, 1.0f, 0.001f);
    static const juce::NormalisableRange<float> hzRange        (40.0f, 10000.0f, 1.0f);
    
    constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
    
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
        
        auto dryWet = std::make_unique<IntParameter> (dryWetID, 0, 100, 100, l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto inGain = std::make_unique<FloatParameter> (inputGainID, gainRange, 0.0f,
                                                        juce::AudioProcessorParameter::inputGain,
                                                        l::gain_stringFromFloat, l::gain_floatFromString);
        
        auto outGain = std::make_unique<FloatParameter> (outputGainID, gainRange, -4.0f,
                                                         juce::AudioProcessorParameter::outputGain,
                                                         l::gain_stringFromFloat, l::gain_floatFromString);
        //  subgroup: bypasses
        auto mainBypass    = std::make_unique<BoolParameter> (mainBypassID,    false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto leadBypass    = std::make_unique<BoolParameter> (leadBypassID,    false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto harmonyBypass = std::make_unique<BoolParameter> (harmonyBypassID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        
        auto bypasses = std::make_unique<Group> ("Bypasses", TRANS ("Bypasses"), "|",
                                                 std::move (mainBypass), std::move (leadBypass), std::move (harmonyBypass));
        //  subgroup: stereo image
        auto stereo_width   = std::make_unique<IntParameter> (stereoWidthID, 0, 100, 100, l::pcnt_stringFromInt, l::pcnt_intFromString);
        auto stereo_lowest  = std::make_unique<IntParameter> (lowestPannedID, 0, 127, 0,  l::pitch_stringFromInt, l::pitch_intFromString);
        auto stereo_leadPan = std::make_unique<IntParameter> (dryPanID, 0, 127, 64, l::midiPan_stringFromInt, l::midiPan_intFromString);
        
        auto stereo = std::make_unique<Group> ("Stereo image", TRANS ("Stereo image"), "|",
                                               std::move (stereo_width), std::move (stereo_lowest), std::move (stereo_leadPan));
        
        groups.emplace_back (std::make_unique<Group> ("Mixing", TRANS ("Mixing"), "|",
                                                      std::move (inputMode), std::move (dryWet), std::move (inGain), std::move (outGain),
                                                      std::move (bypasses), std::move (stereo)));
    }
    {   /* MIDI */
        auto pitchbendRange   = std::make_unique<IntParameter>  (pitchBendRangeID, 0, 12, 2,   l::st_stringFromInt,      l::st_intFromString);
        auto velocitySens     = std::make_unique<IntParameter>  (velocitySensID, 0, 100, 100,  l::pcnt_stringFromInt,    l::pcnt_intFromString);
        auto aftertouchToggle = std::make_unique<BoolParameter> (aftertouchGainToggleID, true, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto voiceStealing    = std::make_unique<BoolParameter> (voiceStealingID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        
        //  subgroup: pedal pitch
        auto pedal_toggle   = std::make_unique<BoolParameter> (pedalPitchIsOnID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto pedal_thresh   = std::make_unique<IntParameter>  (pedalPitchThreshID, 0, 127, 0, l::pitch_stringFromInt, l::pitch_intFromString);
        auto pedal_interval = std::make_unique<IntParameter>  (pedalPitchIntervalID, 1, 12, 12, l::st_stringFromInt, l::st_intFromString);
        
        auto pedal = std::make_unique<Group> ("Pedal pitch", TRANS ("Pedal pitch"), "|",
                                              std::move (pedal_toggle), std::move (pedal_thresh), std::move (pedal_interval));
        
        //  subgroup: descant
        auto descant_toggle   = std::make_unique<BoolParameter> (descantIsOnID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto descant_thresh   = std::make_unique<IntParameter>  (descantThreshID, 0, 127, 127, l::pcnt_stringFromInt, l::pitch_intFromString);
        auto descant_interval = std::make_unique<IntParameter>  (descantIntervalID, 1, 12, 12, l::st_stringFromInt, l::st_intFromString);
        
        auto descant = std::make_unique<Group> ("Descant", TRANS ("Descant"), "|",
                                                std::move (descant_toggle), std::move (descant_thresh), std::move (descant_interval));
        
        groups.emplace_back (std::make_unique<Group> ("MIDI", TRANS ("MIDI"), "|",
                                                      std::move (pitchbendRange), std::move (velocitySens), std::move (aftertouchToggle),
                                                      std::move (voiceStealing), std::move (pedal), std::move (descant)));
    }
    {   /* ADSR */
        auto attack  = std::make_unique<FloatParameter> (adsrAttackID, msRange, 0.35f, generic, l::sec_stringFromFloat, l::sec_floatFromString);
        auto decay   = std::make_unique<FloatParameter> (adsrDecayID, msRange, 0.06f, generic,  l::sec_stringFromFloat, l::sec_floatFromString);
        auto sustain = std::make_unique<FloatParameter> (adsrSustainID, zeroToOneRange, 0.8f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        auto release = std::make_unique<FloatParameter> (adsrReleaseID, msRange, 0.1f, generic, l::sec_stringFromFloat, l::sec_floatFromString);
        
        groups.emplace_back (std::make_unique<Group> ("ADSR", TRANS ("ADSR"), "|",
                                                      std::move (attack), std::move (decay), std::move (sustain), std::move (release)));
    }
    {    /* EFFECTS */
        //  subgroup: noise gate
        auto gate_toggle = std::make_unique<BoolParameter>  (noiseGateToggleID, true, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto gate_thresh = std::make_unique<FloatParameter> (noiseGateThresholdID, gainRange, -20.0f, generic, l::gain_stringFromFloat, l::gain_floatFromString);
        
        auto gate = std::make_unique<Group> ("Noise gate", TRANS ("Noise gate"), "|", std::move (gate_toggle), std::move (gate_thresh));
        
        //  subgroup: de-esser
        auto ess_toggle = std::make_unique<BoolParameter>  (deEsserToggleID, true, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto ess_thresh = std::make_unique<FloatParameter> (deEsserThreshID, gainRange, -6.0f, generic, l::gain_stringFromFloat, l::gain_floatFromString);
        auto ess_amount = std::make_unique<FloatParameter> (deEsserAmountID, zeroToOneRange, 0.5f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto deEss = std::make_unique<Group> ("De-esser", TRANS ("De-esser"), "|",
                                              std::move (ess_toggle), std::move (ess_thresh), std::move (ess_amount));
        //  subgroup: compressor
        auto comp_toggle = std::make_unique<BoolParameter>  (compressorToggleID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto comp_amount = std::make_unique<FloatParameter> (compressorAmountID, zeroToOneRange, 0.35f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        
        auto compressor = std::make_unique<Group> ("Compressor", TRANS ("Compressor"), "|", std::move (comp_toggle), std::move (comp_amount));
        
        //  subgroup: delay
        auto delay_toggle = std::make_unique<BoolParameter> (delayToggleID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto delay_mix    = std::make_unique<IntParameter>  (delayDryWetID, 0, 100, 35, l::pcnt_stringFromInt, l::pcnt_intFromString);
        
        auto delay = std::make_unique<Group> ("Delay", TRANS ("Delay"), "|", std::move (delay_toggle), std::move (delay_mix));
        
        //  subgroup: reverb
        auto verb_toggle = std::make_unique<BoolParameter>  (reverbToggleID, false, l::toggle_stringFromBool, l::toggle_boolFromString);
        auto verb_dryWet = std::make_unique<IntParameter>   (reverbDryWetID, 0, 100, 35, l::pcnt_stringFromInt, l::pcnt_intFromString);
        auto verb_decay  = std::make_unique<FloatParameter> (reverbDecayID, zeroToOneRange, 0.6f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        auto verb_duck   = std::make_unique<FloatParameter> (reverbDuckID, zeroToOneRange, 0.3f, generic, l::normPcnt_stringFromInt, l::normPcnt_intFromString);
        auto verb_loCut  = std::make_unique<FloatParameter> (reverbLoCutID, hzRange, 80.0f,   generic, l::hz_stringFromFloat, l::hz_floatFromString);
        auto verb_hiCut  = std::make_unique<FloatParameter> (reverbHiCutID, hzRange, 5500.0f, generic, l::hz_stringFromFloat, l::hz_floatFromString);
        
        auto reverb = std::make_unique<Group> ("Reverb", TRANS ("Reverb"), "|",
                                               std::move (verb_toggle), std::move (verb_dryWet), std::move (verb_decay),
                                               std::move (verb_duck), std::move (verb_loCut), std::move (verb_hiCut));
        //  limiter
        auto limiter = std::make_unique<Group> ("Limiter", TRANS ("Limiter"), "|",
                                                std::make_unique<BoolParameter>  (limiterToggleID, true,
                                                                                  l::toggle_stringFromBool, l::toggle_boolFromString));
        
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
                
                tree.setProperty (getParameterIdentifier (paramID), parameter->getCurrentDenormalizedValue(), nullptr);
                tree.setProperty (getParameterGestureIdentifier (paramID), false, nullptr);
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


/* Updates the ValueTree with changes from the parameter object */
class ParameterToValueTreeAttachment   :     public juce::Timer,
                                             public juce::AudioProcessorParameter::Listener
{
public:
    ParameterToValueTreeAttachment (bav::Parameter* paramToUse,
                                    juce::ValueTree& treeToUse,
                                    ParameterID parameterID)
        : param (paramToUse),
          paramID (parameterID),
          tree (treeToUse)
    {
        param->orig()->addListener (this);
        startTimerHz (10);
        lastSentValue = param->getCurrentDenormalizedValue();
        lastSentChangeState = false;
    }
    
    virtual ~ParameterToValueTreeAttachment() override
    {
        stopTimer();
        param->orig()->removeListener (this);
    }
    
    void timerCallback() override final
    {
        const auto newValue = param->getCurrentDenormalizedValue();
        
        if (lastSentValue != newValue)
        {
            lastSentValue = newValue;
            tree.setProperty (getParameterIdentifier (paramID), newValue, nullptr);
        }
        
        const auto changeState = isChanging.load();
        
        if (lastSentChangeState != changeState)
        {
            lastSentChangeState = changeState;
            tree.setProperty (getParameterGestureIdentifier (paramID), isChanging.load(), nullptr);
        }
    }
    
    void parameterValueChanged (int, float) override final { }
    
    void parameterGestureChanged (int, bool gestureIsStarting) override final
    {
        isChanging.store (gestureIsStarting);
    }
    
    
private:
    bav::Parameter* const param;
    const ParameterID paramID;
    juce::ValueTree& tree;
    
    float lastSentValue;
    bool  lastSentChangeState;
    std::atomic<bool> isChanging;
};


/* Updates the parameter object with changes from the ValueTree */
class ValueTreeToParameterAttachment   :    public juce::ValueTree::Listener
{
public:
    ValueTreeToParameterAttachment (bav::Parameter* paramToUse,
                                    juce::ValueTree& treeToUse,
                                    ParameterID parameterID)
        : param (paramToUse),
          paramID (parameterID),
          tree (treeToUse),
          paramIdentifier (getParameterIdentifier (paramID)),
          paramGestureIdentifier (getParameterGestureIdentifier (paramID))
    {
        tree.addListener (this);
        lastSentValue = param->getCurrentDenormalizedValue();
        lastSentChangeState = false;
    }
    
    ~ValueTreeToParameterAttachment() override
    {
        tree.removeListener (this);
    }
    
    void valueTreePropertyChanged (juce::ValueTree& ltree, const juce::Identifier& property) override final
    {
        if (property == paramIdentifier)
        {
            const float newValue = ltree.getProperty (paramIdentifier);
            
            if (lastSentValue != newValue)
            {
                lastSentValue = newValue;
                param->orig()->setValueNotifyingHost (newValue);
            }
        }
        else if (property == paramGestureIdentifier)
        {
            const bool isNowChanging = ltree.getProperty (paramGestureIdentifier);
            
            if (lastSentChangeState != isNowChanging)
            {
                lastSentChangeState = isNowChanging;
                
                if (isNowChanging)
                    param->orig()->beginChangeGesture();
                else
                    param->orig()->endChangeGesture();
            }
        }
    }
    
    
private:
    bav::Parameter* const param;
    const ParameterID paramID;
    juce::ValueTree& tree;
    
    float lastSentValue;
    bool  lastSentChangeState;
    
    const juce::Identifier paramIdentifier;
    const juce::Identifier paramGestureIdentifier;
};


struct ParameterAttachment :    public ParameterToValueTreeAttachment,
                                public ValueTreeToParameterAttachment
{
    ParameterAttachment (bav::Parameter* paramToUse,
                         juce::ValueTree& treeToUse,
                         ParameterID parameterID)
        : ParameterToValueTreeAttachment (paramToUse, treeToUse, parameterID),
          ValueTreeToParameterAttachment (paramToUse, treeToUse, parameterID)
    {
        jassert (paramToUse != nullptr);
    }
};


}  // namespace
