#pragma once

namespace Imogen
{
struct AboutPopup : public gui::PopupComponent
{
	using PopupComponent::PopupComponent;

	void resizeTriggered() final;
};

}  // namespace Imogen
