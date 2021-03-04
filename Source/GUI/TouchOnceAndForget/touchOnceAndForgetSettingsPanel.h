#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "PitchSettingsPage.h"
#include "MidiSettingsPage.h"
#include "GeneralSettingsPage.h"


namespace bav

{
    
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
    
    
    juce::Component* createComponentForPage (const juce::String &pageName) override
    {
        if (pageName.equalsIgnoreCase ("General"))
            return new GeneralSettingsPageComponent;

        if (pageName.equalsIgnoreCase ("MIDI"))
            return new MidiSettingsPageComponent;

        return new PitchSettingsPageComponent;
    }
    
    
private:
    
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Imogen_touchOnceAndForgetSettings)
    
};
    
    
}  // namespace 
