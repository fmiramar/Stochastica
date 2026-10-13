#pragma once
#include "SDEventProcesses.hpp"

namespace sd {
// Default path preserves StateProcesses.cpp arithmetic, float uniform conversion
// and RNG consumption. New models opt into double uniforms and positive fallback.
template <typename Weight>
inline int categoricalSample(PCG32& rng, const Weight* row, int states, int fallback,
                             bool preciseUniform = false)
{
    double sum = 0.0;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (finite(weight) && weight > 0.0)
            sum += weight;
    }
    if (!(sum > 0.0) || !finite(sum))
        return fallback;
    double target = (preciseUniform ? openUniformDouble(rng)
                                   : static_cast<double>(uniform01ClosedOpen(rng))) * sum;
    int lastPositive = fallback;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (finite(weight) && weight > 0.0) {
            lastPositive = destination;
            target -= weight;
            if (target <= 0.0)
                return destination;
        }
    }
    return preciseUniform ? lastPositive : states - 1;
}
} // namespace sd
