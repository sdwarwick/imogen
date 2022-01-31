#pragma once

#include "AboutPopup.h"

namespace Imogen
{
class LogoButton : public juce::Component
{
public:

	LogoButton();

private:

	void resized() final;
	void createAboutWindow();

	gui::TextButton button { "Imogen", [&]
							 { createAboutWindow(); } };

	gui::Popup<AboutPopup> aboutWindow;
};

}  // namespace Imogen
