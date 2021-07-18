
#include "imogen_dsp/imogen_dsp.h"

#ifndef IMOGEN_HEADLESS
#    define IMOGEN_HEADLESS 0
#endif

#if ! IMOGEN_HEADLESS
#    include <imogen_gui/imogen_gui.h>
#endif


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    using namespace Imogen;
    
#if IMOGEN_HEADLESS
    return new Processor();
#else
    return new plugin::ProcessorWithEditor<Processor, GUI>(1060, 640);
#endif
}

// it's automatic now!!
