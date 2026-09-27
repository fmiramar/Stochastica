#include "SC_PlugIn.h"
#include "SDDistributions.hpp"
#include "SDRange.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <limits>

extern InterfaceTable* ft;

namespace {

struct OUProcess : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    double state = 0.0;
    sd::RisingEdge reset;
};

void OUProcess_next(OUProcess* unit, int inNumSamples)
{
    float* output = OUT(0);
    const double elapsed = SAMPLEDUR;
    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->reset.update(sd::inputAt(unit, 6, sample))) {
            unit->state = sd::finiteOr(sd::inputAt(unit, 4, sample), 0.0);
            unit->rng.setSeed(unit->originalSeed);
        } else {
            const double mean = sd::finiteOr(sd::inputAt(unit, 0, sample), 0.0);
            const double reversion = std::max(sd::finiteOr(sd::inputAt(unit, 1, sample), 0.0), 0.0);
            const double diffusion = sd::finiteOr(sd::inputAt(unit, 2, sample), 0.0);
            const double timeScale = std::max(sd::finiteOr(sd::inputAt(unit, 3, sample), 0.0), 0.0);
            const double dt = elapsed * timeScale;
            if (dt > 0.0) {
                if (reversion > 0.0) {
                    const double decay = std::exp(-reversion * dt);
                    const double variance = diffusion * diffusion
                        * (1.0 - std::exp(-2.0 * reversion * dt)) / (2.0 * reversion);
                    unit->state = mean + (unit->state - mean) * decay
                        + std::sqrt(std::max(variance, 0.0)) * sd::normalSample(unit->rng);
                } else {
                    unit->state += diffusion * std::sqrt(dt) * sd::normalSample(unit->rng);
                }
                if (!sd::finite(unit->state))
                    unit->state = sd::finiteOr(sd::inputAt(unit, 4, sample), 0.0);
            }
        }
        output[sample] = sd::finiteFloat(unit->state);
    }
}

void OUProcess_Ctor(OUProcess* unit)
{
    unit->originalSeed = sd::seedForUnit(unit, 5);
    unit->rng.setSeed(unit->originalSeed);
    unit->state = sd::finiteOr(IN0(4), 0.0);
    unit->reset.initialize(IN0(6));
    SETCALC(OUProcess_next);
    OUT0(0) = sd::finiteFloat(unit->state);
}

struct RenewalTrig : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    double countdown = DBL_MAX;
    double interval = FLT_MAX;
    bool enabled = false;
    sd::RisingEdge reset;
};

double renewalInterval(RenewalTrig* unit, int sample)
{
    const double rate = sd::finiteOr(sd::inputAt(unit, 0, sample), 0.0);
    if (!(rate > 0.0))
        return FLT_MAX;
    const int distribution = sd::clampInt(
        static_cast<int>(std::lround(sd::inputAt(unit, 1, sample))), 0, 4);
    const double shape = std::max(
        std::abs(sd::finiteOr(sd::inputAt(unit, 2, sample), 1.0)), 1.0e-6);
    const double mean = 1.0 / rate;
    double interval = mean;
    switch (distribution) {
    case 1:
        interval = sd::gammaSample(unit->rng, shape, mean / shape);
        break;
    case 2: {
        const double scale = mean / std::tgamma(1.0 + 1.0 / shape);
        interval = sd::weibullSample(unit->rng, shape, scale);
        break;
    }
    case 3: {
        const double sigma = shape;
        interval = sd::logNormalSample(unit->rng, std::log(mean) - 0.5 * sigma * sigma, sigma);
        break;
    }
    case 4: {
        const double erlang = static_cast<double>(std::max(1, static_cast<int>(std::lround(shape))));
        interval = sd::gammaSample(unit->rng, erlang, mean / erlang);
        break;
    }
    default:
        interval = sd::exponentialSample(unit->rng, mean);
        break;
    }
    interval += std::max(sd::finiteOr(sd::inputAt(unit, 3, sample), 0.0), 0.0);
    if (!(interval > 0.0) || !sd::finite(interval))
        interval = std::max(mean, SAMPLEDUR);
    return interval;
}

void resetRenewal(RenewalTrig* unit, int sample)
{
    unit->rng.setSeed(unit->originalSeed);
    unit->interval = renewalInterval(unit, sample);
    unit->countdown = unit->interval;
    unit->enabled = sd::inputAt(unit, 0, sample) > 0.0f;
}

