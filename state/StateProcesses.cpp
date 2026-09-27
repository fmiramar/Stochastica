#include "SC_PlugIn.h"
#include "SDBuffer.hpp"
#include "SDDistributions.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

extern InterfaceTable* ft;

namespace {

int initialState(Unit* unit, int inputIndex, int states, int sample)
{
    const double requested = sd::finiteOr(sd::inputAt(unit, inputIndex, sample), 0.0);
    return sd::clampInt(static_cast<int>(std::lround(requested)), 0, states - 1);
}

int sampleTransition(Unit* unit, sd::PCG32& rng, int currentState,
                     int states, int bufferInput)
{
    SndBuf* transition = sd::resolveBuffer(unit, IN0(bufferInput));
    const int required = states * states;
    if (!transition || !transition->data || transition->channels != 1
        || static_cast<int>(transition->samples) < required)
        return currentState;

    LOCK_SNDBUF_SHARED(transition);
    const float* row = transition->data + currentState * states;
    double sum = 0.0;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (sd::finite(weight) && weight > 0.0)
            sum += weight;
    }
    if (!(sum > 0.0) || !sd::finite(sum))
        return currentState;
    double target = static_cast<double>(sd::uniform01ClosedOpen(rng)) * sum;
    for (int destination = 0; destination < states; ++destination) {
        const double weight = row[destination];
        if (sd::finite(weight) && weight > 0.0) {
            target -= weight;
            if (target <= 0.0)
                return destination;
        }
    }
    return states - 1;
}

double stateValue(Unit* unit, int state, int states, int bufferInput)
{
    const float number = IN0(bufferInput);
    if (!std::isfinite(number) || number < 0.0f)
        return static_cast<double>(state);
    SndBuf* values = sd::resolveBuffer(unit, number);
    if (!values || !values->data || values->channels != 1
        || static_cast<int>(values->samples) < states)
        return static_cast<double>(state);
    LOCK_SNDBUF_SHARED(values);
    return sd::finiteOr(values->data[state], static_cast<double>(state));
}

struct MarkovChain : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    int states = 2;
    int state = 0;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
    bool warned = false;
};

void MarkovChain_next(MarkovChain* unit, int inNumSamples)
{
    float* stateOutput = OUT(0);
    float* valueOutput = OUT(1);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, 6, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 1, sample));
        if (resetEdge) {
            unit->rng.setSeed(unit->originalSeed);
            unit->state = initialState(unit, 4, unit->states, sample);
        } else if (triggerEdge) {
            unit->state = sampleTransition(unit, unit->rng, unit->state, unit->states, 2);
        }
        stateOutput[sample] = static_cast<float>(unit->state);
        valueOutput[sample] = sd::finiteFloat(stateValue(unit, unit->state, unit->states, 3));
    }
}

void MarkovChain_Ctor(MarkovChain* unit)
{
    unit->states = sd::clampInt(static_cast<int>(std::lround(IN0(0))), 1, 256);
    unit->originalSeed = sd::seedForUnit(unit, 5);
    unit->rng.setSeed(unit->originalSeed);
    unit->state = initialState(unit, 4, unit->states, 0);
    unit->trigger.initialize(IN0(1));
    unit->reset.initialize(IN0(6));
    unit->warned = false;
    SndBuf* transition = sd::resolveBuffer(unit, IN0(2));
    if (!transition || !transition->data || transition->channels != 1
        || static_cast<int>(transition->samples) < unit->states * unit->states)
        sd::warnOnce(unit, unit->warned,
                     "MarkovChain: transition buffer is missing or too short; holding state.");
    if (IN0(3) >= 0.0f) {
        SndBuf* values = sd::resolveBuffer(unit, IN0(3));
        if (!values || !values->data || values->channels != 1
            || static_cast<int>(values->samples) < unit->states)
            sd::warnOnce(unit, unit->warned,
                         "MarkovChain: value buffer is missing or too short; using state indices.");
    }
    SETCALC(MarkovChain_next);
    MarkovChain_next(unit, 1);
}

