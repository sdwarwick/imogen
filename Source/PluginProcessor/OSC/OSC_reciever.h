
#include "GUI/ImogenGUI.h"


class ImogenGuiOSCReciever :    public juce::OSCReceiver::Listener< juce::OSCReceiver::MessageLoopCallback >
{
public:
    ImogenGuiOSCReciever(ImogenGUI* g): gui(g) { jassert (gui != nullptr); }
    
    void oscMessageReceived (const juce::OSCMessage& message) override final
    {
        juce::ignoreUnused (message);
        
        /*
         Redirect OSC messages to call these methods of the GUI:
         
         - parameterChangeRecieved (ParameterID paramID, float newValue)
         - parameterChangeGestureStarted (ParameterID paramID)
         - parameterChangeGestureEnded (ParameterID paramID)
         - updateParameterDefaults()
         - presetNameChanged (const juce::String& newPresetName)
         - mts_connectionChange (bool isNowConnected)
         - mts_scaleChange (const juce::String& newScaleName)
         - abletonLinkChange (bool isNowEnabled)
         - setDarkMode (bool shouldUseDarkMode) [?]
        */
    }
    
private:
    ImogenGUI* const gui;
};
