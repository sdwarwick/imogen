/*
  ==============================================================================

    Window.h
    Created: 18 Nov 2020 1:16:01am
    Author:  Ben Vining
 
 	Computes sample values for a Hanning window of variable length & writes to *window array

  ==============================================================================
*/

#pragma once


class Window
{
	
public:
	
	void calcWindow(int length, const int analysisShift, const int analysisShiftHalved, Array<float>* window) {
		
		if (length < analysisShift + analysisShiftHalved) { length = analysisShift + analysisShiftHalved; };
		
		const int winLen = length;
		
		if(window->size() != winLen) {
			window->resize(winLen);
		}
		
		for(int i = 0; i < winLen; ++i)
		{
			const float cos2 = ncos(2, i, winLen);
			window->set(i, 0.5 - (0.5 * cos2));
		}
		
	};
	
	
private:
	
	float ncos(const int order, const int i, const int size) const
	{
		return std::cos((order * i * MathConstants<float>::pi)/(size - 1));
	}
	
};
