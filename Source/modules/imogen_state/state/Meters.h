
#pragma once


namespace Imogen
{
struct Meters : plugin::ParameterList
{
    Meters();

    GainMeter inputLevel {"Input level", inputMeter};

    GainMeter outputLevelL {"Output level (L)", outputMeter};
    GainMeter outputLevelR {"Output level (R)", outputMeter};

    GainMeter gateRedux {"Noise gate gain reduction", compLimMeter};
    GainMeter compRedux {"Compressor gain reduction", compLimMeter};
    GainMeter deEssRedux {"De-esser gain reduction", compLimMeter};
    GainMeter limRedux {"Limiter gain reduction", compLimMeter};

    GainMeter reverbLevel {"Reverb level", otherMeter};
    GainMeter delayLevel {"Delay level", otherMeter};

private:
    static constexpr auto inputMeter   = juce::AudioProcessorParameter::inputMeter;
    static constexpr auto outputMeter  = juce::AudioProcessorParameter::outputMeter;
    static constexpr auto compLimMeter = juce::AudioProcessorParameter::compressorLimiterGainReductionMeter;
    static constexpr auto otherMeter   = juce::AudioProcessorParameter::otherMeter;
};

}  // namespace Imogen
