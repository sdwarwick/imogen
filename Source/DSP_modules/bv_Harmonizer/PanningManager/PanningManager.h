/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: PanningManager
 */


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    

using namespace juce;
    
    
class PanningManager
{
public:
    
    PanningManager();
    ~PanningManager() {}
    
    void releaseResources();
    
    void prepare (const int numVoices, bool clearArrays = true);
    
    // used to update the width of the stereo field of currently available / valid pan values
    void updateStereoWidth(const int newWidth);
    
    
    // returns next available panning value. Panning values are assigned from the "middle out" - the first pan values sent will be toward the center of the stereo field, working outwards to either side as more pan values are requested
    int getNextPanVal();
    
    
    // used to tell the PanningManager when a previously assigned panningvalue is turned off - ie, is now available again for another voice 
    void panValTurnedOff (int panVal);
    
    
    // used when updating stereo width -- voices should grab the new pan val that's closest to their old pan val
    int getClosestNewPanValFromOld (int oldPan);
    
    
    // tells the PanningManager that all voices have been turned off -- ie, all the pan vals are available again
    void reset();
    
    
    int getCurrentStereoWidth() const
    {
        const ScopedLock sl (lock);
        return lastRecievedStereoWidth;
    }
    
    int getCurrentNumVoices()   const noexcept { return currentNumVoices; }
    
    
private:
    
    CriticalSection lock;
    
    Array<int> panValsInAssigningOrder; // this array stores the pan values in the order they will be sent out, ie "middle out". Index 0 contains 64, and the highest two indices will contain 0 and 127 [if the stereo width is 100]
    
    Array<int> arrayIndexesMapped; // this array is used to facilitate the transfer of values from possiblePanVals to panValsInAssigningOrder
    
    Array<int> unsentPanVals; // this is the array we will actually be reading pan vals from! the others are for sorting.
    
    
    int lastRecievedStereoWidth;
    int currentNumVoices;
    
    void mapArrayIndexes();
    
    int getClosestNewPanValFromNew (int oldPan, Array<int>& readingFrom);
    
    int findClosestValueInNewArray (int targetValue, Array<int>& newArray, bool removeFoundValFromNewArray = false);
    
    Array<int> possiblePanVals;
    
    Array<int> newPanVals;
    Array<int> newUnsentVals;
    
    Array<int> distances;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanningManager)
};


} // namespace
