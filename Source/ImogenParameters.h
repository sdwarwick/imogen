

#include "ImogenCommon.h"


struct ImogenFloatParameter  :    public bav::FloatParameter
{
    ImogenFloatParameter (ParameterID paramtrID, juce::NormalisableRange<float>& nrange,
                          float defaultVal, juce::String parameterLabel = juce::String(),
                          juce::AudioProcessorParameter::Category parameterCategory = juce::AudioProcessorParameter::genericParameter,
                          std::function<juce::String(float value, int maximumStringLength)> stringFromValue = nullptr,
                          std::function<float(const juce::String& text)> valueFromString = nullptr)
    
    : bav::FloatParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                           nrange, defaultVal, parameterLabel, parameterCategory, stringFromValue, valueFromString)
    { }
};



struct ImogenIntParameter    :    public bav::IntParameter
{
    ImogenIntParameter (ParameterID paramtrID, int min, int max, int defaultVal,
                        juce::String parameterLabel = juce::String(),
                        std::function<juce::String(int value, int maximumStringLength)> stringFromInt = nullptr,
                        std::function<int(const juce::String& text)> intFromString = nullptr)
    
    : bav::IntParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                         min, max, defaultVal, parameterLabel, stringFromInt, intFromString)
    { }
};



struct ImogenBoolParameter   :    public bav::BoolParameter
{
    ImogenBoolParameter (ParameterID paramtrID, bool defaultVal, juce::String parameterLabel = juce::String(),
                         std::function<juce::String(bool value, int maximumStringLength)> stringFromBool = nullptr,
                         std::function<bool(const juce::String& text)> boolFromString = nullptr)
    
    : bav::BoolParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                          defaultVal, parameterLabel, stringFromBool, boolFromString)
    { }
};




