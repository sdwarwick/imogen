/*
  ==============================================================================

    LookAndFeel.h
    Created: 9 Dec 2020 7:07:45pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"

class ImogenLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ImogenLookAndFeel()
	{
		
	};
	
	~ImogenLookAndFeel()
	{
	};
	
	void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
						   const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override 
	{
		const auto radius = (float) juce::jmin (width / 2, height / 2) - 5.0f;
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
	};
	
};
