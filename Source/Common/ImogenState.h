

#pragma once

#include "ImogenParameters.h"


class ImogenState
{
public:
    ImogenState()
    {
        for (int i = 0; i < numParams; ++i)
        {
            const auto paramID = static_cast<ParameterID> (i);
            
            const auto paramName = getParameterIdentifier (paramID);
            
            state.setProperty (paramName, 0, nullptr);
            state.setProperty (paramName + ".isChanging", false, nullptr);
        }
    }
    
    
    void storeParameterValue (ParameterID paramID, float newValue)
    {
        
    }
    
    void storeParameterGesture (ParameterID paramID, bool isChanging)
    {
        
    }
    
    
    juce::ValueTree state;
    
    
private:
    
};
