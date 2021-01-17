/*
  ==============================================================================

    Panner.h
    Created: 17 Jan 2021 1:02:07pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class Panner
{
public:
    Panner(): lastRecievedMidiPan(64), leftGain(0.5f), rightGain(0.5f)
    { };
    
    ~Panner()
    { };
    
    float getLeftGain()  const noexcept { return leftGain; };
    
    float getRightGain() const noexcept { return rightGain; };
    
    void setMidiPan (const int newMidiPan)
    {
        jassert (isPositiveAndBelow (newMidiPan, 128));
        
        if (lastRecievedMidiPan == newMidiPan)
            return;
        
        // convert midiPan [0-127] first to an angle between 0 & 90 degrees, then to radians
        const float panningAngle = 90 * newMidiPan / 127 * MathConstants<float>::pi / 180;
        
        float left  = std::sin (panningAngle);
        float right = std::cos (panningAngle);
        
        if (left < 0.0f)  left = 0.0f;
        if (left > 1.0f)  left = 1.0f;
        if (right < 0.0f) right = 0.0f;
        if (right > 1.0f) right = 1.0f;
        
        lastRecievedMidiPan = newMidiPan;
    };
    
    
private:
    
    int lastRecievedMidiPan;
    
    float leftGain;
    float rightGain;
};
