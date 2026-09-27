#pragma once

#include "SDRandom.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace sd {

inline double normalCDF(double value)
{
    return 0.5 * std::erfc(-value / std::sqrt(2.0));
}

inline double inverseNormalCDF(double probability)
{
    probability = clamp(probability, std::numeric_limits<double>::min(),
                        1.0 - std::numeric_limits<double>::epsilon());
    constexpr double a[] = {
        -3.969683028665376e+01, 2.209460984245205e+02,
        -2.759285104469687e+02, 1.383577518672690e+02,
        -3.066479806614716e+01, 2.506628277459239e+00
    };
    constexpr double b[] = {
        -5.447609879822406e+01, 1.615858368580409e+02,
        -1.556989798598866e+02, 6.680131188771972e+01,
        -1.328068155288572e+01
    };
    constexpr double c[] = {
        -7.784894002430293e-03, -3.223964580411365e-01,
        -2.400758277161838e+00, -2.549732539343734e+00,
        4.374664141464968e+00, 2.938163982698783e+00
    };
    constexpr double d[] = {
        7.784695709041462e-03, 3.224671290700398e-01,
        2.445134137142996e+00, 3.754408661907416e+00
    };
    constexpr double low = 0.02425;
    constexpr double high = 1.0 - low;

    if (probability < low) {
        const double q = std::sqrt(-2.0 * std::log(probability));
        return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5])
            / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    if (probability > high) {
        const double q = std::sqrt(-2.0 * std::log(1.0 - probability));
        return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5])
            / ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    const double q = probability - 0.5;
    const double r = q * q;
    return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q
        / (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
}

inline double weibullSample(PCG32& rng, double shape, double scale)
{
    if (!(shape > 0.0) || !(scale >= 0.0) || !finite(shape) || !finite(scale))
        return 0.0;
    return scale * std::pow(-std::log(static_cast<double>(uniform01Open(rng))), 1.0 / shape);
}

inline double logNormalSample(PCG32& rng, double mu, double sigma)
{
    const double exponent = finiteOr(mu, 0.0) + std::abs(finiteOr(sigma, 0.0)) * normalSample(rng);
    if (exponent >= std::log(static_cast<double>(std::numeric_limits<float>::max())))
        return std::numeric_limits<float>::max();
    return std::exp(exponent);
}

inline double cauchySample(PCG32& rng, double location, double scale)
{
    return finiteOr(location, 0.0)
        + std::abs(finiteOr(scale, 0.0))
            * std::tan(kPi * (static_cast<double>(uniform01Open(rng)) - 0.5));
}

inline double geometricSample(PCG32& rng, double probability)
{
    if (!(probability > 0.0))
        return kLargestExactIntegerFloat;
    if (probability >= 1.0)
        return 0.0;
    return std::floor(std::log(static_cast<double>(uniform01Open(rng)))
                      / std::log1p(-probability));
}

inline double truncatedNormalSample(
    PCG32& rng, double mean, double deviation, double low, double high)
{
    if (low > high)
        std::swap(low, high);
    mean = finiteOr(mean, 0.0);
    deviation = std::abs(finiteOr(deviation, 0.0));
    if (deviation == 0.0)
        return clamp(mean, low, high);
    const double a = normalCDF((low - mean) / deviation);
    const double b = normalCDF((high - mean) / deviation);
    if (!(b > a) || !finite(a) || !finite(b))
        return std::abs(mean - low) <= std::abs(mean - high) ? low : high;
    const double probability = a + static_cast<double>(uniform01Open(rng)) * (b - a);
    const double result = mean + deviation * inverseNormalCDF(probability);
    return finite(result) ? clamp(result, low, high)
                          : (std::abs(mean - low) <= std::abs(mean - high) ? low : high);
}

inline double stableS0Sample(
    PCG32& rng, double alpha, double beta, double scale, double location)
{
    alpha = clamp(alpha, 0.01, 2.0);
    beta = clamp(beta, -1.0, 1.0);
    scale = std::abs(finiteOr(scale, 0.0));
    location = finiteOr(location, 0.0);
    if (scale == 0.0)
        return location;
    if (alpha == 2.0)
        return location + scale * std::sqrt(2.0) * normalSample(rng);

    const double v = kPi * (static_cast<double>(uniform01Open(rng)) - 0.5);
    const double w = -std::log(static_cast<double>(uniform01Open(rng)));
    double standard = 0.0;
    if (std::abs(alpha - 1.0) > 1.0e-8) {
        const double tangent = std::tan(0.5 * kPi * alpha);
        const double b = std::atan(beta * tangent) / alpha;
        const double s = std::pow(1.0 + beta * beta * tangent * tangent, 0.5 / alpha);
        const double numerator = std::sin(alpha * (v + b));
        const double denominator = std::pow(std::cos(v), 1.0 / alpha);
        const double factor = std::pow(
            std::cos(v - alpha * (v + b)) / w, (1.0 - alpha) / alpha);
        const double s1 = s * numerator / denominator * factor;
        standard = s1 - beta * tangent;
    } else {
        const double halfPi = 0.5 * kPi;
        standard = (2.0 / kPi)
            * ((halfPi + beta * v) * std::tan(v)
               - beta * std::log((halfPi * w * std::cos(v)) / (halfPi + beta * v)));
    }
    return location + scale * standard;
}

inline double levySample(PCG32& rng, double location, double scale)
{
    scale = finiteOr(scale, 0.0);
    if (scale < 0.0)
        return finiteOr(location, 0.0);
    for (int attempt = 0; attempt < kMaxRejectionAttempts; ++attempt) {
        const double z = normalSample(rng);
        if (std::abs(z) > std::numeric_limits<double>::epsilon()) {
            const double result = location + scale / (z * z);
            return finite(result) ? result : std::numeric_limits<float>::max();
        }
    }
    ++rng.fallbackCount;
    return std::numeric_limits<float>::max();
}

} // namespace sd
