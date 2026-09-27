#pragma once

#include "SDCommon.hpp"

#include <cmath>

namespace sd {

inline double clipValue(double value, double low, double high)
{
    return std::min(std::max(value, low), high);
}

inline double wrapValue(double value, double low, double high)
{
    const double width = high - low;
    if (!(width > 0.0) || !finite(value))
        return low;
    double result = std::fmod(value - low, width);
    if (result < 0.0)
        result += width;
    return low + result;
}

inline double foldValue(double value, double low, double high)
{
    const double width = high - low;
    if (!(width > 0.0) || !finite(value))
        return low;
    double result = std::fmod(value - low, 2.0 * width);
    if (result < 0.0)
        result += 2.0 * width;
    return low + (result <= width ? result : 2.0 * width - result);
}

inline double reflectOnce(double value, double low, double high)
{
    if (value < low)
        return low + (low - value);
    if (value > high)
        return high - (value - high);
    return value;
}

} // namespace sd

