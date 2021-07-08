#pragma once

namespace Imogen
{

struct MidiState
{
    MidiState (plugin::ParameterList& list);
    
    IntParam pitchbendRange {0, 12, 2, "Pitchbend range", "Pitchbend range", st_stringFromInt, st_intFromString, TRANS ("st")};
    
    PercentParam velocitySens {"Velocity amount", "Velocity amount", 100};
    
    ToggleParam aftertouchToggle {"Aftertouch gain", "Aftertouch gain", true};
    
    ToggleParam voiceStealing {"Voice stealing", "Voice stealing", false};
    
    ToggleParam midiLatch {"MIDI latch", "MIDI latch", false};
    
    ToggleParam pitchGlide {"Glide toggle", "Pitch glide toggle", false};
    
    FloatParam glideTime {0.f, 1.f, 0.4f, "Glide time", "Pitch glide time", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    FloatParam adsrAttack {0.f, 1.f, 0.35f, "ADSR Attack", "ADSR attack", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    FloatParam adsrDecay {0.0f, 1.0f, 0.06f, "ADSR Decay", "ADSR decay", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    PercentParam adsrSustain {"ADSR Sustain", "ADSR sustain", 80};
    
    FloatParam adsrRelease {0.f, 1.f, 0.1f, "ADSR Release", "ADSR release", sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    ToggleParam pedalToggle {" Pedal toggle", "Pedal toggle", false};
    IntParam    pedalThresh {0, 127, 0, "Pedal thresh", "Pedal thresh", pitch_stringFromInt, pitch_intFromString};
    IntParam    pedalInterval {1, 12, 12, "Pedal interval", "Pedal interval", st_stringFromInt, st_intFromString};
    
    ToggleParam descantToggle {"Descant toggle", "Descant toggle", false};
    IntParam    descantThresh {0, 127, 127, "Descant thresh", "Descant thresh", pcnt_stringFromInt, pitch_intFromString};
    IntParam    descantInterval {1, 12, 12, "Descant interval", "Descant interval", st_stringFromInt, st_intFromString};
    
    IntParam editorPitchbend {0, 127, 64, "GUI Pitchbend", "GUI Pitchbend",
        [] (int value, int maximumStringLength)
        { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text)
        { return text.retainCharacters ("1234567890").getIntValue(); }};
};

}
