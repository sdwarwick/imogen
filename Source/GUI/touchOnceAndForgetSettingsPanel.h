#pragma once

#include <juce_gui_extra/juce_gui_extra.h>


namespace bav

{
    
using namespace juce;
    
    
class Imogen_touchOnceAndForgetSettings : public juce::PreferencesPanel
{
public:
    
    Imogen_touchOnceAndForgetSettings()
    {
        // this->addSettingsPage (general, const void* imageData, int imageDataSize);
        // this->addSettingsPage (midi, const void* imageData, int imageDataSize);
        // this->addSettingsPage (juce::String("Pitch"), const void* imageData, int imageDataSize);
        // this->setCurrentPage (juce::String("General"));
    }
    
    Component* createComponentForPage (const String &pageName) override
    {
        if (pageName.equalsIgnoreCase (general))
            return createGeneralSettingsPage();

        if (pageName.equalsIgnoreCase (midi))
            return createMidiSettingsPage();

        return createPitchSettingsPage();
    }
    
    
private:
    
    Component* createGeneralSettingsPage()
    {

    }


    Component* createMidiSettingsPage()
    {

    }


    Component* createPitchSettingsPage()
    {

    }
    
    
    const String general = String("General");
    const String midi = String("MIDI");
    
    
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Imogen_touchOnceAndForgetSettings)
    
};
    
    
}  // namespace 