struct SemiMarkov : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    int states = 2;
    int state = 0;
    double remaining = 1.0;
    sd::RisingEdge reset;
    bool warned = false;
};

double stateDuration(SemiMarkov* unit, int state)
{
    SndBuf* durations = sd::resolveBuffer(unit, IN0(2));
    if (!durations || !durations->data || durations->channels != 3
        || static_cast<int>(durations->frames) < unit->states)
        return 1.0;
    LOCK_SNDBUF_SHARED(durations);
    const int offset = state * 3;
    double mean = sd::finiteOr(durations->data[offset], SAMPLEDUR);
    mean = std::max(mean, SAMPLEDUR);
    const double shape = std::max(
        std::abs(sd::finiteOr(durations->data[offset + 1], 1.0)), 1.0e-6);
    const int distribution = sd::clampInt(
        static_cast<int>(std::lround(sd::finiteOr(durations->data[offset + 2], 0.0))), 0, 4);
    double result = mean;
    switch (distribution) {
    case 1:
        result = sd::exponentialSample(unit->rng, mean);
        break;
    case 2:
        result = sd::gammaSample(unit->rng, shape, mean / shape);
        break;
    case 3:
        result = sd::weibullSample(unit->rng, shape, mean / std::tgamma(1.0 + 1.0 / shape));
        break;
    case 4:
        result = sd::logNormalSample(
            unit->rng, std::log(mean) - 0.5 * shape * shape, shape);
        break;
    default:
        break;
    }
    return result > 0.0 && sd::finite(result) ? result : mean;
}

void resetSemiMarkov(SemiMarkov* unit, int sample)
{
    unit->rng.setSeed(unit->originalSeed);
    unit->state = initialState(unit, 4, unit->states, sample);
    unit->remaining = stateDuration(unit, unit->state);
}

void SemiMarkov_next(SemiMarkov* unit, int inNumSamples)
{
    float* stateOutput = OUT(0);
    float* valueOutput = OUT(1);
    float* changedOutput = OUT(2);
    float* remainingOutput = OUT(3);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        bool changed = false;
        if (unit->reset.update(sd::inputAt(unit, 7, sample))) {
            resetSemiMarkov(unit, sample);
        } else {
            const double timeScale = std::max(
                sd::finiteOr(sd::inputAt(unit, 5, sample), 0.0), 0.0);
            if (timeScale > 0.0) {
                unit->remaining -= SAMPLEDUR * timeScale;
                if (unit->remaining <= 0.0) {
                    const double remainder = unit->remaining;
                    unit->state = sampleTransition(unit, unit->rng, unit->state, unit->states, 1);
                    unit->remaining = stateDuration(unit, unit->state) + remainder;
                    changed = true;
                }
            }
        }
        stateOutput[sample] = static_cast<float>(unit->state);
        valueOutput[sample] = sd::finiteFloat(stateValue(unit, unit->state, unit->states, 3));
        changedOutput[sample] = changed ? 1.0f : 0.0f;
        remainingOutput[sample] = sd::finiteFloat(std::max(unit->remaining, 0.0));
    }
}

