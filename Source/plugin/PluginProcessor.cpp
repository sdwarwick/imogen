
#include "imogen_dsp/imogen_dsp.h"

#ifndef IMOGEN_HEADLESS
#    define IMOGEN_HEADLESS 0
#endif

#if ! IMOGEN_HEADLESS
#    include <ImogenGUI/ImogenGUI.h>

namespace Imogen
{

struct Plugin : Processor
{
    bool hasEditor() const final { return true; }
    
    juce::AudioProcessorEditor* createEditor() final
    {
        return new gui::PluginEditor<GUI> (*this, {450, 300}, state);
    }
};

}  // namespace Imogen

#endif


// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
#if IMOGEN_HEADLESS
    return new Imogen::Processor();
#else
    return new Imogen::Plugin();
#endif
}