void RenewalTrig_next(RenewalTrig* unit, int inNumSamples)
{
    float* trigger = OUT(0);
    float* intervalOutput = OUT(1);
    const double elapsed = SAMPLEDUR;
    for (int sample = 0; sample < inNumSamples; ++sample) {
        bool event = false;
        if (unit->reset.update(sd::inputAt(unit, 5, sample))) {
            resetRenewal(unit, sample);
        } else if (sd::inputAt(unit, 0, sample) > 0.0f) {
            if (!unit->enabled) {
                unit->interval = renewalInterval(unit, sample);
                unit->countdown = unit->interval;
                unit->enabled = true;
            } else {
                unit->countdown -= elapsed;
                if (unit->countdown <= 0.0) {
                    event = true;
                    unit->interval = renewalInterval(unit, sample);
                    unit->countdown += unit->interval;
                    if (!(unit->countdown > -FLT_MAX) || !sd::finite(unit->countdown))
                        unit->countdown = unit->interval;
                }
            }
        } else {
            unit->interval = FLT_MAX;
            unit->countdown = DBL_MAX;
            unit->enabled = false;
        }
        trigger[sample] = event ? 1.0f : 0.0f;
        intervalOutput[sample] = sd::finiteFloat(unit->interval);
    }
}

void RenewalTrig_Ctor(RenewalTrig* unit)
{
    unit->originalSeed = sd::seedForUnit(unit, 4);
    unit->reset.initialize(IN0(5));
    resetRenewal(unit, 0);
    SETCALC(RenewalTrig_next);
    OUT0(0) = 0.0f;
    OUT0(1) = sd::finiteFloat(unit->interval);
}

struct HawkesTrig : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    double excitationState = 0.0;
    sd::RisingEdge reset;
};

void HawkesTrig_next(HawkesTrig* unit, int inNumSamples)
{
    float* trigger = OUT(0);
    float* intensityOutput = OUT(1);
    const double elapsed = SAMPLEDUR;
    for (int sample = 0; sample < inNumSamples; ++sample) {
        bool event = false;
        double intensity = 0.0;
        if (unit->reset.update(sd::inputAt(unit, 5, sample))) {
            unit->excitationState = 0.0;
            unit->rng.setSeed(unit->originalSeed);
        } else {
            const double baseRate = std::max(sd::finiteOr(sd::inputAt(unit, 0, sample), 0.0), 0.0);
            const double excitation = std::max(sd::finiteOr(sd::inputAt(unit, 1, sample), 0.0), 0.0);
            const double decay = std::max(sd::finiteOr(sd::inputAt(unit, 2, sample), 0.0), 0.0);
            const double maximum = std::max(sd::finiteOr(sd::inputAt(unit, 3, sample), 0.0), 0.0);
            unit->excitationState *= std::exp(-decay * elapsed);
            intensity = sd::clipValue(baseRate + unit->excitationState, 0.0, maximum);
            const double probability = 1.0 - std::exp(-intensity * elapsed);
            if (static_cast<double>(sd::uniform01ClosedOpen(unit->rng)) < probability) {
                event = true;
                unit->excitationState += excitation;
            }
            if (!sd::finite(unit->excitationState))
                unit->excitationState = 0.0;
        }
        trigger[sample] = event ? 1.0f : 0.0f;
        intensityOutput[sample] = sd::finiteFloat(intensity);
    }
}

void HawkesTrig_Ctor(HawkesTrig* unit)
{
    unit->originalSeed = sd::seedForUnit(unit, 4);
    unit->rng.setSeed(unit->originalSeed);
    unit->excitationState = 0.0;
    unit->reset.initialize(IN0(5));
    SETCALC(HawkesTrig_next);
    OUT0(0) = 0.0f;
    OUT0(1) = sd::finiteFloat(sd::clipValue(
        std::max(sd::finiteOr(IN0(0), 0.0), 0.0), 0.0,
        std::max(sd::finiteOr(IN0(3), 0.0), 0.0)));
}

struct FractionalNoise : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    int quality = 12;
    double states[32] {};
    double decay[32] {};
    double innovation[32] {};
    double weights[32] {};
    double targetWeights[32] {};
    double weightSteps[32] {};
    double previousAlpha = -1.0;
    double previousMinimum = -1.0;
    double previousMaximum = -1.0;
    int weightRampRemaining = 0;
    bool coefficientsInitialized = false;
    sd::RisingEdge reset;
};

