/*
  ==============================================================================

    Panner.h
    Created: 17 Jan 2021 1:02:07pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "bv_GeneralUtils/bv_GeneralUtils.h"

class Panner
{
public:
    Panner(): lastRecievedMidiPan(64), leftGain(0.5f), rightGain(0.5f), prevLeftGain(0.5f), prevRightGain(0.5f)
    { }
    
    ~Panner()
    { }
    
    int getLastMidiPan() const noexcept { return lastRecievedMidiPan; }
    
    float getLeftGain()  const noexcept { return leftGain; }
    
    float getRightGain() const noexcept { return rightGain; }
    
    float getPrevLeftGain()  const noexcept { return prevLeftGain; }
    
    float getPrevRightGain() const noexcept { return prevRightGain; }
    
    
    float getGainMult (const int chan) const
    {
        jassert (chan == 0 || chan == 1);
        
        if (chan == 0)
            return leftGain;
        
        return rightGain;
    }
    
    float getPrevGain (const int chan) const
    {
        jassert (chan == 0 || chan == 1);
        
        if (chan == 0)
            return prevLeftGain;
        
        return prevRightGain;
    }
    
    
    void setMidiPan (const int newMidiPan)
    {
        jassert (juce::isPositiveAndBelow (newMidiPan, 128));
        
        if (lastRecievedMidiPan == newMidiPan)
            return;
        
        prevLeftGain  = leftGain;
        prevRightGain = rightGain;
        
        // convert midiPan [0-127] first to an angle between 0 & 90 degrees, then to radians
        
        const float panningAngle = (90.0f * newMidiPan / 127.0f * juce::MathConstants<float>::pi) / 180.0f;
        
        jassert (panningAngle >= 0.0f && panningAngle <= 90.0f);
        
        float left  = std::sin (panningAngle);
        float right = std::cos (panningAngle);
        
        if (left < 0.0f)  left  = 0.0f;
        if (left > 1.0f)  left  = 1.0f;
        if (right < 0.0f) right = 0.0f;
        if (right > 1.0f) right = 1.0f;
        
        lastRecievedMidiPan = newMidiPan;
    }
    
    
private:
    
    int lastRecievedMidiPan;
    
    float leftGain;
    float rightGain;
    
    float prevLeftGain, prevRightGain;
};
