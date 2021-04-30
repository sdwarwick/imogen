
#include "GUI/GUI_Framework.h"


struct ImogenOSCSenderState
{
    bool oscIsEnabled;
    int portNumber;
    //juce::OSCAddress address;
};



class ImogenOSCSender    :      public juce::OSCSender
{
public:
    ImogenOSCSender() = default;
    
//    ImogenOSCSenderState getState() const
//    {
//
//    }
//
//    void setState (const ImogenOSCSenderState& state)
//    {
//
//    }
    
    void sendParameterChange (ParameterID paramID, float newValue)
    {
        send (OSC::getParameterChangeOSCaddress(), int (paramID), newValue);
    }
    
    void sendParameterChangeGestureStart (ParameterID paramID)
    {
        send (OSC::getParamGestureStartOSCaddress(), int (paramID));
    }
    
    void sendParameterChangeGestureEnd (ParameterID paramID)
    {
        send (OSC::getParamGestureEndOSCaddress(), int (paramID));
    }
    
    void sendEditorPitchbend (int wheelValue)
    {
        juce::ignoreUnused (wheelValue);
    }
    
    void sendMidiLatch (bool shouldBeLatched)
    {
        juce::ignoreUnused (shouldBeLatched);
    }
    
    void sendKillAllMidiEvent()
    {
        
    }
    
    void sendLoadPreset   (const juce::String& presetName)
    {
        juce::ignoreUnused (presetName);
    }
    
    void sendSavePreset   (const juce::String& presetName)
    {
        juce::ignoreUnused (presetName);
    }
    
    void sendDeletePreset (const juce::String& presetName)
    {
        juce::ignoreUnused (presetName);
    }
    
    void sendEnableAbletonLink (bool shouldBeEnabled)
    {
        juce::ignoreUnused (shouldBeEnabled);
    }
    
private:
};
