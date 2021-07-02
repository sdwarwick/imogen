
#pragma once

#if 0

 BEGIN_JUCE_MODULE_DECLARATION

 ID:                 imogen_state
 vendor:             Ben Vining
 version:            0.0.1
 name:               imogen_state
 description:        Imogen's shared state
 dependencies:       bv_plugin bv_networking

 END_JUCE_MODULE_DECLARATION

#endif


#include <bv_plugin/bv_plugin.h>
#include <bv_networking/bv_networking.h>

namespace Imogen
{
using namespace bav;
}

#include "state/State.h"
