#pragma once

namespace Imogen
{

struct MidiState
{
    MidiState (plugin::ParameterList& list);
    
    IntParam pitchbendRange {"Pitchbend range", "Pitchbend range", 0, 12, 2, st_stringFromInt, st_intFromString, TRANS ("st")};
    
    PercentParam velocitySens {"Velocity amount", "Velocity amount", 100};
    
    ToggleParam aftertouchToggle {"Aftertouch gain", "Aftertouch gain", true};
    
    ToggleParam voiceStealing {"Voice stealing", "Voice stealing", false};
    
    ToggleParam midiLatch {"MIDI latch", "MIDI latch", false};
    
    ToggleParam pitchGlide {"Glide toggle", "Pitch glide toggle", false};
    
    FloatParam glideTime {"Glide time", "Pitch glide time",
        juce::NormalisableRange< float > (0.0f, 2.0f, 0.01f),
        0.4f, generic, sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    FloatParam adsrAttack {"ADSR Attack", "ADSR attack",
        juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
        0.35f, generic, sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    FloatParam adsrDecay {"ADSR Decay", "ADSR decay",
        juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
        0.06f, generic, sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    PercentParam adsrSustain {"ADSR Sustain", "ADSR sustain", 80};
    
    FloatParam adsrRelease {"ADSR Release", "ADSR release",
        juce::NormalisableRange< float > (0.0f, 1.0f, 0.01f),
        0.1f, generic, sec_stringFromFloat, sec_floatFromString, TRANS ("sec")};
    
    ToggleParam pedalToggle {" Pedal toggle", "Pedal toggle", false};
    IntParam    pedalThresh {"Pedal thresh", "Pedal thresh", 0, 127, 0, pitch_stringFromInt, pitch_intFromString};
    IntParam    pedalInterval {"Pedal interval", "Pedal interval", 1, 12, 12, st_stringFromInt, st_intFromString};
    
    ToggleParam descantToggle {"Descant toggle", "Descant toggle", false};
    IntParam    descantThresh {"Descant thresh", "Descant thresh", 0, 127, 127, pcnt_stringFromInt, pitch_intFromString};
    IntParam    descantInterval {"Descant interval", "Descant interval", 1, 12, 12, st_stringFromInt, st_intFromString};
    
    IntParam editorPitchbend {"GUI Pitchbend", "GUI Pitchbend", 0, 127, 64,
        [] (int value, int maximumStringLength)
        { return juce::String (value).substring (0, maximumStringLength); },
        [] (const juce::String& text)
        { return text.retainCharacters ("1234567890").getIntValue(); }};
    
private:
    static constexpr auto generic = juce::AudioProcessorParameter::genericParameter;
};

}
