
namespace Imogen
{

class OutputLevelThumb : public juce::Component
{
public:
    OutputLevelThumb (Parameters& paramsToUse);
    
private:
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    Parameters& params;
    
    plugin::GainParameter& outputGain {*params.outputGain};
};

}
