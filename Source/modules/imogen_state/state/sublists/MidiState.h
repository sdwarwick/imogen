#pragma once

namespace Imogen
{
struct MidiState
{
    MidiState (plugin::ParameterList& list);

    SemitonesParam pitchbendRange {12, "Pitchbend range", 2};

    PercentParam velocitySens {"Velocity amount", 100};

    ToggleParam aftertouchToggle {"Aftertouch gain", true};

    ToggleParam voiceStealing {"Voice stealing", false};

    ToggleParam midiLatch {"MIDI latch", false};

    ToggleParam pitchGlide {"Pitch glide toggle", false};

    SecParam glideTime {1.f, "Pitch glide time", 0.4f};

    SecParam adsrAttack {1.f, "ADSR attack", 0.35f};
    SecParam adsrDecay {1.f, "ADSR decay", 0.06f};

    PercentParam adsrSustain {"ADSR sustain", 80};

    SecParam adsrRelease {1.f, "ADSR release", 0.01f};

    ToggleParam    pedalToggle {"Pedal toggle", false};
    PitchParam     pedalThresh {"Pedal thresh", 0};
    SemitonesParam pedalInterval {12, "Pedal interval", 12};

    ToggleParam    descantToggle {"Descant toggle", false};
    PitchParam     descantThresh {"Descant thresh", 127};
    SemitonesParam descantInterval {12, "Descant interval", 12};

    IntParam editorPitchbend {0, 127, 64, "GUI Pitchbend",
                              [] (int value, int maximumStringLength)
                              { return juce::String (value).substring (0, maximumStringLength); },
                              [] (const juce::String& text)
                              { return text.retainCharacters ("1234567890").getIntValue(); }};
};

}  // namespace Imogen
