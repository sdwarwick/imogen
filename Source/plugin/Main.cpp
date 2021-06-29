
#include "imogen_dsp/imogen_dsp.h"

#ifndef IMOGEN_HEADLESS
#    define IMOGEN_HEADLESS 0
#endif

#if ! IMOGEN_HEADLESS
#    include <imogen_gui/imogen_gui.h>
#endif


// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    using namespace Imogen;
    
#if IMOGEN_HEADLESS
    return new Processor();
#else
    return new dsp::ProcessorWithEditor< Processor, GUI, 450, 300 >();
#endif
}
