
#include "ImogenEngine.h"


#include "effects/PreHarmony/StereoReducer.cpp"
#include "effects/PreHarmony/InputGain.cpp"
#include "effects/PreHarmony/NoiseGate.cpp"

#include "Harmonizer/Harmonizer.cpp"
#include "Harmonizer/HarmonizerVoice.cpp"
#include "Harmonizer/Lead/LeadProcessor.cpp"
#include "Harmonizer/Lead/DryPanner.cpp"
#include "Harmonizer/Lead/PitchCorrector.cpp"

#include "effects/PostHarmony/EQ.cpp"
#include "effects/PostHarmony/Compressor.cpp"
#include "effects/PostHarmony/DeEsser.cpp"
#include "effects/PostHarmony/DryWetMixer.cpp"
#include "effects/PostHarmony/Delay.cpp"
#include "effects/PostHarmony/Reverb.cpp"
#include "effects/PostHarmony/OutputGain.cpp"
#include "effects/PostHarmony/Limiter.cpp"

#include "effects/EffectsManager.cpp"

#include "engine/Engine.cpp"
