#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "BinaryData.h"


namespace bav

{
    
class Imogen_touchOnceAndForgetSettings : public juce::PreferencesPanel
{
public:
    
    Imogen_touchOnceAndForgetSettings()
    {
        
    }
    
    
    juce::Component* createComponentForPage (const juce::String &pageName) override
    {
        if (pageName.equalsIgnoreCase ("General"))
            return new GeneralSettingsPageComponent;

        if (pageName.equalsIgnoreCase ("MIDI"))
            return new MidiSettingsPageComponent;
        
        jassert (pageName.equalsIgnoreCase ("Pitch"));

        return new PitchSettingsPageComponent;
    }
    
    
    /* A subclass that defines the component that displays the general settings page
    */
    class GeneralSettingsPageComponent : public juce::Component
    {
    public:
        
        GeneralSettingsPageComponent()
        {
            
        }
        
        void paint (juce::Graphics&) override
        {
            
        }
        
        void resized() override
        {
            
        }
        
    private:
        
    };
    
    
    /* A subclass that defines the component that displays the midi settings page
     */
    class MidiSettingsPageComponent : public juce::Component
    {
    public:
        
        MidiSettingsPageComponent()
        {
            
        }
        
        void paint (juce::Graphics&) override
        {
            
        }
        
        void resized() override
        {
            
        }
        
    private:
        
    };
    
    
    /* A subclass that defines the component that displays the pitch settings page
     */
    class PitchSettingsPageComponent : public juce::Component
    {
    public:
        
        PitchSettingsPageComponent()
        {
            
        }
        
        void paint (juce::Graphics&) override
        {
            
        }
        
        void resized() override
        {
            
        }
        
    private:
        
    };
    
    
private:
    
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Imogen_touchOnceAndForgetSettings)
    
};
    
    
    
    
class TouchOnceAndForgetSettingsComponent : public juce::Component
{
public:
    TouchOnceAndForgetSettingsComponent(): closeIcon(juce::ImageCache::getFromMemory(BinaryData::closeIcon_png, BinaryData::closeIcon_pngSize))
    {
        addAndMakeVisible(preferencePanelComponent);
        
        //preferencePanelComponent.addSettingsPage ("General", const void* imageData, int imageDataSize);
        //preferencePanelComponent.addSettingsPage ("MIDI", const void* imageData, int imageDataSize);
        //preferencePanelComponent.addSettingsPage ("Pitch", const void* imageData, int imageDataSize);
        
        closeButton.setImages(true, true, true, closeIcon, 1.0f, juce::Colours::transparentBlack, closeIcon, 1.0f, juce::Colours::transparentWhite, closeIcon, 1.0f, juce::Colours::transparentWhite);
        addAndMakeVisible(closeButton);
        closeButton.onClick = [this] { this->setVisible(false); };
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        
        g.setColour (juce::Colours::black);
        g.drawRect (getLocalBounds(), 1);
    }
    
    void resized() override
    {
        closeButton.setBounds(5, 5, 25, 25);
        
        auto bounds = getLocalBounds();
        preferencePanelComponent.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
    }
    
    
private:
    
    juce::Image closeIcon;
    juce::ImageButton closeButton;
    
    
    Imogen_touchOnceAndForgetSettings preferencePanelComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TouchOnceAndForgetSettingsComponent)
};
    
    
}  // namespace 
