
#pragma once

#include "ImogenCommon.h"


struct ImogenParameterMetadata
{
    ImogenParameterMetadata(ParameterID paramID, const juce::String& parentGroupName = juce::String())
         :  parameterID(paramID),
            shortName (getParameterNameShort (paramID)),
            verboseName (getParameterNameVerbose (paramID)),
            identifier (getParameterIdentifier (paramID)),
            parentParameterGroupName (parentGroupName)
    { }
    
    const ParameterID      parameterID;
    const juce::String     shortName;
    const juce::String     verboseName;
    const juce::Identifier identifier;
    
    const juce::String parentParameterGroupName;
};



struct ImogenFloatParameter  :    public bav::FloatParameter
{
    ImogenFloatParameter (ParameterID paramtrID, juce::NormalisableRange<float>& nrange, float defaultVal,
                          juce::AudioProcessorParameter::Category parameterCategory = juce::AudioProcessorParameter::genericParameter,
                          std::function<juce::String(float value, int maximumStringLength)> stringFromValue = nullptr,
                          std::function<float(const juce::String& text)> valueFromString = nullptr)
    
    : bav::FloatParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                           nrange, defaultVal, juce::String(), parameterCategory, stringFromValue, valueFromString)
    { }
    
    
    ImogenFloatParameter (ImogenParameterMetadata metadata,
                          juce::NormalisableRange<float>& nrange, float defaultVal,
                          juce::AudioProcessorParameter::Category parameterCategory = juce::AudioProcessorParameter::genericParameter,
                          std::function<juce::String(float value, int maximumStringLength)> stringFromValue = nullptr,
                          std::function<float(const juce::String& text)> valueFromString = nullptr)
    
    : bav::FloatParameter (metadata.parameterID, metadata.shortName, metadata.verboseName, nrange, defaultVal, juce::String(),
                           parameterCategory, stringFromValue, valueFromString)
    { }
};



struct ImogenIntParameter    :    public bav::IntParameter
{
    ImogenIntParameter (ParameterID paramtrID, int min, int max, int defaultVal,
                        std::function<juce::String(int value, int maximumStringLength)> stringFromInt = nullptr,
                        std::function<int(const juce::String& text)> intFromString = nullptr)
    
    : bav::IntParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                         min, max, defaultVal, juce::String(), stringFromInt, intFromString)
    { }
    
    
    ImogenIntParameter (ImogenParameterMetadata metadata, int min, int max, int defaultVal,
                        std::function<juce::String(int value, int maximumStringLength)> stringFromInt = nullptr,
                        std::function<int(const juce::String& text)> intFromString = nullptr)
    
    : bav::IntParameter (metadata.parameterID, metadata.shortName, metadata.verboseName, min, max, defaultVal,
                         juce::String(), stringFromInt, intFromString)
    { }
};



struct ImogenBoolParameter   :    public bav::BoolParameter
{
    ImogenBoolParameter (ParameterID paramtrID, bool defaultVal,
                         std::function<juce::String(bool value, int maximumStringLength)> stringFromBool = nullptr,
                         std::function<bool(const juce::String& text)> boolFromString = nullptr)
    
    : bav::BoolParameter (paramtrID, getParameterIdentifier (paramtrID), getParameterNameVerbose (paramtrID),
                          defaultVal, juce::String(), stringFromBool, boolFromString)
    { }
    
    
    ImogenBoolParameter (ImogenParameterMetadata metadata, bool defaultVal,
                         std::function<juce::String(bool value, int maximumStringLength)> stringFromBool = nullptr,
                         std::function<bool(const juce::String& text)> boolFromString = nullptr)
    
    : bav::BoolParameter (metadata.parameterID, metadata.shortName, metadata.verboseName,
                          defaultVal, juce::String(), stringFromBool, boolFromString)
    { }
};

