
#include <imogen_gui/imogen_gui.h>
#include <lemons_app_utils/lemons_app_utils.h>

namespace Imogen
{
struct RemoteApp : lemons::GuiApp< Remote >
{
    RemoteApp()
        : lemons::GuiApp< Imogen::Remote > (String ("Imogen ") + TRANS ("Remote"), "0.0.1", {1060, 640})
    {
    }
};

}  // namespace Imogen

START_JUCE_APPLICATION (Imogen::RemoteApp)
