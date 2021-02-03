/*
    Part of module: bv_Harmonizer
    Direct parent file: PanningManager.h
    Classes: PanningManager
 */


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    
    

PanningManager::PanningManager(): lastRecievedStereoWidth(64), currentNumVoices(0)
{ };


void PanningManager::releaseResources()
{
    panValsInAssigningOrder.clear();
    arrayIndexesMapped.clear();
    unsentPanVals.clear();
};


void PanningManager::prepare (const int numVoices)
{
    panValsInAssigningOrder.ensureStorageAllocated(numVoices);
    arrayIndexesMapped.ensureStorageAllocated(numVoices);
    unsentPanVals.ensureStorageAllocated(numVoices);
    
    setNumberOfVoices(numVoices);
    
    updateStereoWidth(lastRecievedStereoWidth);
};


void PanningManager::setNumberOfVoices(const int newNumVoices)
{
    jassert(newNumVoices > 0);
    
    currentNumVoices = newNumVoices;
    
    mapArrayIndexes();
    updateStereoWidth(lastRecievedStereoWidth);
};


void PanningManager::updateStereoWidth(const int newWidth)
{
    if (currentNumVoices == 0)
        return;
    
    lastRecievedStereoWidth = newWidth;
    
    const auto rangeMultiplier = newWidth/100.0f;
    const auto maxPan = 63.5f + (63.5f * rangeMultiplier);
    const auto minPan = 63.5f - (63.5f * rangeMultiplier);
    const auto increment = (maxPan - minPan) / currentNumVoices;
    
    Array<int> possiblePanVals;
    possiblePanVals.ensureStorageAllocated (currentNumVoices);
    
    for (int i = 0; i < currentNumVoices; ++i)
        possiblePanVals.add (juce::roundToInt (minPan + (i * increment) + (increment/2.0f)));
    
    // then reorder them into "assigning order" -- center out, by writing from the possiblePanVals array to the panValsInAssigningOrder array in the array index order held in arrayIndexesMapped
    panValsInAssigningOrder.clearQuick();
    
    for (int index : arrayIndexesMapped)
        panValsInAssigningOrder.add (possiblePanVals.getUnchecked(index));
    
    jassert (! panValsInAssigningOrder.isEmpty());
    
    if (unsentPanVals.isEmpty())
        return;
    
    // the # of values we actually transfer to the I/O array being read from should correspond to the number of unsent pan vals left now -- ie, if some voices are already on, etc. And the values put in the I/O array should also be the values out of the panValsInAssigningOrder array that are closest to the old values from unsentPanVals...
    
    // make copy of panValsInAssigningOrder bc items will be removed during the searching process below
    Array<int> newPanVals;
    newPanVals.ensureStorageAllocated (panValsInAssigningOrder.size());
    
    for (int newPan : panValsInAssigningOrder)
        newPanVals.add (newPan);
    
    Array<int> newUnsentVals;
    newUnsentVals.ensureStorageAllocated (unsentPanVals.size());
    
    for (int oldPan : unsentPanVals)
        newUnsentVals.add (getClosestNewPanValFromNew (oldPan, newPanVals));
    
    newUnsentVals.removeAllInstancesOf (-1);
    
    // transfer to I/O array we will be actually reading from
    unsentPanVals.clearQuick();
    
    for (int newPan : newUnsentVals)
        unsentPanVals.add (newPan);
    
    jassert (! unsentPanVals.isEmpty());
};


int PanningManager::getNextPanVal() 
{
    if (! unsentPanVals.isEmpty())
    {
        const auto nextPan = unsentPanVals.getUnchecked(0);
        unsentPanVals.remove(0);
        return nextPan;
    }
    else
    {
        reset(true);
        return 64;
    }
};


void PanningManager::panValTurnedOff (const int panVal)
{
    // this function is called when a pan value is turned off and is available again for assigning. This function attempts to reinsert the pan value into unsentPanVals with respect to the order the values are in in panValsInAssigningOrder
    
    const auto targetindex = panValsInAssigningOrder.indexOf (panVal);
    
    if (targetindex == -1) // targetindex will be -1 if the turned off pan val is not in panValsInAssigningOrder. in this case, do nothing.
        return;
    
    if (unsentPanVals.isEmpty())
    {
        unsentPanVals.add(panVal);
        return;
    }
    
    int i = 0;
    bool addedIt = false;
    
    while (i < currentNumVoices)
    {
        if (i > panValsInAssigningOrder.size())
            break;
        
        if (panValsInAssigningOrder.indexOf (unsentPanVals.getUnchecked(i)) > targetindex)
        {
            unsentPanVals.insert (i, panVal);
            addedIt = true;
            break;
        }
        else
            ++i;
    }
    
    if (! addedIt)
        unsentPanVals.add(panVal);
};


