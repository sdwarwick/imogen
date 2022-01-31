
#include "imogen_dsp.h"


#include "Engine/effects/PreHarmony/StereoReducer.cpp"
#include "Engine/effects/PreHarmony/InputGain.cpp"
#include "Engine/effects/PreHarmony/NoiseGate.cpp"
#include "Engine/effects/PreHarmonyEffects.cpp"

#include "Engine/Harmonizer/Harmonizer.cpp"
#include "Engine/Harmonizer/HarmonizerVoice.cpp"

#include "Engine/Lead/LeadProcessor.cpp"
#include "Engine/Lead/DryPanner.cpp"
#include "Engine/Lead/PitchCorrector.cpp"

#include "Engine/effects/PostHarmony/EQ.cpp"
#include "Engine/effects/PostHarmony/Compressor.cpp"
#include "Engine/effects/PostHarmony/DeEsser.cpp"
#include "Engine/effects/PostHarmony/DryWetMixer.cpp"
#include "Engine/effects/PostHarmony/Delay.cpp"
#include "Engine/effects/PostHarmony/Reverb.cpp"
#include "Engine/effects/PostHarmony/OutputGain.cpp"
#include "Engine/effects/PostHarmony/Limiter.cpp"

#include "Engine/effects/PostHarmonyEffects.cpp"

#include "Engine/Engine.cpp"

#include "Processor/Processor.cpp"
