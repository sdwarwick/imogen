#pragma once

namespace Imogen
{
class Remote : public juce::Component
{
public:
    Remote();
    ~Remote() override;

private:
    void paint (juce::Graphics&) final;
    void resized() final;

    plugin::PluginState< State > state;

    GUI gui {state};

    // network::OscDataSynchronizer dataSync {state};
};

}  // namespace Imogen