void SemiMarkov_Ctor(SemiMarkov* unit)
{
    unit->states = sd::clampInt(static_cast<int>(std::lround(IN0(0))), 1, 256);
    unit->originalSeed = sd::seedForUnit(unit, 6);
    unit->reset.initialize(IN0(7));
    unit->warned = false;
    resetSemiMarkov(unit, 0);
    SndBuf* durations = sd::resolveBuffer(unit, IN0(2));
    if (!durations || !durations->data || durations->channels != 3
        || static_cast<int>(durations->frames) < unit->states)
        sd::warnOnce(unit, unit->warned,
                     "SemiMarkov: duration buffer is malformed; using fixed one-second durations.");
    SndBuf* transitions = sd::resolveBuffer(unit, IN0(1));
    if (!transitions || !transitions->data || transitions->channels != 1
        || static_cast<int>(transitions->samples) < unit->states * unit->states)
        sd::warnOnce(unit, unit->warned,
                     "SemiMarkov: transition buffer is malformed; holding the current state.");
    if (IN0(3) >= 0.0f) {
        SndBuf* values = sd::resolveBuffer(unit, IN0(3));
        if (!values || !values->data || values->channels != 1
            || static_cast<int>(values->samples) < unit->states)
            sd::warnOnce(unit, unit->warned,
                         "SemiMarkov: value buffer is malformed; using state indices.");
    }
    SETCALC(SemiMarkov_next);
    OUT0(0) = static_cast<float>(unit->state);
    OUT0(1) = sd::finiteFloat(stateValue(unit, unit->state, unit->states, 3));
    OUT0(2) = 0.0f;
    OUT0(3) = sd::finiteFloat(std::max(unit->remaining, 0.0));
}

struct MultiGaussNoise : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    int channels = 2;
    double* covariance = nullptr;
    double* factor = nullptr;
    double* means = nullptr;
    double* normal = nullptr;
    double* values = nullptr;
    sd::RisingEdge trigger;
    sd::RisingEdge reload;
    sd::RisingEdge reset;
    bool warned = false;
};

bool loadMultiGauss(MultiGaussNoise* unit)
{
    SndBuf* covarianceBuffer = sd::resolveBuffer(unit, IN0(1));
    const int matrixSize = unit->channels * unit->channels;
    bool validBuffer = covarianceBuffer && covarianceBuffer->data
        && covarianceBuffer->channels == 1
        && static_cast<int>(covarianceBuffer->samples) >= matrixSize;
    if (validBuffer) {
        LOCK_SNDBUF_SHARED(covarianceBuffer);
        for (int row = 0; row < unit->channels; ++row) {
            for (int column = 0; column < unit->channels; ++column) {
                const double a = covarianceBuffer->data[row * unit->channels + column];
                const double b = covarianceBuffer->data[column * unit->channels + row];
                unit->covariance[row * unit->channels + column] =
                    0.5 * (sd::finiteOr(a, 0.0) + sd::finiteOr(b, 0.0));
            }
        }
    } else {
        std::fill(unit->covariance, unit->covariance + matrixSize, 0.0);
        for (int diagonal = 0; diagonal < unit->channels; ++diagonal)
            unit->covariance[diagonal * unit->channels + diagonal] = 1.0;
    }

    std::fill(unit->means, unit->means + unit->channels, 0.0);
    if (IN0(2) >= 0.0f) {
        SndBuf* meanBuffer = sd::resolveBuffer(unit, IN0(2));
        if (meanBuffer && meanBuffer->data && meanBuffer->channels == 1
            && static_cast<int>(meanBuffer->samples) >= unit->channels) {
            LOCK_SNDBUF_SHARED(meanBuffer);
            for (int channel = 0; channel < unit->channels; ++channel)
                unit->means[channel] = sd::finiteOr(meanBuffer->data[channel], 0.0);
        }
    }

    double largestDiagonal = 0.0;
    for (int diagonal = 0; diagonal < unit->channels; ++diagonal)
        largestDiagonal = std::max(
            largestDiagonal,
            std::abs(unit->covariance[diagonal * unit->channels + diagonal]));
    double jitter = 0.0;
    bool factored = sd::choleskyFactor(
        unit->covariance, unit->factor, unit->channels, jitter);
    for (int attempt = 0; attempt < 8 && !factored; ++attempt) {
        jitter = std::max(largestDiagonal, 1.0) * 1.0e-12 * std::pow(10.0, attempt);
        factored = sd::choleskyFactor(
            unit->covariance, unit->factor, unit->channels, jitter);
    }
    if (!factored) {
        std::fill(unit->factor, unit->factor + matrixSize, 0.0);
        for (int diagonal = 0; diagonal < unit->channels; ++diagonal) {
            const double variance = std::max(
                unit->covariance[diagonal * unit->channels + diagonal], 0.0);
            unit->factor[diagonal * unit->channels + diagonal] = std::sqrt(variance);
        }
    }
    return validBuffer && factored;
}

