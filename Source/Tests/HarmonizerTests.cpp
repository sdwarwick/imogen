
#include "Source/Tests/tests.cpp"

#include "bv_Harmonizer/bv_Harmonizer.h"


TEST_CASE("Harmonizer MIDI is working correctly", "[Harmonizer][MIDI]")
{
    bav::Harmonizer testHarmonizer;
    
    for (int i = 0; i < 12; ++i)
        harmonizer.addVoice (new HarmonizerVoice<SampleType>(&harmonizer));
    
    testHarmonizer.setCurrentPlaybackSampleRate (44100.0);
    testHarmonizer.prepare (512);
    
    SECTION("Turning on a single note")
    {
        testHarmonizer.allNotesOff (false);
        
    }
    
};
