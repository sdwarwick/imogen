#pragma once

namespace Imogen
{
class MidiSettingsPopup : public gui::PopupComponent
{
public:

private:

	void paint (juce::Graphics& g) final;
	void resizeTriggered() final;
};

}  // namespace Imogen
