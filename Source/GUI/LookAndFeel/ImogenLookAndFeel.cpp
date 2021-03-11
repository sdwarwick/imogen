
#include "ImogenLookAndFeel.h"


namespace bav
{
    
    ImogenLookAndFeel::ImogenLookAndFeel()
    {
        // rotary sliders
        this->setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::black);
        
        // labels
        this->setColour(juce::Label::ColourIds::textColourId, juce::Colours::black);
        
        // buttons
        this->setColour(uiColourIds::toggleButtonColourId, juce::Colours::black);
    
        // numbox slider fill
        this->setColour(uiColourIds::numboxSliderFill, juce::Colours::grey);
    }
    
    void ImogenLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos,
                                              const float rotaryStartAngle,
                                              const float rotaryEndAngle,
                                              juce::Slider&)
    {
        const auto radius  = (float) juce::jmin (width / 2, height / 2) - 5.0f;
        const auto centreX = (float) x + (float) width  * 0.5f;
        const auto centreY = (float) y + (float) height * 0.5f;
        const auto rx = centreX - radius;
        const auto ry = centreY - radius;
        const auto rw = radius * 2.0f;
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // fill
        g.setColour (this->findColour(juce::Slider::ColourIds::rotarySliderFillColourId));
        g.fillEllipse (rx, ry, rw, rw);
        
        // outline
        g.setColour (this->findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        
        // pointer
        juce::Path p;
        const auto pointerLength = radius * 0.75f;
        const auto pointerThickness = 3.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        
        g.setColour (this->findColour(juce::Slider::ColourIds::thumbColourId));
        g.fillPath (p);
    }
    
    
    void ImogenLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos,
                                              float minSliderPos,
                                              float maxSliderPos,
                                              const juce::Slider::SliderStyle style, juce::Slider& slider)
    {
        if (slider.isBar())
        {
            g.setColour (this->findColour(uiColourIds::numboxSliderFill));
            g.fillRect (slider.isHorizontal() ? juce::Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                        : juce::Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));
        }
        else
        {
            auto isTwoVal   = (style == juce::Slider::SliderStyle::TwoValueVertical   || style == juce::Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical || style == juce::Slider::SliderStyle::ThreeValueHorizontal);
            
            auto trackWidth = juce::jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);
            
            juce::Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                           slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));
            
            juce::Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                                         slider.isHorizontal() ? startPoint.y : (float) y);
            
            juce::Path backgroundTrack;
            backgroundTrack.startNewSubPath (startPoint);
            backgroundTrack.lineTo (endPoint);
            g.setColour (slider.findColour (juce::Slider::backgroundColourId));
            g.strokePath (backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
            
            juce::Path valueTrack;
            juce::Point<float> minPoint, maxPoint, thumbPoint;
            
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
            g.setColour (this->findColour (juce::Slider::trackColourId));
            g.strokePath (valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
            
            if (! isTwoVal)
            {
                g.setColour (this->findColour(juce::Slider::ColourIds::thumbColourId));
                g.fillEllipse (juce::Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
            }
            
            if (isTwoVal || isThreeVal)
            {
                auto sr = juce::jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
                auto pointerColour = this->findColour(juce::Slider::ColourIds::thumbColourId);
                
                if (slider.isHorizontal())
                {
                    drawPointer (g, minSliderPos - sr,
                                 juce::jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f),
                                 trackWidth * 2.0f, pointerColour, 2);
                    
                    drawPointer (g, maxSliderPos - trackWidth,
                                 juce::jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f),
                                 trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer (g, juce::jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f),
                                 minSliderPos - trackWidth,
                                 trackWidth * 2.0f, pointerColour, 1);
                    
                    drawPointer (g, juce::jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr,
                                 trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }
    
    
    void ImogenLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
    {
        juce::ignoreUnused (shouldDrawButtonAsDown, shouldDrawButtonAsHighlighted);
        
        const auto fontSize  = juce::jmin (15.0f, (float) b.getHeight() * 0.75f);
        const auto tickWidth = fontSize * 1.1f;
        
        drawCheckbox(g, 4.0f, ((float) b.getHeight() - tickWidth) * 0.5f,
                     tickWidth, tickWidth, b.getToggleState());
        
        g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
        g.setFont (fontSize);
        
        if (! b.isEnabled())
            g.setOpacity (0.5f);
        
        g.drawFittedText (b.getButtonText(),
                          b.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10)
                          .withTrimmedRight (2),
                          juce::Justification::centredLeft, 10);
    }
    
    void ImogenLookAndFeel::drawCheckbox (juce::Graphics& g, float x, float y, float w, float h, bool ticked) 
    {
        juce::Rectangle<float> tickBounds (x, y, w, h);
        
        g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
        g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);
        
        if (ticked)
        {
            g.setColour (this->findColour(uiColourIds::toggleButtonColourId));
            const auto tick = getTickShape (0.75f);
            g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
        }
    }
    
    
}  // namespace bav