void updateFractionalCoefficients(FractionalNoise* unit)
{
    const double nyquist = std::max(0.5 * SAMPLERATE, 1.0e-5);
    const double alpha = sd::clamp(sd::finiteOr(IN0(0), 1.0), 0.0, 2.0);
    double minimum = sd::clamp(sd::finiteOr(IN0(1), 0.5), 1.0e-6, nyquist);
    double maximum = sd::clamp(sd::finiteOr(IN0(2), nyquist), 1.0e-6, nyquist);
    if (maximum <= minimum) {
        if (minimum < nyquist) {
            maximum = std::nextafter(minimum, nyquist);
        } else {
            minimum = std::max(1.0e-6, 0.5 * nyquist);
            maximum = nyquist;
        }
    }

    const bool changed = std::abs(alpha - unit->previousAlpha) > 1.0e-6
        || std::abs(minimum - unit->previousMinimum)
            > 1.0e-6 * std::max(minimum, 1.0)
        || std::abs(maximum - unit->previousMaximum)
            > 1.0e-6 * std::max(maximum, 1.0);
    if (!changed)
        return;

    const double ratio = unit->quality > 1
        ? std::pow(maximum / minimum, 1.0 / static_cast<double>(unit->quality - 1))
        : 1.0;
    double frequency = minimum;
    double sumSquares = 0.0;
    for (int component = 0; component < unit->quality; ++component) {
        const double lambda = sd::kTwoPi * frequency;
        unit->decay[component] = std::exp(-lambda * SAMPLEDUR);
        unit->innovation[component] =
            std::sqrt(std::max(0.0, 1.0 - unit->decay[component] * unit->decay[component]));
        unit->targetWeights[component] = std::sqrt(std::pow(lambda, 1.0 - alpha));
        sumSquares += unit->targetWeights[component] * unit->targetWeights[component];
        frequency *= ratio;
    }
    const double normalization = sumSquares > 0.0 ? 1.0 / std::sqrt(sumSquares) : 0.0;
    for (int component = 0; component < unit->quality; ++component)
        unit->targetWeights[component] *= normalization;
    if (!unit->coefficientsInitialized) {
        for (int component = 0; component < unit->quality; ++component) {
            unit->weights[component] = unit->targetWeights[component];
            unit->weightSteps[component] = 0.0;
        }
        unit->weightRampRemaining = 0;
        unit->coefficientsInitialized = true;
    } else {
        const int rampSamples = std::max(1, unit->mBufLength);
        for (int component = 0; component < unit->quality; ++component)
            unit->weightSteps[component] =
                (unit->targetWeights[component] - unit->weights[component])
                / static_cast<double>(rampSamples);
        unit->weightRampRemaining = rampSamples;
    }
    unit->previousAlpha = alpha;
    unit->previousMinimum = minimum;
    unit->previousMaximum = maximum;
}

void resetFractional(FractionalNoise* unit)
{
    unit->rng.setSeed(unit->originalSeed);
    std::fill(unit->states, unit->states + 32, 0.0);
}

void FractionalNoise_next(FractionalNoise* unit, int inNumSamples)
{
    updateFractionalCoefficients(unit);
    float* output = OUT(0);
    const bool white = unit->previousAlpha <= 1.0e-8;
    const double coloredBlend = sd::clipValue(unit->previousAlpha / 0.02, 0.0, 1.0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->weightRampRemaining > 0) {
            for (int component = 0; component < unit->quality; ++component)
                unit->weights[component] += unit->weightSteps[component];
            if (--unit->weightRampRemaining == 0) {
                for (int component = 0; component < unit->quality; ++component)
                    unit->weights[component] = unit->targetWeights[component];
            }
        }
        if (unit->reset.update(sd::inputAt(unit, 5, sample))) {
            resetFractional(unit);
            output[sample] = 0.0f;
            continue;
        }
        if (white) {
            output[sample] = static_cast<float>(
                std::sqrt(3.0) * static_cast<double>(sd::uniformSigned(unit->rng)));
            continue;
        }
        double result = 0.0;
        for (int component = 0; component < unit->quality; ++component) {
            const double noise = std::sqrt(3.0)
                * static_cast<double>(sd::uniformSigned(unit->rng));
            unit->states[component] = unit->states[component] * unit->decay[component]
                + noise * unit->innovation[component];
            result += unit->weights[component] * unit->states[component];
        }
        if (coloredBlend < 1.0) {
            const double whiteSample = std::sqrt(3.0)
                * static_cast<double>(sd::uniformSigned(unit->rng));
            result = whiteSample + coloredBlend * (result - whiteSample);
        }
        output[sample] = sd::finiteFloat(result);
    }
}

