/*
    This file defines a visual staff display component, which displays noteheads corresponding to the harmonizer's currently playing notes, updating in real-time.
    Parent file: StaffDisplay.h.
*/


#include <JuceHeader.h>
#include "../../Source/GUI/StaffDisplay/StaffDisplay.h"


StaffDisplay::StaffDisplay(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
    audioProcessor(p), lookAndFeel(l),  grandStaff(ImageCache::getFromMemory(BinaryData::grandStaff_png, BinaryData::grandStaff_pngSize)),
    useFlats(false), halfTheStafflineHeight(10.0f),
    noteheadSvg("m521.58476,353.29514c-29.04901,-6.89218 -49.78421,-21.77307 -58.79819,-39.21951c-4.23883,-7.8918 -6.74962,-16.49637 -7.31462,-25.45385c-0.00869,-0.13604 -0.00782,-0.25729 -0.01,-0.38638c-0.17037,-10.00491 2.51122,-19.45438 10.71421,-30.12512c7.52237,-9.83976 19.03888,-18.26917 33.92977,-24.74368c15.74448,-6.87349 34.40655,-10.27959 54.51677,-9.65809c4.09019,0.12647 8.12258,0.41376 12.08412,0.85185c4.30576,0.43983 8.52721,1.06047 12.64652,1.8493c8.00436,1.31602 15.62496,3.30918 22.73444,5.88995c6.99083,2.43647 13.40493,5.36102 19.71211,9.13785c5.57789,3.34048 9.92668,6.62097 14.19506,10.58904c2.13876,2.06965 4.12626,4.22492 5.94905,6.45712c1.43772,1.81844 2.77417,3.67383 4.00283,5.56137c1.36427,2.19526 2.56859,4.30011 3.69208,6.68356c3.67948,7.80444 5.61527,16.66326 5.81563,24.15956c0.00522,14.84916 -7.53671,28.20281 -20.14933,38.73665c-9.29474,8.09868 -20.42792,14.14421 -34.84204,18.66381c-10.73898,3.36742 -21.93344,5.18543 -33.05054,5.50748c-3.68165,0.18645 -6.52405,0.05172 -10.26785,0.01608c-12.61566,-0.4068 -24.57201,-1.96752 -35.56002,-4.51699zm62.51678,-18.83114c1.92927,-1.49378 3.25877,-2.87152 5.29017,-5.26757c3.5165,-4.148 5.84953,-8.28991 7.30854,-13.74915c0.16429,-0.53458 0.31249,-1.07177 0.44418,-1.61156c0.31901,-1.26083 0.54892,-2.52991 0.69669,-3.80247c0.29989,-4.15234 0.0691,-9.03919 -1.1713,-13.74784c-0.31553,-1.36644 -0.61672,-2.64422 -0.91226,-3.86332c-1.26865,-5.65699 -2.49427,-9.98666 -4.57218,-15.84444c-0.123,-0.32988 -0.2812,-0.82534 -0.40811,-1.15522c-4.38877,-11.41959 -8.92923,-19.21621 -15.34898,-25.02184c-8.75147,-8.26383 -21.64138,-12.88469 -35.40008,-8.66542c-5.50965,1.6898 -10.10009,4.55176 -13.14329,7.65406c-1.77281,1.77498 -3.50128,3.97676 -4.84338,6.12116c-3.40349,5.47532 -5.04201,9.62505 -5.58876,15.4585c-0.11995,2.05748 -0.25599,4.55784 0,6.95389c0.42419,2.68464 0.68192,4.93422 1.2278,7.52324c1.52247,7.81226 3.68556,14.94261 5.40622,20.29536c0.01956,0.06085 0.03651,0.123 0.05476,0.18428c0.82186,2.74679 1.81149,5.36319 2.94541,7.83964c0.93182,2.13919 2.08573,4.58305 3.20009,6.50797c0.74885,1.35166 1.53768,2.66812 2.35215,3.91156c1.39773,2.14875 2.72636,3.64514 4.23622,5.35797c2.38866,2.62596 4.89771,4.74951 7.49325,6.43061c4.07324,2.48515 8.27948,3.98719 12.50223,4.713c7.74707,1.59809 16.39727,0.23382 20.95772,-1.92927c3.0232,-1.43381 4.64476,-2.25828 7.2729,-4.29316z"),
noteheadPath(Drawable::parseSVGPath(noteheadSvg))
{
    displayFlats.addItem("Display flats", 1);
    displayFlats.addItem("Display sharps", 2);
    displayFlats.setSelectedId(2);
    addAndMakeVisible(displayFlats);
    displayFlats.onChange = [this] { this->repaint(); };
    
    yCoordsOfActiveNotes.ensureStorageAllocated(12);
    yCoordsOfActiveNotes.clearQuick();
    
    const int noteheadHeightPx = 5;
    yCoordLookupTable[0] = 0;
    for(int n = 1; n < 127; ++n)
    {
        if(MidiUtils::NoteHelpers::isMidiNoteBlackKey(n))
            yCoordLookupTable[n] = yCoordLookupTable[n - 1];
        else
            yCoordLookupTable[n] = yCoordLookupTable[n - 1] + noteheadHeightPx;
    }
    
    staffImage.setImage(grandStaff);
    addAndMakeVisible(staffImage);
};

StaffDisplay::~StaffDisplay()
{
    setLookAndFeel (nullptr);
};

void StaffDisplay::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::staffDisplayBackgroundColourId));
    
    Array<int> currentlyActive;
    currentlyActive.ensureStorageAllocated(12);
    audioProcessor.returnActivePitches (currentlyActive);
    
    drawPitches (currentlyActive, g);
};

