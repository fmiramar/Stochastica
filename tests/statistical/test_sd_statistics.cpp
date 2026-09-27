#include "SDBuffer.hpp"
#include "SDDistributions.hpp"
#include "SDRandom.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void requireNear(double actual, double expected, double tolerance, const char* message)
{
    if (std::abs(actual - expected) > tolerance)
        throw std::runtime_error(message);
}

struct Moments {
    double mean = 0.0;
    double variance = 0.0;
};

template <typename Sampler>
Moments measure(int count, Sampler sampler)
{
    double sum = 0.0;
    double sumSquares = 0.0;
    for (int index = 0; index < count; ++index) {
        const double value = sampler();
        sum += value;
        sumSquares += value * value;
    }
    const double mean = sum / static_cast<double>(count);
    return { mean, sumSquares / static_cast<double>(count) - mean * mean };
}

double renewalSample(sd::PCG32& rng, int distribution, double rate, double shape)
{
    const double mean = 1.0 / rate;
    switch (distribution) {
    case 1:
        return sd::gammaSample(rng, shape, mean / shape);
    case 2:
        return sd::weibullSample(rng, shape, mean / std::tgamma(1.0 + 1.0 / shape));
    case 3:
        return sd::logNormalSample(
            rng, std::log(mean) - 0.5 * shape * shape, shape);
    case 4: {
        const double erlang = std::round(shape);
        return sd::gammaSample(rng, erlang, mean / erlang);
    }
    default:
        return sd::exponentialSample(rng, mean);
    }
}

double fractionalIncrementLagCorrelation(double hurst, uint32_t seed)
{
    constexpr int memory = 256;
    constexpr int count = 120000;
    std::vector<double> coefficients(memory);
    std::vector<double> innovations(memory);
    const double d = hurst - 0.5;
    coefficients[0] = 1.0;
    double sumSquares = 1.0;
    for (int index = 1; index < memory; ++index) {
        coefficients[index] = coefficients[index - 1]
            * (static_cast<double>(index - 1) + d) / static_cast<double>(index);
        sumSquares += coefficients[index] * coefficients[index];
    }
    const double normalization = 1.0 / std::sqrt(sumSquares);
    for (double& coefficient : coefficients)
        coefficient *= normalization;

    sd::PCG32 rng {};
    rng.setSeed(seed);
    int writeIndex = 0;
    double previous = 0.0;
    double sum = 0.0;
    double sumPrevious = 0.0;
    double sumSquaresCurrent = 0.0;
    double sumSquaresPrevious = 0.0;
    double cross = 0.0;
    for (int sample = 0; sample < count; ++sample) {
        innovations[writeIndex] = sd::normalSample(rng);
        double increment = 0.0;
        int readIndex = writeIndex;
        for (int lag = 0; lag < memory; ++lag) {
            increment += coefficients[lag] * innovations[readIndex];
            if (--readIndex < 0)
                readIndex = memory - 1;
        }
        writeIndex = (writeIndex + 1) % memory;
        if (sample > 0) {
            sum += increment;
            sumPrevious += previous;
            sumSquaresCurrent += increment * increment;
            sumSquaresPrevious += previous * previous;
            cross += increment * previous;
        }
        previous = increment;
    }
    const double observations = static_cast<double>(count - 1);
    const double mean = sum / observations;
    const double meanPrevious = sumPrevious / observations;
    const double covariance = cross / observations - mean * meanPrevious;
    const double variance = sumSquaresCurrent / observations - mean * mean;
    const double variancePrevious =
        sumSquaresPrevious / observations - meanPrevious * meanPrevious;
    return covariance / std::sqrt(variance * variancePrevious);
}

} // namespace

