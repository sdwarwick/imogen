
#pragma once

#include "ImogenCommon.h"


using namespace Imogen;


/* The interface used to communicate from the GUI to the processor */
struct ImogenGuiHandle
{
    virtual ~ImogenGuiHandle() = default;
    
    virtual void sendParameterChange (ParameterID paramID, float newValue) = 0;
    virtual void startParameterChangeGesture (ParameterID paramID) = 0;
    virtual void endParameterChangeGesture (ParameterID paramID) = 0;
    
    virtual void sendEditorPitchbend (int wheelValue) = 0;
    
    virtual void sendMidiLatch (bool shouldBeLatched) = 0;
    
    virtual void loadPreset   (const juce::String& presetName) = 0;
    virtual void savePreset   (const juce::String& presetName) = 0;
    virtual void deletePreset (const juce::String& presetName) = 0;
    
    virtual void enableAbletonLink (bool shouldBeEnabled) = 0;
};


/*
 */

class ImogenComponent;


/* A very simple representation of a parameter's current value, bound to a ParameterID, used for updating components */
class ImogenGUIParameter
{
public:
    ImogenGUIParameter(ParameterID idToUse, const juce::String& nameToDisplay): paramID(idToUse), value(0.0f)
    {
        jassert (! nameToDisplay.isEmpty());
        displayName = nameToDisplay;
        component = nullptr;
    }
    
    virtual ~ImogenGUIParameter() = default;
    
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void parameterValueChanged (float newValue) = 0;
    };
    
    void addListener (Listener* newListener) { listeners.add (newListener); }
    
    void removeListener (Listener* listener) { listeners.remove (listener); }
    
    void setComponent (ImogenComponent* c) { component = c; }
    
    ImogenComponent* getComponent() const noexcept { return component; }
    
    
    void setValue (float newValue, bool notifyListeners = true)
    {
        if (newValue != value.load())
        {
            value.store (newValue);
            
            if (notifyListeners)
                listeners.call ([newValue] (Listener& l) { l.parameterValueChanged (newValue); });
        }
    }
    
    float getValue() const noexcept { return value.load(); }
    
    ParameterID getID() const noexcept { return paramID; }
    
    juce::String getDisplayName() const noexcept { return displayName; }
    
private:
    const ParameterID paramID;
    juce::String displayName;
    std::atomic<float> value;
    juce::ListenerList<Listener> listeners;
    ImogenComponent* component;
};


/*
 */


class ImogenComponent   :       public juce::Component,
                                public ImogenGUIParameter::Listener
{
public:
    ImogenComponent(ImogenGuiHandle* h, ImogenGUIParameter* p)
        : holder(h), parameter(p)
    {
        jassert (holder != nullptr && parameter != nullptr);
        parameter->addListener (this);
        parameter->setComponent (this);
    }
    
    virtual ~ImogenComponent()
    {
        parameter->removeListener (this);
        
        if (parameter->getComponent() == this)
            parameter->setComponent (nullptr);
    }
    
    ImogenGUIParameter* getParameter() const noexcept { return parameter; }
    
    virtual void setDarkMode (bool shouldUseDarkMode) { juce::ignoreUnused (shouldUseDarkMode); }
    
protected:
    ImogenGuiHandle* const holder;
    ImogenGUIParameter* const parameter;
};
