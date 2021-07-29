#pragma once

namespace Imogen
{
struct MidiState
{
    MidiState (plugin::ParameterList& list);

    IntParam pitchbendRange {0, 12, 2, "Pitchbend range", st_stringFromInt, st_intFromString, TRANS ("st")};

    PercentParam velocitySens {"Velocity amount", 100};

    ToggleParam aftertouchToggle {"Aftertouch gain", true};

    ToggleParam voiceStealing {"Voice stealing", false};

    ToggleParam midiLatch {"MIDI latch", false};

    ToggleParam pitchGlide {"Pitch glide toggle", false};

    FloatParam glideTime {0.f, 1.f, 0.4f, "Pitch glide time", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};

    FloatParam adsrAttack {0.f, 1.f, 0.35f, "ADSR attack", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};

    FloatParam adsrDecay {0.0f, 1.0f, 0.06f, "ADSR decay", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};

    PercentParam adsrSustain {"ADSR sustain", 80};

    FloatParam adsrRelease {0.f, 1.f, 0.1f, "ADSR release", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};

    ToggleParam pedalToggle {"Pedal toggle", false};
    IntParam    pedalThresh {0, 127, 0, "Pedal thresh", pitch_stringFromInt, pitch_intFromString};
    IntParam    pedalInterval {1, 12, 12, "Pedal interval", st_stringFromInt, st_intFromString};

    ToggleParam descantToggle {"Descant toggle", false};
    IntParam    descantThresh {0, 127, 127, "Descant thresh", pcnt_stringFromInt, pitch_intFromString};
    IntParam    descantInterval {1, 12, 12, "Descant interval", st_stringFromInt, st_intFromString};

    IntParam editorPitchbend {0, 127, 64, "GUI Pitchbend",
                              [] (int value, int maximumStringLength)
                              { return juce::String (value).substring (0, maximumStringLength); },
                              [] (const juce::String& text)
                              { return text.retainCharacters ("1234567890").getIntValue(); }};
};

}  // namespace Imogen
