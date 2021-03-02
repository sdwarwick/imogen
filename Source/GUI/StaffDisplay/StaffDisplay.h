/*
    This file defines a visual staff display component, which displays noteheads corresponding to the harmonizer's currently playing notes, updating in real-time.
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/


#pragma once

#include "BinaryData.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/GUI/LookAndFeel.h"

#include "bv_GeneralUtils/bv_GeneralUtils.h"


namespace bav

{
    

class StaffDisplay  : public juce::Component
{
public:
    ImogenAudioProcessor& audioProcessor;
    ImogenLookAndFeel& lookAndFeel;
    StaffDisplay(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~StaffDisplay() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
private:
    
    juce::Image grandStaff;
    juce::ImageComponent staffImage;
    juce::ComboBox displayFlats;
    
    juce::Array<int> yCoordsOfActiveNotes;
    int yCoordLookupTable[127];
    bool useFlats;
    
    void drawPitches(const juce::Array<int>& activePitches, juce::Graphics& g);
    void drawNotehead(const int x, const int y, juce::Graphics& g);
    void drawAccidental(const int x, const int y, juce::Graphics& g);
    
    const float halfTheStafflineHeight;
    
    juce::String noteheadSvg;
    juce::Path noteheadPath;
    
    //String flatSvg;
    //Path flatPath;
    //String sharpSvg;
    //Path sharpPath;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaffDisplay)
};


}  // namespace
