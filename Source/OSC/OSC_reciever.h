
#pragma once

#include "ImogenCommon.h"

#include <juce_osc/juce_osc.h>


struct ImogenOSCRecieverState
{
    bool oscIsEnabled;
    int  portNumber;
};



template <typename CallbackType = juce::OSCReceiver::MessageLoopCallback>
class ImogenOSCReciever :    public juce::OSCReceiver::Listener< CallbackType >
{
public:
    ImogenOSCReciever(ImogenEventReciever* r): reciever(r) { jassert (reciever != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
    }
    
    ImogenOSCRecieverState getState() const
    {
        
    }
    
    void setState (const ImogenOSCRecieverState& state)
    {
        
    }
    
private:
    ImogenEventReciever* const reciever;
};

template class ImogenOSCReciever<juce::OSCReceiver::MessageLoopCallback>;
template class ImogenOSCReciever<juce::OSCReceiver::RealtimeCallback>;
