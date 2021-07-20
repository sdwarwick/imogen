#pragma once

namespace Imogen
{
class ScaleChooser : public juce::Component
{
public:
    ScaleChooser (Internals& internalsToUse);

private:
    Internals& internals;

    plugin::StringProperty& scaleName {internals.mtsEspScaleName};
};

}  // namespace Imogen
