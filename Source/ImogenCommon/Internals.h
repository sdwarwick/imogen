
#pragma once

namespace Imogen
{

struct Internals :  bav::ParameterList
{
    using IntParam   = bav::IntParam;
    using BoolParam  = bav::BoolParam;
    using ToggleParam  = bav::ToggleParam;
    
    Internals()
    : ParameterList ("ImogenInternals")
    {
        addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentInputNote, currentCentsSharp, editorSizeX, editorSizeY);
    }
    
    
    ToggleParam abletonLinkEnabled {"Toggle", "Ableton link toggle", false};
    
    IntParam abletonLinkSessionPeers {"Num peers", "Ableton link num session peers", 0, 50, 0,
        [] (int value, int maximumStringLength)
        { return juce::String (value).substring (0, maximumStringLength); }};
    
    ToggleParam mtsEspIsConnected {"Is connected", "MTS-ESP is connected", false};
    
    IntParam lastMovedMidiController {"Number", "Last moved MIDI controller number", 0, 127, 0};
    
    IntParam lastMovedCCValue {"Value", "Last moved MIDI controller value", 0, 127, 0};
    
    BoolParam guiDarkMode {"Dark mode", "GUI Dark mode", true,
        [] (bool val, int maxLength)
        {
            if (val) return TRANS ("Dark mode is on").substring (0, maxLength);
            
            return TRANS ("Dark mode is off").substring (0, maxLength);
        }};
    
    IntParam currentInputNote {"Current note", "Current input note", -1, 127, -1,
        [](int note, int maxLength)
        {
            if (note == -1) return juce::String("Unpitched");
            return bav::pitchToString(note).substring(0, maxLength);
        }};
    
    IntParam currentCentsSharp {"Cents sharp", "Current input cents sharp", -100, 100, 0,
        [] (int cents, int maxLength)
        {
            if (cents == 0) return TRANS ("Perfect!");
            
            if (cents > 0) return (juce::String (cents) + TRANS (" cents sharp")).substring (0, maxLength);
            
            return (juce::String (abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
        },
        nullptr,
        TRANS ("cents")};
    
    IntParam editorSizeX {"editorSizeX", "editor size X", 0, 10000, 900};
    IntParam editorSizeY {"editorSizeY", "editor size Y", 0, 10000, 400};
    
};

}  // namespace


// auto scaleName = std::make_unique< StringNode > ("Scale name", "MTS-ESP scale name", "No active scale");
