
#include "Source/Tests/tests.cpp"

#include "bv_Harmonizer/bv_Harmonizer.h"


TEST_CASE("HarmonizerVoice MIDI is working correctly", "[Harmonizer][HarmonizerVoice][MIDI]")
{
    bav::HarmonizerVoice testHarmonizerVoice;
    
    testHarmonizerVoice.prepare (512);
    
    SECTION("Turning on a single note")
    {
        testHarmonizer.allNotesOff (false);
        
    }
    
};