void FractionalNoise_Ctor(FractionalNoise* unit)
{
    unit->quality = sd::clampInt(static_cast<int>(std::lround(IN0(3))), 4, 32);
    unit->originalSeed = sd::seedForUnit(unit, 4);
    unit->previousAlpha = -1.0;
    unit->previousMinimum = -1.0;
    unit->previousMaximum = -1.0;
    unit->weightRampRemaining = 0;
    unit->coefficientsInitialized = false;
    unit->reset.initialize(IN0(5));
    std::fill(unit->states, unit->states + 32, 0.0);
    std::fill(unit->weights, unit->weights + 32, 0.0);
    std::fill(unit->targetWeights, unit->targetWeights + 32, 0.0);
    std::fill(unit->weightSteps, unit->weightSteps + 32, 0.0);
    resetFractional(unit);
    updateFractionalCoefficients(unit);
    SETCALC(FractionalNoise_next);
    OUT0(0) = 0.0f;
}

struct FBrownianMotion : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    int memorySize = 256;
    int writeIndex = 0;
    double* innovations = nullptr;
    double* coefficients = nullptr;
    double state = 0.0;
    double previousHurst = -1.0;
    sd::RisingEdge reset;
};

void updateFBrownianCoefficients(FBrownianMotion* unit)
{
    const double hurst = sd::clamp(sd::finiteOr(IN0(0), 0.5), 0.01, 0.99);
    if (std::abs(hurst - unit->previousHurst) <= 1.0e-4)
        return;
    const double d = hurst - 0.5;
    unit->coefficients[0] = 1.0;
    double sumSquares = 1.0;
    for (int index = 1; index < unit->memorySize; ++index) {
        unit->coefficients[index] = unit->coefficients[index - 1]
            * (static_cast<double>(index - 1) + d) / static_cast<double>(index);
        sumSquares += unit->coefficients[index] * unit->coefficients[index];
    }
    const double scale = 1.0 / std::sqrt(sumSquares);
    for (int index = 0; index < unit->memorySize; ++index)
        unit->coefficients[index] *= scale;
    unit->previousHurst = hurst;
}

void resetFBrownian(FBrownianMotion* unit, int sample)
{
    unit->rng.setSeed(unit->originalSeed);
    std::fill(unit->innovations, unit->innovations + unit->memorySize, 0.0);
    unit->writeIndex = 0;
    unit->state = sd::finiteOr(sd::inputAt(unit, 2, sample), 0.0);
}

void FBrownianMotion_next(FBrownianMotion* unit, int inNumSamples)
{
    updateFBrownianCoefficients(unit);
    float* output = OUT(0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->reset.update(sd::inputAt(unit, 5, sample))) {
            resetFBrownian(unit, sample);
        } else {
            unit->innovations[unit->writeIndex] = sd::normalSample(unit->rng);
            double increment = 0.0;
            int readIndex = unit->writeIndex;
            for (int lag = 0; lag < unit->memorySize; ++lag) {
                increment += unit->coefficients[lag] * unit->innovations[readIndex];
                if (--readIndex < 0)
                    readIndex = unit->memorySize - 1;
            }
            unit->writeIndex = (unit->writeIndex + 1) % unit->memorySize;
            const double hurst = sd::clamp(sd::finiteOr(sd::inputAt(unit, 0, sample), 0.5), 0.01, 0.99);
            const double timeScale = std::max(sd::finiteOr(sd::inputAt(unit, 1, sample), 0.0), 0.0);
            const double dt = SAMPLEDUR * timeScale;
            if (dt > 0.0)
                unit->state += increment * std::pow(dt, hurst);
            if (!sd::finite(unit->state))
                resetFBrownian(unit, sample);
        }
        output[sample] = sd::finiteFloat(unit->state);
    }
}

void FBrownianMotion_Ctor(FBrownianMotion* unit)
{
    unit->memorySize = sd::clampInt(static_cast<int>(std::lround(IN0(3))), 8, 4096);
    unit->innovations = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->memorySize));
    unit->coefficients = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->memorySize));
    unit->originalSeed = sd::seedForUnit(unit, 4);
    unit->reset.initialize(IN0(5));
    if (!unit->innovations || !unit->coefficients) {
        SETCALC(ClearUnitOutputs);
        ClearUnitOutputs(unit, 1);
        return;
    }
    unit->previousHurst = -1.0;
    updateFBrownianCoefficients(unit);
    resetFBrownian(unit, 0);
    SETCALC(FBrownianMotion_next);
    OUT0(0) = sd::finiteFloat(unit->state);
}

void FBrownianMotion_Dtor(FBrownianMotion* unit)
{
    sd::freeRT(unit, unit->innovations);
    sd::freeRT(unit, unit->coefficients);
}

