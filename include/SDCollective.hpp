#pragma once
#include "SDCircular.hpp"

namespace sd {
constexpr int kMaxProcessSubsteps = 16;
constexpr int kMaxKuramotoOscillators = 64;

inline bool representableState(double x)
{
    return finite(x) && std::abs(x) <= std::numeric_limits<float>::max();
}

// dx = (a*x-b*x^3+drive)dt + |noise|dW. No conditioning of finite raw state.
inline double doubleWellStep(PCG32& rng, double x, double drive, double a, double b,
                             double noise, double timeScale, double initial, int substeps, double dt)
{
    if (!(timeScale > 0.0) || !finite(timeScale)) return x;
    const double h = dt*timeScale/substeps;
    drive = finiteOr(drive, 0.0); a = finiteOr(a, 0.0); b = finiteOr(b, 0.0);
    noise = std::abs(finiteOr(noise, 0.0));
    for (int i = 0; i < substeps; ++i) {
        x += (a*x-b*x*x*x+drive)*h + (noise > 0.0 ? noise*std::sqrt(h)*normalSample(rng) : 0.0);
        if (!representableState(x)) return finiteOr(initial, 0.0);
    }
    return x;
}

struct OrderParameter {
    double cosine, sine;
    double r() const { return clamp(std::hypot(cosine, sine), 0.0, 1.0); }
    double psi() const { return wrapAngle(std::atan2(sine, cosine)); }
};

inline OrderParameter kuramotoOrder(const double* phases, int n)
{
    OrderParameter result { 0.0, 0.0 };
    for (int i = 0; i < n; ++i) {
        result.cosine += std::cos(phases[i]);
        result.sine += std::sin(phases[i]);
    }
    result.cosine /= n; result.sine /= n;
    return result;
}

inline bool kuramotoStep(PCG32& rng, double* phases, const double* omega, double* next,
                         int n, double coupling, double noise, double timeScale, int substeps, double dt)
{
    if (!(timeScale > 0.0) || !finite(timeScale)) return true;
    const double h = dt*timeScale/substeps;
    coupling = finiteOr(coupling, 0.0);
    noise = std::abs(finiteOr(noise, 0.0));
    for (int step = 0; step < substeps; ++step) {
        const auto order = kuramotoOrder(phases, n);
        for (int i = 0; i < n; ++i) {
            const double drift = omega[i] + coupling*(order.sine*std::cos(phases[i])-order.cosine*std::sin(phases[i]));
            const double value = phases[i] + drift*h
                + (noise > 0.0 ? noise*std::sqrt(h)*normalSample(rng) : 0.0);
            if (!finite(value)) return false;
            next[i] = wrapAngle(value);
        }
        std::copy(next, next+n, phases);
    }
    return true;
}
} // namespace sd
