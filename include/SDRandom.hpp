#pragma once

#include "SDCommon.hpp"

#include <cmath>
#include <cstdint>
#include <limits>

namespace sd {

struct PCG32 {
    uint64_t state = 0U;
    uint64_t increment = 0xda3e39cb94b95bdbULL;
    bool hasSpareNormal = false;
    double spareNormal = 0.0;
    uint64_t fallbackCount = 0U;

    void setSeed(uint32_t seed)
    {
        if (seed == 0U)
            seed = 0x853c49e6U;
        state = 0U;
        increment = 0xda3e39cb94b95bdbULL;
        hasSpareNormal = false;
        spareNormal = 0.0;
        fallbackCount = 0U;
        nextUInt();
        state += static_cast<uint64_t>(seed);
        nextUInt();
    }

    uint32_t nextUInt()
    {
        const uint64_t oldState = state;
        state = oldState * 6364136223846793005ULL + (increment | 1ULL);
        const uint32_t xorShifted = static_cast<uint32_t>(((oldState >> 18U) ^ oldState) >> 27U);
        const uint32_t rotation = static_cast<uint32_t>(oldState >> 59U);
        return (xorShifted >> rotation) | (xorShifted << ((-rotation) & 31U));
    }
};

inline float uniform01ClosedOpen(PCG32& rng)
{
    return static_cast<float>(static_cast<double>(rng.nextUInt()) * (1.0 / 4294967296.0));
}

inline float uniform01Open(PCG32& rng)
{
    return static_cast<float>((static_cast<double>(rng.nextUInt()) + 0.5) * (1.0 / 4294967296.0));
}

inline float uniformSigned(PCG32& rng)
{
    return 2.0f * uniform01ClosedOpen(rng) - 1.0f;
}

inline double normalSample(PCG32& rng)
{
    if (rng.hasSpareNormal) {
        rng.hasSpareNormal = false;
        return rng.spareNormal;
    }

    const double radius = std::sqrt(-2.0 * std::log(static_cast<double>(uniform01Open(rng))));
    const double angle = kTwoPi * static_cast<double>(uniform01Open(rng));
    rng.spareNormal = radius * std::sin(angle);
    rng.hasSpareNormal = true;
    return radius * std::cos(angle);
}

inline double exponentialSample(PCG32& rng, double mean)
{
    if (!(mean > 0.0) || !finite(mean))
        return 0.0;
    return -mean * std::log(static_cast<double>(uniform01Open(rng)));
}

inline double gammaSample(PCG32& rng, double shape, double scale)
{
    if (!(shape > 0.0) || !(scale >= 0.0) || !finite(shape) || !finite(scale))
        return finite(shape * scale) && shape * scale >= 0.0 ? shape * scale : 0.0;
    if (scale == 0.0)
        return 0.0;

    if (shape < 1.0) {
        const double raised = gammaSample(rng, shape + 1.0, 1.0);
        const double result = scale * raised
            * std::pow(static_cast<double>(uniform01Open(rng)), 1.0 / shape);
        return finite(result) ? result : shape * scale;
    }

    const double d = shape - 1.0 / 3.0;
    const double c = 1.0 / std::sqrt(9.0 * d);
    for (int attempt = 0; attempt < kMaxRejectionAttempts; ++attempt) {
        const double x = normalSample(rng);
        const double base = 1.0 + c * x;
        if (base <= 0.0)
            continue;
        const double v = base * base * base;
        const double u = static_cast<double>(uniform01Open(rng));
        if (u < 1.0 - 0.0331 * x * x * x * x
            || std::log(u) < 0.5 * x * x + d * (1.0 - v + std::log(v))) {
            const double result = scale * d * v;
            return finite(result) ? result : shape * scale;
        }
    }
    ++rng.fallbackCount;
    return shape * scale;
}

inline uint32_t poissonSample(PCG32& rng, double lambda)
{
    if (!(lambda > 0.0) || !finite(lambda))
        return 0U;

    if (lambda < 30.0) {
        const double threshold = std::exp(-lambda);
        double product = 1.0;
        for (uint32_t k = 1U; k <= static_cast<uint32_t>(kMaxRejectionAttempts); ++k) {
            product *= static_cast<double>(uniform01Open(rng));
            if (product <= threshold)
                return k - 1U;
        }
        ++rng.fallbackCount;
        return static_cast<uint32_t>(std::llround(lambda));
    }

    const double slam = std::sqrt(lambda);
    const double logLambda = std::log(lambda);
    const double b = 0.931 + 2.53 * slam;
    const double a = -0.059 + 0.02483 * b;
    const double inverseAlpha = 1.1239 + 1.1328 / (b - 3.4);
    const double vr = 0.9277 - 3.6224 / (b - 2.0);

    for (int attempt = 0; attempt < kMaxRejectionAttempts; ++attempt) {
        const double u = static_cast<double>(uniform01ClosedOpen(rng)) - 0.5;
        const double v = static_cast<double>(uniform01Open(rng));
        const double us = 0.5 - std::abs(u);
        if (us <= 0.0)
            continue;
        const double candidate = std::floor((2.0 * a / us + b) * u + lambda + 0.43);
        if (candidate < 0.0)
            continue;
        if (us >= 0.07 && v <= vr)
            return static_cast<uint32_t>(candidate);
        if (us < 0.013 && v > us)
            continue;
        const double lhs = std::log(v * inverseAlpha / (a / (us * us) + b));
        const double rhs = -lambda + candidate * logLambda - std::lgamma(candidate + 1.0);
        if (lhs <= rhs)
            return static_cast<uint32_t>(candidate);
    }

    ++rng.fallbackCount;
    return static_cast<uint32_t>(std::min(
        std::round(lambda), static_cast<double>(std::numeric_limits<uint32_t>::max())));
}

} // namespace sd