void StaffDisplay::resized()
{
    staffImage.setBounds(17, 40, 265, 197);
    displayFlats.setBounds(80, 300, 140, 35);
    
    noteheadPath.applyTransform(noteheadPath.getTransformToScaleToFit(getLocalBounds().toFloat().reduced(10.0), true, Justification::centred));
    
    auto ctr{ noteheadPath.getBounds().getCentre() };
    noteheadPath.applyTransform(AffineTransform::translation(-ctr.getX(), -ctr.getY()));
};


void StaffDisplay::drawPitches(const Array<int>& activePitches, Graphics& g)
{
    // TO DO :: draw ledger lines, if needed !
    
    useFlats = displayFlats.getSelectedId() == 1 ? true : false;
    
    if (! activePitches.isEmpty())
    {
        yCoordsOfActiveNotes.clearQuick();
        for(int n = 0; n < activePitches.size(); ++n)
            yCoordsOfActiveNotes.add(yCoordLookupTable[activePitches.getUnchecked(n)] + 17);
        
        if(! yCoordsOfActiveNotes.isEmpty())
        {
            int xOffset = 0;
            const int someScaleFactor = 7; // the scaleing factor applied to the incremented "x offset" value (ie, the amount that "1 offset" equals)
            const int baseXCoord = 75; // the base x coordinate to which the offset value is added
            
            for(int n = 0; n < yCoordsOfActiveNotes.size(); ++n)
            {
                
                const int yCoord = yCoordsOfActiveNotes.getUnchecked(n);
                
                if(n == yCoordsOfActiveNotes.size() - 1)
                    xOffset = 0;
                
                if(n < yCoordsOfActiveNotes.size() - 2)
                    if(yCoordsOfActiveNotes.getUnchecked(n + 1) - yCoord > halfTheStafflineHeight)
                        xOffset = 0;
                
                const int xCoord = xOffset * someScaleFactor + baseXCoord;
                
                drawNotehead(xCoord, yCoord, g);
                
                if(MidiUtils::NoteHelpers::isMidiNoteBlackKey(activePitches.getUnchecked(n)))
                    drawAccidental(xCoord, yCoord, g);
                
                xOffset ^= 1;
                
            }
        }
    }
};


void StaffDisplay::drawNotehead(const int x, const int y, Graphics& g)
{
    // x & y coords are the center of the notehead.
    
    g.setColour(juce::Colours::black);
    
    g.fillPath(noteheadPath, AffineTransform::translation(x, y));
};


void StaffDisplay::drawAccidental(const int x, const int y, Graphics& g)
{
    g.setColour(juce::Colours::black);
    
    ignoreUnused (x, y);
    
    // x & y coords are the center of the notehead
    // use different x offset values for sharps / flats
    if (useFlats)
    {
        
    }
    else
    {
        
    }
    
};
