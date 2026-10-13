#pragma once
#include "SDRandom.hpp"

namespace sd {
// Extracted verbatim arithmetic/order from StateProcesses.cpp. Preserve its
// float uniform conversion and RNG consumption for legacy seeded sequences.
template <typename Weight>
inline int categoricalSample(PCG32& rng, const Weight* row, int states, int fallback)
{
    double sum = 0.0;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (finite(weight) && weight > 0.0)
            sum += weight;
    }
    if (!(sum > 0.0) || !finite(sum))
        return fallback;
    double target = static_cast<double>(uniform01ClosedOpen(rng)) * sum;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (finite(weight) && weight > 0.0) {
            target -= weight;
            if (target <= 0.0)
                return destination;
        }
    }
    return states - 1;
}
} // namespace sd
