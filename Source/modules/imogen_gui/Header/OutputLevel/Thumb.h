
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
    
    GainParameter& outputGain {*params.outputGain.get()};
};

}
