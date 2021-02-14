/*
    Part of module: bv_GeneralUtils
    Parent file: bv_GeneralUtils.h
*/


#include "bv_GeneralUtils/bv_GeneralUtils.h"


namespace bav

{
    

class Panner
{
public:
    Panner(): lastRecievedMidiPan(64), leftGain(0.5f), rightGain(0.5f), prevLeftGain(0.5f), prevRightGain(0.5f)
    { }
    
    ~Panner()
    { }
    
    int getLastMidiPan() const noexcept { return lastRecievedMidiPan.load(); }
    
    float getLeftGain()  const noexcept { return leftGain.load(); }
    
    float getRightGain() const noexcept { return rightGain.load(); }
    
    float getPrevLeftGain()  const noexcept { return prevLeftGain.load(); }
    
    float getPrevRightGain() const noexcept { return prevRightGain.load(); }
    
    
    float getGainMult (const int chan) const
    {
        jassert (chan == 0 || chan == 1);
        
        if (chan == 0)
            return leftGain.load();
        
        return rightGain.load();
    }
    
    float getPrevGain (const int chan) const
    {
        jassert (chan == 0 || chan == 1);
        
        if (chan == 0)
            return prevLeftGain.load();
        
        return prevRightGain.load();
    }
    
    
    void setMidiPan (int newMidiPan)
    {
        newMidiPan = juce::jlimit (0, 127, newMidiPan);
        
        if (lastRecievedMidiPan.load() == newMidiPan)
            return;
        
        prevLeftGain.store(leftGain.load());
        prevRightGain.store(rightGain.load());
        
        float panningAngle = (90.0f * newMidiPan / 127.0f * juce::MathConstants<float>::pi) / 180.0f;
        
        panningAngle = juce::jlimit (0.0f, 90.0f, panningAngle);
        
        const float left  = juce::jlimit (0.0f, 1.0f, std::sin (panningAngle));
        const float right = juce::jlimit (0.0f, 1.0f, std::cos (panningAngle));
        
        leftGain.store(left);
        rightGain.store(right);
        
        lastRecievedMidiPan.store(newMidiPan);
    }
    
    
private:
    
    std::atomic<int> lastRecievedMidiPan;
    
    std::atomic<float> leftGain;
    std::atomic<float> rightGain;
    
    std::atomic<float> prevLeftGain, prevRightGain;
};

}; // namespace
