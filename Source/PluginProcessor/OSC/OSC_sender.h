
#include <juce_osc/juce_osc.h>
 
#include "GUI/GUI_Framework.h"


class ImogenGuiOSCSender    :   public ImogenGuiHandle,
                                public juce::OSCSender
{
public:
    ImogenGuiOSCSender() = default;
    
    void sendParameterChange (ParameterID paramID, float newValue) override final
    {
        juce::ignoreUnused (paramID, newValue);
    }
    
    void startParameterChangeGesture (ParameterID paramID) override final
    {
        juce::ignoreUnused (paramID);
    }
    
    void endParameterChangeGesture (ParameterID paramID) override final
    {
        juce::ignoreUnused (paramID);
    }
    
    void sendEditorPitchbend (int wheelValue) override final
    {
        juce::ignoreUnused (wheelValue);
    }
    
    void sendMidiLatch (bool shouldBeLatched) override final
    {
        juce::ignoreUnused (shouldBeLatched);
    }
    
    void loadPreset   (const juce::String& presetName) override final
    {
        juce::ignoreUnused (presetName);
    }
    
    void savePreset   (const juce::String& presetName) override final
    {
        juce::ignoreUnused (presetName);
    }
    
    void deletePreset (const juce::String& presetName) override final
    {
        juce::ignoreUnused (presetName);
    }
    
    void enableAbletonLink (bool shouldBeEnabled) override final
    {
        juce::ignoreUnused (shouldBeEnabled);
    }
    
private:
};
