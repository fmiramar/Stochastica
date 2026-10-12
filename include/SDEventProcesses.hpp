#pragma once

#include "SDRandom.hpp"

namespace sd {

constexpr int kMaxTelegraphTransitionsPerCalc = 8;
constexpr int kMaxShotEventsPerCalc = 8;

// The existing float uniform helper can round to 1. Use the same PCG bits in
// double precision when a strictly open interval is mathematically required.
inline double openUniformDouble(PCG32& rng)
{
    return (static_cast<double>(rng.nextUInt()) + 0.5) / 4294967296.0;
}

inline double exponentialHazardThreshold(PCG32& rng)
{
    return -std::log(openUniformDouble(rng));
}

inline double nonnegative(double value)
{
    return std::max(finiteOr(value, 0.0), 0.0);
}

inline double eventProbability(double intensity, double dt)
{
    return -std::expm1(-nonnegative(intensity) * nonnegative(dt));
}

struct EventResult {
    bool event;
    double intensity;
};

// lambda = rate exp(z); between events dz/dt = correction*rate,
// and an event subtracts correction. Only the explicit ceiling caps z.
inline EventResult selfCorrectingStep(PCG32& rng, double& z, double rate,
                                     double correction, double maximum, double dt)
{
    rate = nonnegative(rate);
    correction = nonnegative(correction);
    maximum = nonnegative(maximum);
    if (rate == 0.0 || maximum == 0.0)
        return { false, 0.0 };
    double intensity;
    if (correction == 0.0) {
        z = 0.0;
        intensity = std::min(rate, maximum);
    } else {
        const double ceiling = std::log(maximum) - std::log(rate);
        z = std::min(finiteOr(z, 0.0) + correction * rate * dt, ceiling);
        intensity = std::min(std::exp(std::log(rate) + z), maximum);
    }
    const bool event = openUniformDouble(rng) < eventProbability(intensity, dt);
    if (event)
        z -= correction;
    return { event, intensity };
}

struct TelegraphState {
    bool high = false;
    double hazard = 1.0;
    double debt = 0.0;

    void reset(PCG32& rng, bool initial)
    {
        high = initial;
        hazard = exponentialHazardThreshold(rng);
        debt = 0.0;
    }

    int step(PCG32& rng, double up, double down, double dt)
    {
        double remaining = dt + debt;
        debt = 0.0;
        int transitions = 0;
        for (; transitions < kMaxTelegraphTransitionsPerCalc; ++transitions) {
            const double rate = nonnegative(high ? down : up);
            if (rate == 0.0)
                return transitions;
            const double wait = hazard / rate;
            if (wait > remaining) {
                hazard -= rate * remaining;
                return transitions;
            }
            remaining = std::max(remaining - wait, 0.0);
            high = !high;
            hazard = exponentialHazardThreshold(rng);
        }
        debt = remaining;
        return transitions;
    }
};

struct ShotState {
    double value = 0.0;
    double hazard = 1.0;
    double debt = 0.0;

    void reset(PCG32& rng)
    {
        value = 0.0;
        hazard = exponentialHazardThreshold(rng);
        debt = 0.0;
    }

    int step(PCG32& rng, double rate, double decay, double mean, double deviation, double dt)
    {
        rate = nonnegative(rate);
        decay = nonnegative(decay);
        mean = finiteOr(mean, 0.0);
        deviation = std::abs(finiteOr(deviation, 0.0));
        double remaining = dt + debt;
        debt = 0.0;
        if (decay == 0.0)
            value = 0.0;
        int events = 0;
        for (; events < kMaxShotEventsPerCalc; ++events) {
            const double wait = rate > 0.0 ? hazard / rate
                                          : std::numeric_limits<double>::infinity();
            if (wait > remaining) {
                if (decay > 0.0)
                    value *= std::exp(-remaining / decay);
                hazard -= rate * remaining;
                break;
            }
            if (decay > 0.0)
                value *= std::exp(-wait / decay);
            remaining = std::max(remaining - wait, 0.0);
            value += mean + (deviation > 0.0 ? deviation * normalSample(rng) : 0.0);
            hazard = exponentialHazardThreshold(rng);
        }
        if (events == kMaxShotEventsPerCalc)
            debt = remaining;
        if (!finite(value) || std::abs(value) > std::numeric_limits<float>::max())
            reset(rng);
        return events;
    }
};

} // namespace sd
