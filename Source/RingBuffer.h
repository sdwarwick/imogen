/*
  ==============================================================================

    RingBuffer.h
    Created: 11 Nov 2020 1:25:16am
    Author:  Ben Vining
 
 	Implements a circular buffer class

  ==============================================================================
*/

#pragma once


class RingBuffer
{
	
public:
	
	ScopedPointer<AbstractFifo> lockFreeFifo;
	Array<float> data;
	int lastReadPos;
	
	RingBuffer(): lastReadPos(0)
	{
		lockFreeFifo = new AbstractFifo(512);
		data.ensureStorageAllocated(512);
		FloatVectorOperations::clear(data.getRawDataPointer(), 512);
		while (data.size() < 512) {
			data.add(0.0f);
		}
	};
	
	void setTotalSize(const int newSize) {
		lockFreeFifo->setTotalSize(newSize);
		data.ensureStorageAllocated(newSize);
		FloatVectorOperations::clear(data.getRawDataPointer(), newSize);
		while (data.size() < newSize) {
			data.add(0.0f);
		}
	};
	
	void writeTo(const float* writeData, int numToWrite) {
		
	};
	
	void readFrom(float* readData, int numToRead) {
		
	};
	
	
	Array<float> getArray() { // writes from lockFreeFifo to array for GUI
		
	};
	
	
	
private:
	Array<float> data;
};
