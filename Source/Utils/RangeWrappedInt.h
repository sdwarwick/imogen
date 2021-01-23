/*
  ==============================================================================

    RangeWrappedInt.h
    Created: 21 Jan 2021 7:37:16pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


class RangeWrappedInt

{
public:
    
    RangeWrappedInt (int mininum, int maximum, int initialValue): value(initialValue), min(mininum), max(maximum)
    {
        if (! (max > min))
            max = min + 1;
        
        updateValue (0, true); // check to make sure the user didn't initialize the value outside the range
    };
    
    RangeWrappedInt(): value(0), min(0), max(100)
    { };
    
    ~RangeWrappedInt()
    { };
    
    
    int getValue() const noexcept
    {
        return value;
    }
    
    int getCurrentMin() const noexcept
    {
        return min;
    }
    
    int getCurrentMax() const noexcept
    {
        return max;
    }
    
    
    void setMin (int newMinimum) noexcept
    {
        if (! (newMinimum < max))
            newMinimum = max - 1;
        
        if (min == newMinimum)
            return;
        
        min = newMinimum;
        
        updateValue (0, true);
    }
    
    
    void setMax (int newMaximum) noexcept
    {
        if (! (newMaximum > min))
            newMaximum = min + 1;
        
        if (max == newMaximum)
            return;
        
        max = newMaximum;
        
        updateValue (0, true);
    }
    
    
    template<type>
    operator == (type other)
    {
        return static_cast<type>(value) == other;
    }
    
    template<type>
    operator != (type other)
    {
        return static_cast<type>(value) != other;
    }
    
    template<type>
    operator += (type inc)
    {
        updateValue (round(inc));
    }
    
    template<type>
    operator -= (type dec)
    {
        updateValue (0 - round(dec));
    }
    
    template<type>
    operator *= (type other)
    {
        updateValue (round (value * other) - value);
    }
    
    template<type>
    operator /= (type other)
    {
        updateValue (round (value / other) - value);
    }
    
    template<type>
    operator + (type other)
    {
        return static_cast<type>(value) + other;
    }
    
    template<type>
    operator - (type other)
    {
        return static_cast<type>(value) - other;
    }
    
    template<type>
    operator * (type other)
    {
        return static_cast<type>(value) * other;
    }
    
    template<type>
    operator / (type other)
    {
        return static_cast<type>(value) / other;
    }
    
    template<type>
    operator % (type other)
    {
        return value % other;
    }
    
    template<type>
    operator %= (type other)
    {
        value = round (value % other);
    }
    
    template<type>
    operator = (type other)
    {
        updateValue (round(other) - value);
    }
    
    template<type>
    operator < (type other)
    {
        return static_cast<type>(value) < other;
    }
    
    template<type>
    operator > (type other)
    {
        return static_cast<type>(value) > other;
    }
    
    template<type>
    operator <= (type other)
    {
        return static_cast<type>(value) <= other;
    }
    
    template<type>
    operator >= (type other)
    {
        return static_cast<type>(value) >= other;
    }
    
    operator ++ ()
    {
        updateValue (1);
    }
    
    operator -- ()
    {
        updateValue (-1);
    }
    
    
private:
    
    int value;
    
    int min, max;
    
    
    void updateValue (int changeApplied, bool forceRecalc = false) noexcept
    {
        if (changeApplied == 0 && ! forceRecalc)
            return;
        
        int tmp = value + changeApplied;
        
        while (tmp < min)
            tmp += (max - min);
        
        if (tmp > max)
            tmp = tmp % (max - min);
        
        value = tmp;
    };
    
};
