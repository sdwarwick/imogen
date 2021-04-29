
#include <juce_osc/juce_osc.h>


namespace Imogen::OSC
{


static inline juce::String getParameterChangeOSCaddress()
{
    return { "/imogen/param/" };
}

static inline juce::String getParamGestureStartOSCaddress()
{
    return { "/imogen/gestureStart/" };
}

static inline juce::String getParamGestureEndOSCaddress()
{
    return { "/imogen/gestureEnd/" };
}


}  // namespace OSC


#include "OSC_sender.h"
#include "OSC_reciever.h"
