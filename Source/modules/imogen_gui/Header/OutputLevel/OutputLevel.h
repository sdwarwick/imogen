#pragma once

#include "LevelMeter.h"
#include "Thumb.h"

namespace Imogen
{

class OutputLevel : public juce::Component
{
public:
    OutputLevel (State& stateToUse);
    
private:
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    State& state;
    
    OutputLevelMeter meter {state.meters};
    OutputLevelThumb thumb {state.parameters};
};

}
