/*
  ==============================================================================

    PanningManager.cpp
    Created: 14 Dec 2020 3:26:58pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "PanningManager.h"

PanningManager::PanningManager(): lastRecievedStereoWidth(64), currentNumVoices(0)
{
	possiblePanVals.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	panValsInAssigningOrder.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	arrayIndexesMapped.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	unsentPanVals.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	absDistances.ensureStorageAllocated(MAX_POSSIBLE_NUMBER_OF_VOICES);
	
	setNumberOfVoices(1);
};


PanningManager::~PanningManager()
{
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
	const ScopedLock sl (lock);
	lastRecievedStereoWidth = newWidth;
	
	const auto rangeMultiplier = newWidth/100.0f;
	const auto maxPan = 63.5f + (63.5f * rangeMultiplier);
	const auto minPan = 63.5f - (63.5f * rangeMultiplier);
	const auto increment = (maxPan - minPan) / currentNumVoices;
	
	// first, assign all possible, evenly spaced pan vals within range to an array
	possiblePanVals.clearQuick();
	possiblePanVals.add(64);
	for (int i = 1; i < currentNumVoices; ++i)
		possiblePanVals.add(round(minPan + (i * increment) + (increment/2.0f)));
	
	// then reorder them into "assigning order" -- center out, by writing from the possiblePanVals array to the panValsInAssigningOrder array in the array index order held in arrayIndexesMapped
	panValsInAssigningOrder.clearQuick();
	panValsInAssigningOrder.add(64);
	for (int i = 1; i < currentNumVoices; ++i)
		panValsInAssigningOrder.add(possiblePanVals.getUnchecked(arrayIndexesMapped.getUnchecked(i) - 1));
	
	// transfer to I/O array we will be actually reading from
	unsentPanVals.clearQuick();
	for(int i = 0; i < currentNumVoices; ++i)
		unsentPanVals.add(panValsInAssigningOrder.getUnchecked(i));
};


int PanningManager::getNextPanVal() 
{
	const ScopedLock sl (lock);
	
	if(! unsentPanVals.isEmpty())
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


void PanningManager::panValTurnedOff(const int panVal)
{
	const ScopedLock sl (lock);
	
	const auto targetindex = panValsInAssigningOrder.indexOf(panVal);
	
	if(targetindex > -1) // targetindex will be -1 if the turned off pan val is not in panValsInAssigningOrder. in this case, do nothing.
	{
		if(! unsentPanVals.isEmpty())
		{
			if(panValsInAssigningOrder.indexOf(unsentPanVals.getUnchecked(0)) > targetindex)
				unsentPanVals.insert(0, panVal);
			else
			{
				int i = 1;
				bool addedIt = false;
				while (i < currentNumVoices)
				{
					if(panValsInAssigningOrder.indexOf(unsentPanVals.getUnchecked(i)) > targetindex)
					{
						unsentPanVals.insert(i, panVal);
						addedIt = true;
						break;
					}
					else
						++i;
				}
				if(! addedIt)
					unsentPanVals.add(panVal);
			}
		}
		else
			unsentPanVals.add(panVal);
	}
};


int PanningManager::getClosestNewPanValFromOld(const int oldPan)
{
	const ScopedLock sl (lock);
	
	if(unsentPanVals.isEmpty())
	{
		reset(true);
		return 64;
	}
	
	absDistances.clearQuick();
	
	// find & return the element in unsentPanVals that is the closest to oldPan, then remove that val from unsentPanVals
	
	if(const int targetsize = unsentPanVals.size(); targetsize != absDistances.size())
		absDistances.resize(targetsize);
	
	for(int i = 0; i < unsentPanVals.size(); ++i)
		absDistances.add(abs(oldPan - unsentPanVals.getUnchecked(i)));
	
	// find the minimum val in absDistances *in place* -- if we sort, we lose the index # and can't find the original panValue from unsentPanVals
	int minimum = 128; // higher than highest possible midiPan
	for(int i = 0; i < unsentPanVals.size(); ++i)
		if(const int tester = absDistances.getUnchecked(i); tester < minimum)
			minimum = tester;
	
	const auto minIndex = absDistances.indexOf(minimum);
	
	const auto newPan = unsentPanVals.getUnchecked(minIndex);
	unsentPanVals.remove(minIndex);
	return newPan;
};


void PanningManager::reset(const bool grabbedFirst64)
{
	const ScopedLock sl (lock);
	
	unsentPanVals.clearQuick();
	
	for(int i = 0; i < currentNumVoices; ++i)
		unsentPanVals.add(panValsInAssigningOrder.getUnchecked(i));
	
	if(grabbedFirst64)
		unsentPanVals.remove(0);
};


void PanningManager::mapArrayIndexes()
{
	/* In my updateStereoWidth() function, possible panning values are written to the possiblePanVals array in order from least to greatest absolute value. Index 0 will contain the smallest midiPan value, and index 11 the highest.
	 
	 When an instance of the harmony algorithm requests a new midiPan value, I want to assign them from the "center out" -- so that the first voice that turns on will be the one in the center, then the sides get added as more voices turn on.
	 
	 So I need to transfer the values in possiblePanVals into another array - panValsInAssigningOrder - which will hold the panning values in order so that index 0 contains 64, index 1 contains 72, index 2 contains 52... index 10 contains 127 and index 11 contains 0.
	 
	 In order to transfer the panning values from array to array like this, I need to essentially have a list of array indices of possiblePanVals to read from, in order of the panValsInAssigningOrder indices I'll be writing to. IE, in this list, index 0 will contain 6, meaning that I want to write the value in index 6 of possiblePanVals to index 0 of panValsInAssigningOrder.
	 
	 I'm storing this list in another array called arrayIndexesMapped.
	 */
	
	const ScopedLock sl (lock);
	
	const auto middleIndex = currentNumVoices > 1 ? floor(currentNumVoices / 2) : 1;
	
	arrayIndexesMapped.clearQuick();
	
	arrayIndexesMapped.add(middleIndex);
	
	int i = 1;
	int p = 1;
	int m = -1;
	
	while (i < currentNumVoices)
	{
		if(i % 2 == 0) { // i is even
			arrayIndexesMapped.add(middleIndex + p);
			++p;
		} else { // i is odd
			arrayIndexesMapped.add(middleIndex - m);
			--m;
		}
		++i;
	}
};
