
#pragma once


namespace Imogen
{
struct Meters : plugin::ParameterList
{
    Meters();

    GainMeter inputLevel {"Input level", "Input level", inputMeter};

    GainMeter outputLevelL {"OutL", "Output level (L)", outputMeter};
    GainMeter outputLevelR {"OutR", "Output level (R)", outputMeter};

    GainMeter gateRedux {"Gate redux", "Noise gate gain reduction", compLimMeter};
    GainMeter compRedux {"Comp redux", "Compressor gain reduction", compLimMeter};
    GainMeter deEssRedux {"D-S redux", "De-esser gain reduction", compLimMeter};
    GainMeter limRedux {"Lim redux", "Limiter gain reduction", compLimMeter};

    GainMeter reverbLevel {"Reverb level", "Reverb level", otherMeter};
    GainMeter delayLevel {"Delay level", "Delay level", otherMeter};

private:
    static constexpr auto inputMeter   = juce::AudioProcessorParameter::inputMeter;
    static constexpr auto outputMeter  = juce::AudioProcessorParameter::outputMeter;
    static constexpr auto compLimMeter = juce::AudioProcessorParameter::compressorLimiterGainReductionMeter;
    static constexpr auto otherMeter   = juce::AudioProcessorParameter::otherMeter;
};

}  // namespace Imogen