int main()
{
    try {
        constexpr int count = 200000;
        sd::PCG32 rng {};

        rng.setSeed(1001U);
        const Moments gamma = measure(count, [&] { return sd::gammaSample(rng, 2.0, 3.0); });
        requireNear(gamma.mean, 6.0, 0.06, "gamma mean");
        requireNear(gamma.variance, 18.0, 0.35, "gamma variance");

        rng.setSeed(1002U);
        const Moments weibull = measure(count, [&] { return sd::weibullSample(rng, 1.5, 2.0); });
        const double weibullMean = 2.0 * std::tgamma(1.0 + 1.0 / 1.5);
        requireNear(weibull.mean, weibullMean, 0.025, "Weibull mean");

        rng.setSeed(1003U);
        const Moments lognormal =
            measure(count, [&] { return sd::logNormalSample(rng, 0.2, 0.4); });
        requireNear(lognormal.mean, std::exp(0.2 + 0.5 * 0.16), 0.02, "lognormal mean");

        rng.setSeed(1004U);
        const Moments poisson = measure(
            count, [&] { return static_cast<double>(sd::poissonSample(rng, 40.0)); });
        requireNear(poisson.mean, 40.0, 0.08, "large Poisson mean");
        requireNear(poisson.variance, 40.0, 0.35, "large Poisson variance");

        rng.setSeed(1005U);
        const Moments geometric =
            measure(count, [&] { return sd::geometricSample(rng, 0.3); });
        requireNear(geometric.mean, (1.0 - 0.3) / 0.3, 0.035, "geometric mean");

        rng.setSeed(1006U);
        const Moments truncated = measure(
            count, [&] { return sd::truncatedNormalSample(rng, 0.0, 1.0, -1.0, 1.0); });
        requireNear(truncated.mean, 0.0, 0.01, "symmetric truncated-normal mean");
        require(truncated.variance < 1.0 && truncated.variance > 0.2,
                "truncated-normal variance range");

        for (int distribution = 0; distribution <= 4; ++distribution) {
            rng.setSeed(static_cast<uint32_t>(2000 + distribution));
            const Moments intervals = measure(
                count, [&] { return renewalSample(rng, distribution, 8.0, 1.7); });
            requireNear(intervals.mean, 0.125, 0.003, "renewal interval mean");
        }

        {
            rng.setSeed(3001U);
            constexpr double meanTarget = 1.5;
            constexpr double reversion = 2.0;
            constexpr double diffusion = 0.7;
            constexpr double dt = 0.01;
            const double decay = std::exp(-reversion * dt);
            const double noiseScale = std::sqrt(
                diffusion * diffusion * (1.0 - decay * decay) / (2.0 * reversion));
            double state = -4.0;
            for (int warmup = 0; warmup < 20000; ++warmup)
                state = meanTarget + (state - meanTarget) * decay
                    + noiseScale * sd::normalSample(rng);
            double sum = 0.0;
            double sumSquares = 0.0;
            double cross = 0.0;
            double previous = state;
            for (int index = 0; index < 500000; ++index) {
                state = meanTarget + (state - meanTarget) * decay
                    + noiseScale * sd::normalSample(rng);
                sum += state;
                sumSquares += state * state;
                cross += state * previous;
                previous = state;
            }
            const double samples = 500000.0;
            const double mean = sum / samples;
            const double variance = sumSquares / samples - mean * mean;
            const double autocorrelation = (cross / samples - mean * mean) / variance;
            requireNear(mean, meanTarget, 0.025, "OU stationary mean");
            requireNear(variance, diffusion * diffusion / (2.0 * reversion), 0.015,
                        "OU stationary variance");
            requireNear(autocorrelation, decay, 0.008, "OU lag-one autocorrelation");
        }

        {
            rng.setSeed(4001U);
            constexpr double dt = 0.001;
            constexpr int steps = 3000000;
            double excitationState = 0.0;
            int events = 0;
            for (int index = 0; index < steps; ++index) {
                excitationState *= std::exp(-10.0 * dt);
                const double intensity = std::min(2.0 + excitationState, 1000.0);
                if (static_cast<double>(sd::uniform01ClosedOpen(rng))
                    < 1.0 - std::exp(-intensity * dt)) {
                    ++events;
                    excitationState += 3.0;
                }
            }
            const double rate = static_cast<double>(events) / (steps * dt);
            requireNear(rate, 2.0 / (1.0 - 0.3), 0.2, "subcritical Hawkes rate");
        }

        {
            const double covariance[4] { 1.0, 0.8, 0.8, 2.0 };
            double factor[4] {};
            require(sd::choleskyFactor(covariance, factor, 2), "covariance Cholesky");
            rng.setSeed(5001U);
            double sumX = 0.0;
            double sumY = 0.0;
            double sumXX = 0.0;
            double sumYY = 0.0;
            double sumXY = 0.0;
            for (int index = 0; index < count; ++index) {
                const double z0 = sd::normalSample(rng);
                const double z1 = sd::normalSample(rng);
                const double x = factor[0] * z0;
                const double y = factor[2] * z0 + factor[3] * z1;
                sumX += x;
                sumY += y;
                sumXX += x * x;
                sumYY += y * y;
                sumXY += x * y;
            }
            const double samples = static_cast<double>(count);
            const double meanX = sumX / samples;
            const double meanY = sumY / samples;
            requireNear(sumXX / samples - meanX * meanX, 1.0, 0.02, "covariance xx");
            requireNear(sumYY / samples - meanY * meanY, 2.0, 0.03, "covariance yy");
            requireNear(sumXY / samples - meanX * meanY, 0.8, 0.02, "covariance xy");
        }

        {
            const double antiPersistent = fractionalIncrementLagCorrelation(0.2, 6001U);
            const double brownian = fractionalIncrementLagCorrelation(0.5, 6002U);
            const double persistent = fractionalIncrementLagCorrelation(0.8, 6003U);
            require(antiPersistent < brownian && brownian < persistent,
                    "fractional increment persistence ordering");
            require(antiPersistent < -0.05, "anti-persistent lag correlation");
            require(std::abs(brownian) < 0.03, "Brownian lag correlation");
            require(persistent > 0.05, "persistent lag correlation");
        }

        std::cout << "StochasticDynamics statistical tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "StochasticDynamics statistical test failure: " << error.what() << '\n';
        return 1;
    }
}

