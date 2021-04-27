
#pragma once

#include "ImogenCommon.h"


class ImogenOSCReciever :    public juce::OSCReceiver::Listener< juce::OSCReceiver::MessageLoopCallback >
{
public:
    ImogenOSCReciever(ImogenEventReciever* r): reciever(r) { jassert (reciever != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
        
        /*
         Redirect OSC messages to call these methods of the reciever:
         
         - parameterChangeRecieved (ParameterID paramID, float newValue)
         - parameterChangeGestureStarted (ParameterID paramID)
         - parameterChangeGestureEnded (ParameterID paramID)
         - presetNameChanged (const juce::String& newPresetName)
         - abletonLinkChange (bool isNowEnabled)
        */
    }
    
private:
    ImogenEventReciever* const reciever;
};
