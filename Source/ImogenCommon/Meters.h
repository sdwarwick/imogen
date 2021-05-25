
#pragma once


namespace Imogen
{

struct Meters : bav::ParameterList
{
    using Gain  = bav::ParameterHolder< bav::GainMeterParameter >;
    using Param = juce::AudioProcessorParameter;
    
    Meters()
    : ParameterList ("ImogenMeters")
    {
        add (inputLevel, outputLevelL, outputLevelR, gateRedux, compRedux, deEssRedux, limRedux, reverbLevel, delayLevel);
    }
    
    Gain inputLevel {"In", "Input level", Param::inputMeter};
    
    Gain outputLevelL {"OutL", "Output level (L)", Param::outputMeter};
    Gain outputLevelR {"OutR", "Output level (R)", Param::outputMeter};
    
    Gain gateRedux {"Gate redux", "Noise gate gain reduction", compLimMeter};
    Gain compRedux {"Comp redux", "Compressor gain reduction", compLimMeter};
    Gain deEssRedux {"D-S redux", "De-esser gain reduction", compLimMeter};
    Gain limRedux {"Lim redux", "Limiter gain reduction", compLimMeter};
    
    Gain reverbLevel {"Reverb", "Reverb level", otherMeter};
    Gain delayLevel {"Delay", "Delay level", otherMeter};
    
private:
    static constexpr auto compLimMeter = Param::compressorLimiterGainReductionMeter;
    static constexpr auto otherMeter   = Param::otherMeter;
};

}  // namespace
