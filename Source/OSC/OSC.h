
#include <juce_osc/juce_osc.h>


namespace Imogen::OSC
{

static inline juce::String getParameterOSCaddress (ParameterID param)
{
    const auto temp = getParameterNameVerbose (param).replaceCharacters (" ", "/").trim();
    return (temp.endsWith ("/")) ? temp : temp + "/";
}

static inline juce::String getParameterChangeOSCaddress (ParameterID param)
{
    return { "/imogen/param/" + getParameterOSCaddress (param) };
}

static inline juce::String getParamGestureStartOSCaddress (ParameterID param)
{
    return { "/imogen/gestureStart/" + getParameterOSCaddress (param) };
}

static inline juce::String getParamGestureEndOSCaddress (ParameterID param)
{
    return { "/imogen/gestureEnd/" + getParameterOSCaddress (param) };
}

}  // namespace OSC



#include "OSC_sender.h"
#include "OSC_reciever.h"
