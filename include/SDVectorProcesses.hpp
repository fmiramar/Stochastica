#pragma once
#include "SDBuffer.hpp"
#include "SDEventProcesses.hpp"

namespace sd {
constexpr int kMaxMultiOUChannels = 32;
constexpr int kMaxHawkesProcesses = 16;

// Same factorization, eight jitter attempts and diagonal fallback as
// MultiGaussNoise. Retain exact silence for an exactly zero covariance.
inline bool factorCovariance(const double* matrix, double* factor, int n)
{
    bool allZero = true;
    double largest = 0.0;
    for (int i = 0; i < n*n; ++i) allZero = allZero && matrix[i] == 0.0;
    if (allZero) { std::fill(factor, factor+n*n, 0.0); return true; }
    for (int i = 0; i < n; ++i) largest = std::max(largest, std::abs(matrix[i*n+i]));
    bool ok = choleskyFactor(matrix, factor, n);
    for (int attempt = 0; attempt < 8 && !ok; ++attempt)
        ok = choleskyFactor(matrix, factor, n, std::max(largest, 1.0)*1e-12*std::pow(10.0, attempt));
    if (!ok) {
        std::fill(factor, factor+n*n, 0.0);
        for (int i = 0; i < n; ++i) factor[i*n+i] = std::sqrt(nonnegative(matrix[i*n+i]));
    }
    return ok;
}

inline void ouInnovationCovariance(double* covariance, const double* theta, double* decay, int n, double dt)
{
    for (int i = 0; i < n; ++i) {
        decay[i] = std::exp(-theta[i]*dt);
        for (int j = 0; j <= i; ++j) {
            const double sigma = 0.5*(covariance[i*n+j]+covariance[j*n+i]);
            const double sum = theta[i]+theta[j];
            const double q = sigma * (sum > 0.0 ? -std::expm1(-sum*dt)/sum : dt);
            covariance[i*n+j] = covariance[j*n+i] = q;
        }
    }
}

inline void multiOUStep(PCG32& rng, double* values, const double* means,
                        const double* decay, const double* factor, double* normals, int n)
{
    for (int i = 0; i < n; ++i) normals[i] = normalSample(rng);
    bool valid = true;
    for (int i = 0; i < n; ++i) {
        double value = means[i] + decay[i]*(values[i]-means[i]);
        for (int j = 0; j <= i; ++j) value += factor[i*n+j]*normals[j];
        values[i] = value;
        valid = valid && finite(value) && std::abs(value) <= std::numeric_limits<float>::max();
    }
    if (!valid) std::copy(means, means+n, values);
}

// Separate decision and excitation phases make the update simultaneous.
inline void hawkesIntensities(double* history, const double* base, const double* decay,
                              double* intensities, int n, double maximum)
{
    for (int i = 0; i < n; ++i) {
        double value = base[i];
        for (int j = 0; j < n; ++j) {
            const int k = i*n+j;
            history[k] *= decay[k];
            value += history[k];
        }
        intensities[i] = std::min(value, maximum);
    }
}

inline void hawkesExcite(double* history, const double* alpha, const double* events, int n)
{
    bool valid = true;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            const int k = i*n+j;
            history[k] += alpha[k]*events[j];
            valid = valid && finite(history[k]);
        }
    if (!valid) std::fill(history, history+n*n, 0.0);
}

inline void multiHawkesStep(PCG32& rng, double* history, const double* base,
                            const double* alpha, const double* decay, double* intensities,
                            double* events, int n, double maximum, double dt)
{
    hawkesIntensities(history, base, decay, intensities, n, nonnegative(maximum));
    for (int i = 0; i < n; ++i)
        events[i] = openUniformDouble(rng) < eventProbability(intensities[i], dt) ? 1.0 : 0.0;
    hawkesExcite(history, alpha, events, n);
}
} // namespace sd
