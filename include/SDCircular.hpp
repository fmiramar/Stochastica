#pragma once
#include "SDEventProcesses.hpp"
#include "SDRange.hpp"

namespace sd {
constexpr double kVonMisesUniformThreshold = 1e-8;
constexpr double kVonMisesNormalThreshold = 1e6;

inline double wrapAngle(double angle) { return wrapValue(finiteOr(angle, 0.0), -kPi, kPi); }

// Keep the float output inside the mathematical half-open interval too.
inline float angleFloat(double angle)
{
    const float insidePi = std::nextafter(static_cast<float>(kPi), 0.0f);
    return std::min(std::max(static_cast<float>(wrapAngle(angle)), -insidePi), insidePi);
}

// Best & Fisher (1979), section 4, wrapped-Cauchy rejection envelope.
inline double vonMisesSample(PCG32& rng, double mean, double kappa)
{
    mean = wrapAngle(mean);
    kappa = nonnegative(kappa);
    if (kappa <= kVonMisesUniformThreshold)
        return wrapAngle(mean + kTwoPi * (openUniformDouble(rng) - 0.5));
    if (kappa > kVonMisesNormalThreshold)
        return wrapAngle(mean + normalSample(rng) / std::sqrt(kappa));
    const double tau = 1.0 + std::hypot(1.0, 2.0*kappa);
    // Rationalized equivalent of (tau - sqrt(2*tau))/(2*kappa).
    const double rho = (2.0*kappa) / (tau + std::sqrt(2.0*tau));
    const double r = (1.0 + rho*rho) / (2.0*rho);
    for (int attempt = 0; attempt < kMaxRejectionAttempts; ++attempt) {
        const double z = std::cos(kPi * openUniformDouble(rng));
        const double f = clamp((1.0 + r*z)/(r+z), -1.0, 1.0);
        const double c = kappa * (r-f);
        const double u = openUniformDouble(rng);
        if (u < c*(2.0-c) || (c > 0.0 && std::log(c/u)+1.0-c >= 0.0)) {
            const double sign = openUniformDouble(rng) < 0.5 ? -1.0 : 1.0;
            return wrapAngle(mean + sign * std::acos(f));
        }
    }
    ++rng.fallbackCount;
    return mean;
}
} // namespace sd
