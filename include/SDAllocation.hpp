#pragma once
#include "SDCategorical.hpp"
#include "SDEventProcesses.hpp"

namespace sd {
constexpr int kMaxDirichletChannels = 32;
constexpr int kMaxUrnStates = 64;
constexpr double kUrnRescaleTotal = 1099511627776.0; // 2^40

inline void concentrationMean(const double* alpha, double* values, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += alpha[i];
    for (int i = 0; i < n; ++i) values[i] = alpha[i]/sum;
}

inline void dirichletSample(PCG32& rng, const double* alpha, double* values, int n)
{
    double largest = -std::numeric_limits<double>::infinity();
    for (int i = 0; i < n; ++i) {
        // Same shape-boost identity as gammaSample, evaluated in log space.
        // The rejection sampler itself is the existing bounded primitive.
        values[i] = alpha[i] < 1.0
            ? std::log(gammaSample(rng, alpha[i]+1.0, 1.0))
                + std::log(openUniformDouble(rng))/alpha[i]
            : std::log(gammaSample(rng, alpha[i], 1.0));
        largest = std::max(largest, values[i]);
    }
    if (!finite(largest)) { concentrationMean(alpha, values, n); return; }
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        values[i] = std::exp(values[i]-largest);
        sum += values[i];
    }
    for (int i = 0; i < n; ++i) values[i] /= sum;
}

inline double weightTotal(const double* weights, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += weights[i];
    return sum;
}

inline double normalizedEntropy(const double* weights, int n)
{
    const double sum = weightTotal(weights, n);
    if (!(sum > 0.0)) return 0.0;
    double entropy = 0.0;
    for (int i = 0; i < n; ++i) {
        const double p = weights[i]/sum;
        if (p > 0.0) entropy -= p*std::log(p);
    }
    return clamp(entropy/std::log(double(n)), 0.0, 1.0);
}

inline void rescaleUrn(double* weights, int n, double& unitScale)
{
    const double sum = weightTotal(weights, n);
    if (sum > kUrnRescaleTotal) {
        for (int i = 0; i < n; ++i) weights[i] /= sum;
        // Future reinforcement must use the same units, preserving the model.
        unitScale /= sum;
    }
}

struct UrnDraw { int state; double probability, entropy; };
inline UrnDraw urnDraw(PCG32& rng, double* weights, int n, double reinforcement, double& unitScale)
{
    rescaleUrn(weights, n, unitScale);
    const int chosen = categoricalSample(rng, weights, n, 0, true);
    const double p = weights[chosen]/weightTotal(weights, n);
    weights[chosen] += nonnegative(reinforcement)*unitScale;
    rescaleUrn(weights, n, unitScale);
    return {chosen, p, normalizedEntropy(weights, n)};
}
} // namespace sd
