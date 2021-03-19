
/*
    This class attaches to an ImogenAudioProcessor and reports current or default parameter values
*/


class ImogenParameterFetcher
{
    using ids = ImogenAudioProcessor::parameterID;
    
public:
    ImogenParameterFetcher(ImogenAudioProcessor& p): processor(p)
    { }
    
    // returns the raw parameter value as a float, in the normalized range 0 to 1
    float getRaw (ids paramID) const
    {
        return processor.getCurrentParameterValue (paramID);
    }
    
    // returns a float parameter in its natural range
    float getFloat (ids paramID) const
    {
        return scaleToFloat (paramID, processor.getCurrentParameterValue (paramID));
    }
    
    // returns an int parameter in its natural range
    int getInt (ids paramID) const
    {
        return scaleToInt (paramID, processor.getCurrentParameterValue (paramID));
    }
    
    // returns a boolean parameter as a natural true or false value
    bool getBool (ids paramID) const
    {
        return (processor.getCurrentParameterValue (paramID) >= 0.5f);
    }
    
    
    // returns a parameter's normalized default value
    float getDefaultRaw (ids paramID) const
    {
        return processor.getDefaultParameterValue (paramID);
    }
    
    // returns a float parameter's default value in its natural range
    float getDefaultFloat (ids paramID) const
    {
        return scaleToFloat (paramID, processor.getDefaultParameterValue (paramID));
    }
    
    // returns an int parameter's default value in its natural range
    int getDefaultInt (ids paramID) const
    {
        return scaleToInt (paramID, processor.getDefaultParameterValue (paramID));
    }
    
    // returns a boolean parameter as a natural true or false value
    bool getDefaultBool (ids paramID) const
    {
        return (processor.getDefaultParameterValue(paramID) >= 0.5f);
    }
    
    
    // takes a parameter ID and a normalized float value as input, and returns a float value in the parameter's natural range
    float scaleToFloat (ids paramID, float value) const
    {
        return processor.getParameterRange(paramID).convertFrom0to1 (value);
    }
    
    // takes a parameter ID and a normalized float and returns an integer value in the parameter's natural range
    int scaleToInt (ids paramID, float value) const
    {
        return juce::roundToInt (scaleToFloat (paramID, value));
    }
    
    
private:
    ImogenAudioProcessor& processor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImogenParameterFetcher)
};