struct BoundedWalk : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    double state = 0.0;
    bool absorbed = false;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
};

double boundInitial(double value, double low, double high, int boundary)
{
    switch (boundary) {
    case 1:
        return sd::wrapValue(value, low, high);
    case 2:
    case 3:
    case 4:
        return sd::clipValue(value, low, high);
    default:
        return sd::foldValue(value, low, high);
    }
}

double walkIncrement(BoundedWalk* unit, double step, int distribution)
{
    switch (distribution) {
    case 1:
        return step * sd::normalSample(unit->rng);
    case 2:
        return step * std::tan(
            sd::kPi * (static_cast<double>(sd::uniform01Open(unit->rng)) - 0.5));
    default:
        return step * static_cast<double>(sd::uniformSigned(unit->rng));
    }
}

void resetBoundedWalk(BoundedWalk* unit, int sample)
{
    unit->rng.setSeed(unit->originalSeed);
    unit->absorbed = false;
    const double low = sd::finiteOr(sd::inputAt(unit, 2, sample), -1.0);
    const double high = sd::finiteOr(sd::inputAt(unit, 3, sample), 1.0);
    const int boundary = sd::clampInt(
        static_cast<int>(std::lround(sd::inputAt(unit, 4, sample))), 0, 4);
    const double initial = sd::finiteOr(sd::inputAt(unit, 6, sample), 0.0);
    unit->state = high > low ? boundInitial(initial, low, high, boundary) : low;
    if (boundary == 4 && high > low)
        unit->absorbed = unit->state <= low || unit->state >= high;
}

void BoundedWalk_next(BoundedWalk* unit, int inNumSamples)
{
    float* output = OUT(0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, 8, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 0, sample));
        if (resetEdge) {
            resetBoundedWalk(unit, sample);
        } else if (triggerEdge) {
            const double low = sd::finiteOr(sd::inputAt(unit, 2, sample), -1.0);
            const double high = sd::finiteOr(sd::inputAt(unit, 3, sample), 1.0);
            if (!(high > low)) {
                unit->state = low;
            } else if (!unit->absorbed) {
                const double step = std::abs(sd::finiteOr(sd::inputAt(unit, 1, sample), 0.0));
                const int boundary = sd::clampInt(
                    static_cast<int>(std::lround(sd::inputAt(unit, 4, sample))), 0, 4);
                const int distribution = sd::clampInt(
                    static_cast<int>(std::lround(sd::inputAt(unit, 5, sample))), 0, 2);
                double candidate = unit->state + walkIncrement(unit, step, distribution);
                if (boundary == 3) {
                    bool accepted = candidate >= low && candidate <= high && sd::finite(candidate);
                    for (int attempt = 1; attempt < sd::kMaxRejectionAttempts && !accepted; ++attempt) {
                        candidate = unit->state + walkIncrement(unit, step, distribution);
                        accepted = candidate >= low && candidate <= high && sd::finite(candidate);
                    }
                    if (!accepted) {
                        ++unit->rng.fallbackCount;
                        candidate = sd::clipValue(sd::finiteOr(candidate, unit->state), low, high);
                    }
                } else if (boundary == 1) {
                    candidate = sd::wrapValue(candidate, low, high);
                } else if (boundary == 2) {
                    candidate = sd::clipValue(sd::finiteOr(candidate, unit->state), low, high);
                } else if (boundary == 4) {
                    if (!sd::finite(candidate))
                        candidate = unit->state;
                    if (candidate <= low) {
                        candidate = low;
                        unit->absorbed = true;
                    } else if (candidate >= high) {
                        candidate = high;
                        unit->absorbed = true;
                    }
                } else {
                    candidate = sd::foldValue(candidate, low, high);
                }
                unit->state = candidate;
            }
        }
        output[sample] = sd::finiteFloat(unit->state);
    }
}

void BoundedWalk_Ctor(BoundedWalk* unit)
{
    unit->originalSeed = sd::seedForUnit(unit, 7);
    unit->trigger.initialize(IN0(0));
    unit->reset.initialize(IN0(8));
    resetBoundedWalk(unit, 0);
    SETCALC(BoundedWalk_next);
    OUT0(0) = sd::finiteFloat(unit->state);
}

} // namespace

void registerSDProcesses()
{
    DefineSimpleUnit(OUProcess);
    DefineSimpleUnit(RenewalTrig);
    DefineSimpleUnit(HawkesTrig);
    DefineSimpleUnit(FractionalNoise);
    DefineDtorUnit(FBrownianMotion);
    DefineSimpleUnit(BoundedWalk);
}
