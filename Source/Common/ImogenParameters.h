
#pragma once

#include "ImogenCommon.h"


struct ImogenParameterMetadata
{
    ImogenParameterMetadata (ParameterID paramID)
         :  parameterID(paramID),
            shortName (getParameterNameShort (paramID)),
            verboseName (getParameterNameVerbose (paramID)),
            identifier (getParameterIdentifier (paramID))
    { }
    
    const ParameterID      parameterID;
    const juce::String     shortName;
    const juce::String     verboseName;
    const juce::Identifier identifier;
    
    std::atomic<bool> isChanging;
};


struct ImogenFloatParameter :   ImogenParameterMetadata
{
    ImogenFloatParameter (ParameterID paramID,
                          const juce::NormalisableRange<float>& nRange,
                          float defaultVal,
                          std::function<juce::String(float value, int maximumStringLength)> stringfromfloat = nullptr,
                          std::function<float(const juce::String& text)> floatfromstring = nullptr)
        :  ImogenParameterMetadata(paramID), range(nRange), defaultValue(defaultVal),
           stringFromFloat(stringfromfloat), floatFromString(floatfromstring)
    {
        value.store (defaultValue);
    }
    
    std::atomic<float> value;
    juce::NormalisableRange<float> range;
    const float defaultValue;
    
    std::function<juce::String(float value, int maximumStringLength)> stringFromFloat = nullptr;
    std::function<float(const juce::String& text)> floatFromString = nullptr;
};


struct ImogenIntParameter   :   ImogenParameterMetadata
{
    ImogenIntParameter (ParameterID paramID,
                        int min, int max, int defaultVal,
                        std::function<juce::String(int value, int maximumStringLength)> stringfromint = nullptr,
                        std::function<int(const juce::String& text)> intfromstring = nullptr)
        :  ImogenParameterMetadata(paramID), minimum(min), maximum(max), defaultValue(defaultVal),
           stringFromInt(stringfromint), intFromString(intfromstring)
    {
        value.store (defaultValue);
    }
    
    std::atomic<int> value;
    const int minimum;
    const int maximum;
    const int defaultValue;
    
    std::function<juce::String(int value, int maximumStringLength)> stringFromInt = nullptr;
    std::function<int(const juce::String& text)> intFromString = nullptr;
};


struct ImogenBoolParameter  :   ImogenParameterMetadata
{
    ImogenBoolParameter(ParameterID paramID,
                        bool defaultVal,
                        std::function<juce::String(bool value, int maximumStringLength)> stringfrombool = nullptr,
                        std::function<bool(const juce::String& text)> boolfromstring = nullptr)
        :  ImogenParameterMetadata(paramID), defaultValue(defaultVal),
           stringFromBool(stringfrombool), boolFromString(boolfromstring)
    {
        value.store (defaultValue);
    }
    
    std::atomic<bool> value;
    const bool defaultValue;
    
    std::function<juce::String(bool value, int maximumStringLength)> stringFromBool = nullptr;
    std::function<bool(const juce::String& text)> boolFromString = nullptr;
};





class ImogenParameterOwner      :       public juce::ValueTreeSynchroniser  // this interface does not implement the virtual stateChanged()!
{
public:
    ImogenParameterOwner()
        : juce::ValueTreeSynchroniser (state)
    {
        
    }
    
    virtual ~ImogenParameterOwner() = default;
    
    virtual void refreshParameterFunctionPointers() = 0;
    
    bav::Parameter* getParameter (ParameterID paramID)
    {
        
    }
    
    
    void recieveStateUpdate (const void* encodedChangeData, size_t encodedChangeDataSize)
    {
        juce::ValueTreeSynchroniser::applyChange (state, encodedChangeData, encodedChangeDataSize, nullptr);
    }
    
    
    juce::ValueTree state;
    
private:
    std::vector<std::unique_ptr<bav::Parameter>> parameters;
};