int PanningManager::getClosestNewPanValFromNew (const int oldPan, Array<int>& readingFrom)
{
    jassert (isPositiveAndBelow(oldPan, 128));
    
    if (readingFrom.isEmpty())
        return -1;
    
    Array<int> distances;
    distances.ensureStorageAllocated (readingFrom.size());
    
    for (int pan : readingFrom)
    {
        const int distance = abs (oldPan - pan);
        distances.add (distance);
        
        if (distance == 0)
            break;
    }
    
    // find the minimum val in absDistances
    int minimum = 128; // higher than highest possible midiPan of 127
    
    for (int distance : distances)
    {
        if (distance < minimum)
            minimum = distance;
        
        if (distance == 0)
            break;
    }
    
    int minIndex = distances.indexOf (minimum); // this is the index of the located pan value in both absDistances & unsentPanVals
    
    if (minIndex < 0)
        minIndex = 0;
    
    const auto newPan = readingFrom.getUnchecked(minIndex);
    readingFrom.remove (minIndex);
    return newPan;
};


int PanningManager::getClosestNewPanValFromOld (const int oldPan)
{
    // find & return the element in readingFrom array that is the closest to oldPan, then remove that val from unsentPanVals
    // this is normally used with the unsentPanVals array, but the same function can also be used in the updating of the stereo width, to identify which new pan values should be sent to the unsentPanVals array itself, based on which new pan values are closest to the ones that were already in unsentPanVals.
    
    jassert (isPositiveAndBelow(oldPan, 128));
    
    if (unsentPanVals.isEmpty())
    {
        reset(true);
        return 64;
    }
    
    Array<int> distances;
    distances.ensureStorageAllocated (unsentPanVals.size());
    
    for (int pan : unsentPanVals)
    {
        const int distance = abs (oldPan - pan);
        distances.add (distance);
        
        if (distance == 0)
            break;
    }
    
    // find the minimum val in absDistances
    int minimum = 128; // higher than highest possible midiPan of 127
    
    for (int distance : distances)
    {
        if (distance < minimum)
            minimum = distance;
        
        if (distance == 0)
            break;
    }
    
    int minIndex = distances.indexOf (minimum); // this is the index of the located pan value in both absDistances & unsentPanVals
    
    if (minIndex < 0)
        minIndex = 0;
    
    const auto newPan = unsentPanVals.getUnchecked(minIndex);
    unsentPanVals.remove (minIndex);
    return newPan;
};


void PanningManager::reset(const bool grabbedFirst64)
{
    unsentPanVals.clearQuick();
    
    int starting = grabbedFirst64 ? 1 : 0;
    
    for (int i = starting; i < panValsInAssigningOrder.size(); ++i)
        unsentPanVals.add (panValsInAssigningOrder.getUnchecked(i));
};


void PanningManager::mapArrayIndexes()
{
    /* In my updateStereoWidth() function, possible panning values are written to the possiblePanVals array in order from least to greatest absolute value. Index 0 will contain the smallest midiPan value, and the highest index will contain the largest midiPan value.
     
     When a new midiPan value is requested with getNextPanVal(), I want to assign them from the "center out" -- so that the first voice that turns on will be the one in the center, then the sides get added as more voices turn on.
     
     So I need to transfer the values in possiblePanVals into another array - panValsInAssigningOrder - which will hold the panning values in order so that index 0 contains 64, index 1 contains 72, index 2 contains 52... index 10 contains 127 and index 11 contains 0. [for 12 voices]
     
     In order to transfer the panning values from array to array like this, I need to essentially have a list of array indices of possiblePanVals to read from, in order of the panValsInAssigningOrder indices I'll be writing to. IE, in this list, if I'm working with 12 voices, index 0 will contain 6, meaning that I want to write the value in index 6 of possiblePanVals to index 0 of panValsInAssigningOrder.
     
     I'm storing this list in another array called arrayIndexesMapped.
     */
    
    arrayIndexesMapped.clearQuick();
    
    const int middleIndex = currentNumVoices > 1 ? roundToInt (floor (currentNumVoices / 2)) : 0;
    
    arrayIndexesMapped.add (middleIndex);
    
    int i = 1;
    int p = 1;
    int m = -1;
    
    while (i < currentNumVoices)
    {
        if (i % 2 == 0) // i is even
        { 
            const int newI = middleIndex + p;
            
            if (newI > currentNumVoices)
                continue;
        
            arrayIndexesMapped.add(newI);
            ++p;
        } 
        else // i is odd
        { 
            const int newI = middleIndex + m;
            
            if (newI < 0)
                continue;
            
            arrayIndexesMapped.add(newI);
            --m;
        }
        ++i;
    }
};


}; // namespace
