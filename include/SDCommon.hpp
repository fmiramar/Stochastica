#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace sd {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kTwoPi = 2.0 * kPi;
constexpr int kMaxRejectionAttempts = 64;
constexpr float kLargestExactIntegerFloat = 16777215.0f;
constexpr double kTiny = 1.0e-12;

inline bool finite(double value)
{
    return std::isfinite(value);
}

inline double finiteOr(double value, double fallback)
{
    return finite(value) ? value : fallback;
}

inline int clampInt(int value, int low, int high)
{
    return std::min(std::max(value, low), high);
}

inline double clamp(double value, double low, double high)
{
    if (!finite(value))
        return low;
    return std::min(std::max(value, low), high);
}

inline float finiteFloat(double value)
{
    if (std::isnan(value))
        return 0.0f;
    const double maximum = static_cast<double>(std::numeric_limits<float>::max());
    if (value > maximum)
        return std::numeric_limits<float>::max();
    if (value < -maximum)
        return -std::numeric_limits<float>::max();
    return static_cast<float>(value);
}

inline uint32_t explicitSeedFromFloat(float seed)
{
    if (!std::isfinite(seed))
        return 0U;
    const double magnitude = std::abs(std::trunc(static_cast<double>(seed)));
    return static_cast<uint32_t>(std::fmod(magnitude, 4294967296.0));
}

struct RisingEdge {
    float previous = 0.0f;

    void initialize(float first) { previous = first; }

    bool update(float current)
    {
        const bool result = current > 0.0f && previous <= 0.0f;
        previous = current;
        return result;
    }
};

} // namespace sd

