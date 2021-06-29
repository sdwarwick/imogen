
#pragma once

namespace Imogen
{

class OutputLevelMeter : public juce::Component
{
public:
    OutputLevelMeter (Meters& metersToUse);
    
private:
    struct Bar : juce::Component
    {
        Bar (GainMeterParameter& meter);
        
    private:
        void paint (juce::Graphics& g) final;
        void resized() final;
        
        GainMeterParameter& level;
    };
    
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    Meters& meters;
    
    Bar left {*meters.outputLevelL.get()};
    Bar right {*meters.outputLevelR.get()};
};

}
