
#include <juce_osc/juce_osc.h>
 
#include "GUI/ImogenGUI.h"


class OSC_Parser  :     public juce::OSCReceiver::Listener< juce::OSCReceiver::MessageLoopCallback >
{
public:
    OSC_Parser(ImogenGuiHandle* h): handle(h) { jassert (handle != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
    }
    
    
private:
    ImogenGuiHandle* const handle;
};
