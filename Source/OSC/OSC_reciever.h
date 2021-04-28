
#pragma once

#include "ImogenCommon.h"


class ImogenOSCReciever :    public juce::OSCReceiver::Listener< juce::OSCReceiver::MessageLoopCallback >
{
public:
    ImogenOSCReciever(ImogenEventReciever* r): reciever(r) { jassert (reciever != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
    }
    
private:
    ImogenEventReciever* const reciever;
};
