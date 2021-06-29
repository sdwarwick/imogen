#pragma once

namespace Imogen
{

class ScaleChooser : public juce::Component
{
public:
    ScaleChooser (Internals& internalsToUse);
    
private:
    Internals& internals;
    
    StringProperty& scaleName {internals.mtsEspScaleName};
};

}
