
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
    ImogenOSCReciever() { }
    
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        const auto address = message.getAddressPattern();
        
        if (address.matches ({ OSC::getParameterChangeOSCaddress() }))
        {
            if (message.size() == 2)  // parameter change messages are expected to have 2 arguments
            {
                const auto arg1 = message[0];  // parameter ID
                const auto arg2 = message[1];  // new value
//
//                if (arg1.isInt32() && arg2.isFloat32())
//                    reciever->recieveParameterChange (ParameterID (arg1.getInt32()),
//                                                      juce::jlimit (0.0f, 1.0f, arg2.getFloat32()));
            }
        }
        else if (address.matches ({ OSC::getParamGestureStartOSCaddress() }))
        {
            if (message.size() == 1)  // parameter gesture messages are expected to have 1 argument
            {
                const auto arg = message[0];  // parameter ID
//
//                if (arg.isInt32())
//                    reciever->recieveParameterChangeGestureStart (ParameterID (arg.getInt32()));
            }
        }
        else if (address.matches ({ OSC::getParamGestureEndOSCaddress() }))
        {
            if (message.size() == 1)  // parameter gesture messages are expected to have 1 argument
            {
                const auto arg = message[0];  // parameter ID
//
//                if (arg.isInt32())
//                    reciever->recieveParameterChangeGestureEnd (ParameterID (arg.getInt32()));
            }
        }
    }
    
    ImogenOSCRecieverState getState() const
    {
        
    }
    
    void setState (const ImogenOSCRecieverState& state)
    {
        
    }
    
private:
};

template class ImogenOSCReciever<juce::OSCReceiver::MessageLoopCallback>;
template class ImogenOSCReciever<juce::OSCReceiver::RealtimeCallback>;
