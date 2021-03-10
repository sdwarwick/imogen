/*
    This file defines a global set of "look and feel" characteristics for Imogen's GUI, including colors and drawing methods for individual components
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>


namespace bav

{


class ImogenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ImogenLookAndFeel();
    
    enum Skin { CasualDenim, PlayfulBoho, ChicEveningwear };
    
    void changeSkin (Skin& newskin);
    
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           juce::Slider&) override;
    
    
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawCheckbox (juce::Graphics& g, float x, float y, float w, float h, bool ticked);  // draws tick box for toggleButtons
    
    
    inline void initializeLabel (juce::Label& l, juce::String labelText)
    {
        l.setFont(juce::Font(14.0f, juce::Font::bold));
        l.setJustificationType(juce::Justification::centred);
        l.setText(labelText, juce::dontSendNotification);
    }
    
    
    template<typename valueType>
    inline void initializeSlider (juce::Slider& slider, juce::Slider::SliderStyle style, valueType initValue)
    {
        slider.setSliderStyle (style);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        slider.setValue (float(initValue), juce::NotificationType::dontSendNotification);
    }
    
    
    enum uiColourIds
    {
        backgroundPanelColourId,
        insetPanelColourId,
        blankCanvasColourId,
        staffDisplayBackgroundColourId,
        toggleButtonColourId,
        numboxSliderFill
    };
    
    
private:
    
    inline void CasualDenimSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::dimgrey);
        
        // rotary sliders
        this->setColour(juce::Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::royalblue);
        this->setColour(juce::Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(juce::TextButton::buttonColourId,						juce::Colours::cadetblue);
        
        // comboboxes
        this->setColour(juce::ComboBox::ColourIds::backgroundColourId,		juce::Colours::cadetblue);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::burlywood);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::steelblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::oldlace);
    }
    
    
    inline void PlayfulBohoSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::tomato);
        
        // rotary sliders
        this->setColour(juce::Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::indianred);
        this->setColour(juce::Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(juce::TextButton::buttonColourId,						juce::Colours::slategrey);
        
        // comboboxes
        this->setColour(juce::ComboBox::ColourIds::backgroundColourId,		juce::Colours::slategrey);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::rosybrown);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::slateblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::navajowhite);
    }
    
    
    inline void ChicEveningwearSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::black);
        
        // rotary sliders
        this->setColour(juce::Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::darkred);
        this->setColour(juce::Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(juce::TextButton::buttonColourId,						juce::Colours::black);
        
        // comboboxes
        this->setColour(juce::ComboBox::ColourIds::backgroundColourId,		juce::Colours::black);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::blueviolet);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::darkslateblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::snow);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImogenLookAndFeel)
};


}  // namespace
