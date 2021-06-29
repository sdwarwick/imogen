#pragma once

namespace Imogen
{

class AboutPopup : public gui::PopupComponent
{
    using PopupComponent::PopupComponent;
    
    void resizeTriggered() final;
};

}
