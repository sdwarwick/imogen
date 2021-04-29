
#pragma once

#include "ImogenCommon.h"


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
        const auto address = message.getAddressPattern();
        
        if (address.matches ({ OSC::getParameterOSCaddress (inputSourceID) }))
        {
            const auto arg = message[0];
            
            if (arg.isFloat32())
                reciever->recieveParameterChange (inputSourceID, arg.getFloat32());
        }
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
