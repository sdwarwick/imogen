/*
 Part of module: bv_SharedCode
 Parent file: bv_SharedCode.h
 */


namespace bav

{

namespace dsp
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
        switch (chan)
        {
            case 0:
                return leftGain.load();
                
            case 1:
                return rightGain.load();
                
            default:
                return 1.0f;
        }
    }
    
    float getPrevGain (const int chan) const
    {
        switch (chan)
        {
            case 0:
                return prevLeftGain.load();
                
            case 1:
                return prevRightGain.load();
                
            default:
                return 1.0f;
        }
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
        
        jassert (left >= 0.0f && left <= 1.0f);
        jassert (right >= 0.0f && right <= 1.0f);
        
        leftGain.store(left);
        rightGain.store(right);
        
        lastRecievedMidiPan.store(newMidiPan);
    }
    
    
private:
    
    std::atomic<int> lastRecievedMidiPan;
    
    std::atomic<float> leftGain;
    std::atomic<float> rightGain;
    
    std::atomic<float> prevLeftGain, prevRightGain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panner)
};
    
    
}  // namespace dsp

} // namespace bav