void sampleMultiGauss(MultiGaussNoise* unit)
{
    for (int channel = 0; channel < unit->channels; ++channel)
        unit->normal[channel] = sd::normalSample(unit->rng);
    for (int row = 0; row < unit->channels; ++row) {
        double value = unit->means[row];
        for (int column = 0; column <= row; ++column)
            value += unit->factor[row * unit->channels + column] * unit->normal[column];
        unit->values[row] = sd::finiteOr(value, unit->means[row]);
    }
}

void MultiGaussNoise_next(MultiGaussNoise* unit, int inNumSamples)
{
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, 5, sample));
        const bool reloadEdge = unit->reload.update(sd::inputAt(unit, 3, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 0, sample));
        if (resetEdge) {
            unit->rng.setSeed(unit->originalSeed);
            loadMultiGauss(unit);
            sampleMultiGauss(unit);
        }
        if (reloadEdge)
            loadMultiGauss(unit);
        if (triggerEdge)
            sampleMultiGauss(unit);
        for (int channel = 0; channel < unit->channels; ++channel)
            OUT(channel)[sample] = sd::finiteFloat(unit->values[channel]);
    }
}

void MultiGaussNoise_Ctor(MultiGaussNoise* unit)
{
    unit->channels = sd::clampInt(static_cast<int>(unit->mNumOutputs), 1, 32);
    const std::size_t matrixSize = static_cast<std::size_t>(unit->channels * unit->channels);
    unit->covariance = sd::allocateRT<double>(unit, matrixSize);
    unit->factor = sd::allocateRT<double>(unit, matrixSize);
    unit->means = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->channels));
    unit->normal = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->channels));
    unit->values = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->channels));
    unit->originalSeed = sd::seedForUnit(unit, 4);
    unit->rng.setSeed(unit->originalSeed);
    unit->trigger.initialize(IN0(0));
    unit->reload.initialize(IN0(3));
    unit->reset.initialize(IN0(5));
    unit->warned = false;
    if (!unit->covariance || !unit->factor || !unit->means || !unit->normal || !unit->values) {
        SETCALC(ClearUnitOutputs);
        ClearUnitOutputs(unit, 1);
        return;
    }
    if (!loadMultiGauss(unit))
        sd::warnOnce(unit, unit->warned,
                     "MultiGaussNoise: covariance was missing or not positive definite; using a safe factor.");
    if (IN0(2) >= 0.0f) {
        SndBuf* means = sd::resolveBuffer(unit, IN0(2));
        if (!means || !means->data || means->channels != 1
            || static_cast<int>(means->samples) < unit->channels)
            sd::warnOnce(unit, unit->warned,
                         "MultiGaussNoise: mean buffer is malformed; using zero means.");
    }
    sampleMultiGauss(unit);
    SETCALC(MultiGaussNoise_next);
    MultiGaussNoise_next(unit, 1);
}

void MultiGaussNoise_Dtor(MultiGaussNoise* unit)
{
    sd::freeRT(unit, unit->covariance);
    sd::freeRT(unit, unit->factor);
    sd::freeRT(unit, unit->means);
    sd::freeRT(unit, unit->normal);
    sd::freeRT(unit, unit->values);
}

} // namespace

void registerSDStateProcesses()
{
    DefineSimpleUnit(MarkovChain);
    DefineSimpleUnit(SemiMarkov);
    DefineDtorUnit(MultiGaussNoise);
}
