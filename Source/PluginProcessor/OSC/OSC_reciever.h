
#include "GUI/ImogenGUI.h"


class ImogenProcessorOSCReciever :    public juce::OSCReceiver::Listener< juce::OSCReceiver::RealtimeCallback >
{
public:
    ImogenProcessorOSCReciever(ImogenEventReciever* p): processor(p) { jassert (processor != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
        
        /*
         Redirect OSC messages to call these methods of the processor:
         
         - parameterChangeRecieved (ParameterID paramID, float newValue)
         - parameterChangeGestureStarted (ParameterID paramID)
         - parameterChangeGestureEnded (ParameterID paramID)
         - presetNameChanged (const juce::String& newPresetName)
         - abletonLinkChange (bool isNowEnabled)
        */
    }
    
private:
    ImogenEventReciever* const processor;
};
