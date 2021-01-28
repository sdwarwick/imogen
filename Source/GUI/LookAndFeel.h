/*
 ==============================================================================
 
 LookAndFeel.h
 Created: 9 Dec 2020 7:07:45pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

class ImogenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ImogenLookAndFeel()
    {
        // so far these colors are consistent across all the themes:
        
        // rotary sliders
        this->setColour(Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::black);
        
        // labels
        this->setColour(Label::ColourIds::textColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(uiColourIds::toggleButtonColourId, 				juce::Colours::black);
        
        
        // numbox slider fill
        this->setColour(uiColourIds::numboxSliderFill,                  juce::Colours::grey);
    }
    
    
    enum Skin { CasualDenim, PlayfulBoho, ChicEveningwear, design4 };
    
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           Slider&) override
    {
        const auto radius  = (float) juce::jmin (width / 2, height / 2) - 5.0f;
        const auto centreX = (float) x + (float) width  * 0.5f;
        const auto centreY = (float) y + (float) height * 0.5f;
        const auto rx = centreX - radius;
        const auto ry = centreY - radius;
        const auto rw = radius * 2.0f;
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // fill
        g.setColour (this->findColour(Slider::ColourIds::rotarySliderFillColourId));
        g.fillEllipse (rx, ry, rw, rw);
        
        // outline
        g.setColour (this->findColour(Slider::ColourIds::rotarySliderOutlineColourId));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        
        // pointer
        juce::Path p;
        const auto pointerLength = radius * 0.75f;
        const auto pointerThickness = 3.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        
        g.setColour (this->findColour(Slider::ColourIds::thumbColourId));
        g.fillPath (p);
    }
    
    
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle style, Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour (this->findColour(uiColourIds::numboxSliderFill));
            g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                        : Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));
        }
        else
        {
            auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);
            
            auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);
            
            Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                     slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));
            
            Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                                   slider.isHorizontal() ? startPoint.y : (float) y);
            
            Path backgroundTrack;
            backgroundTrack.startNewSubPath (startPoint);
            backgroundTrack.lineTo (endPoint);
            g.setColour (slider.findColour (Slider::backgroundColourId));
            g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
            
            Path valueTrack;
            Point<float> minPoint, maxPoint, thumbPoint;
            
            if (isTwoVal || isThreeVal)
            {
                minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                    slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };
                
                if (isThreeVal)
                    thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                        slider.isHorizontal() ? (float) height * 0.5f : sliderPos };
                
                maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                    slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
                auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;
                
                minPoint = startPoint;
                maxPoint = { kx, ky };
            }
            
            auto thumbWidth = getSliderThumbRadius (slider);
            
            valueTrack.startNewSubPath (minPoint);
            valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
            g.setColour (this->findColour (Slider::trackColourId));
            g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
            
            if (! isTwoVal)
            {
                g.setColour (this->findColour(Slider::ColourIds::thumbColourId));
                g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
            }
            
            if (isTwoVal || isThreeVal)
            {
                auto sr = jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
                auto pointerColour = this->findColour(Slider::ColourIds::thumbColourId);
                
                if (slider.isHorizontal())
                {
                    drawPointer (g, minSliderPos - sr,
                                 jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                                 trackWidth * 2.0f, pointerColour, 2);
                    
                    drawPointer (g, maxSliderPos - trackWidth,
                                 jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                                 trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f),
                                 minSliderPos - trackWidth,
                                 trackWidth * 2.0f, pointerColour, 1);
                    
                    drawPointer (g, jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr,
                                 trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }
    
    
    void drawToggleButton (Graphics& g, ToggleButton& b, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        ignoreUnused (shouldDrawButtonAsDown, shouldDrawButtonAsHighlighted);
        
        const auto fontSize  = jmin (15.0f, (float) b.getHeight() * 0.75f);
        const auto tickWidth = fontSize * 1.1f;
        
        drawCheckbox(g, 4.0f, ((float) b.getHeight() - tickWidth) * 0.5f,
                     tickWidth, tickWidth, b.getToggleState());
        
        g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
        g.setFont (fontSize);
        
        if (! b.isEnabled())
            g.setOpacity (0.5f);
        
        g.drawFittedText (b.getButtonText(),
                          b.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                          .withTrimmedRight (2),
                          Justification::centredLeft, 10);
    }
    
    void drawCheckbox(Graphics& g, float x, float y, float w, float h, bool ticked) // draws tick box for toggleButtons
    {
        Rectangle<float> tickBounds (x, y, w, h);
        
        g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
        g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);
        
        if (ticked)
        {
            g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
            const auto tick = getTickShape (0.75f);
            g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
        }
    }
    
    
    void changeSkin(Skin& newskin)
    {
        switch(newskin)
        {
            case(CasualDenim):
                CasualDenimSkin();
                break;
                
            case(PlayfulBoho):
                PlayfulBohoSkin();
                break;
                
            case(ChicEveningwear):
                ChicEveningwearSkin();
                break;
                
            case(design4):
                goDesign4();
                break;
        }
    }
    
    
    void initializeLabel(Label& l, String labelText)
    {
        l.setFont(juce::Font(14.0f, juce::Font::bold));
        l.setJustificationType(juce::Justification::centred);
        l.setText(labelText, juce::dontSendNotification);
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
    
    void CasualDenimSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::dimgrey);
        
        // rotary sliders
        this->setColour(Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::royalblue);
        this->setColour(Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(TextButton::buttonColourId,						juce::Colours::cadetblue);
        
        // comboboxes
        this->setColour(ComboBox::ColourIds::backgroundColourId,		juce::Colours::cadetblue);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::burlywood);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::steelblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::oldlace);
    }
    
    
    void PlayfulBohoSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::tomato);
        
        // rotary sliders
        this->setColour(Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::indianred);
        this->setColour(Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(TextButton::buttonColourId,						juce::Colours::slategrey);
        
        // comboboxes
        this->setColour(ComboBox::ColourIds::backgroundColourId,		juce::Colours::slategrey);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::rosybrown);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::slateblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::navajowhite);
    }
    
    
    void ChicEveningwearSkin()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::black);
        
        // rotary sliders
        this->setColour(Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::darkred);
        this->setColour(Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(TextButton::buttonColourId,						juce::Colours::black);
        
        // comboboxes
        this->setColour(ComboBox::ColourIds::backgroundColourId,		juce::Colours::black);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::blueviolet);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::darkslateblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::snow);
    }
    
    
    void goDesign4()
    {
        // GUI background
        this->setColour(uiColourIds::blankCanvasColourId,				juce::Colours::dimgrey);
        
        // rotary sliders
        this->setColour(Slider::ColourIds::rotarySliderFillColourId, 	juce::Colours::darkred);
        this->setColour(Slider::ColourIds::thumbColourId, 				juce::Colours::black);
        
        // buttons
        this->setColour(TextButton::buttonColourId,						juce::Colours::black);
        
        // gui panels
        this->setColour(uiColourIds::backgroundPanelColourId, 			juce::Colours::lightcyan);
        this->setColour(uiColourIds::insetPanelColourId, 				juce::Colours::darkslateblue);
        
        // staff display
        this->setColour(uiColourIds::staffDisplayBackgroundColourId,	juce::Colours::snow);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImogenLookAndFeel)
};
