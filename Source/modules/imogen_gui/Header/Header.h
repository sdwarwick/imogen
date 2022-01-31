#pragma once

#include "InputIcon.h"
#include "OutputLevel/OutputLevel.h"
#include "ScaleChooser.h"
#include "AboutPopup/LogoButton.h"
#include "MidiSettings/KeyboardButton.h"

namespace Imogen
{
class Header : public juce::Component
{
public:

	Header (State& stateToUse);

private:

	void paint (juce::Graphics& g) final;
	void resized() final;

	State& state;

	LogoButton logo;

	KeyboardButton keyboardButton;

	InputIcon	inputIcon { state };
	OutputLevel outputLevel { state };

	// plugin::PresetBar presetBar {state, "Imogen", ".imogenpreset"};

	ScaleChooser scale { state.internals };
};

}  // namespace Imogen
