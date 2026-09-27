#include "SC_PlugIn.h"
#include "SDFlow.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <array>
#include <cmath>

extern InterfaceTable* ft;

namespace {

struct Chua : Unit {
    std::array<double, 3> state { 0.7, 0.0, 0.0 };
    sd::RisingEdge reset;
    int substeps = 2;
};

void resetChua(Chua* unit, int sample)
{
    unit->state = {
        sd::finiteOr(sd::inputAt(unit, 4, sample), 0.7),
        sd::finiteOr(sd::inputAt(unit, 5, sample), 0.0),
        sd::finiteOr(sd::inputAt(unit, 6, sample), 0.0)
    };
}

void Chua_next(Chua* unit, int inNumSamples)
{
    float* outputX = OUT(0);
    float* outputY = OUT(1);
    float* outputZ = OUT(2);
    const double elapsed = SAMPLEDUR;

    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->reset.update(sd::inputAt(unit, 9, sample))) {
            resetChua(unit, sample);
        } else {
            const double alpha = sd::finiteOr(sd::inputAt(unit, 0, sample), 15.6);
            const double beta = sd::finiteOr(sd::inputAt(unit, 1, sample), 28.0);
            const double m0 = sd::finiteOr(sd::inputAt(unit, 2, sample), -1.143);
            const double m1 = sd::finiteOr(sd::inputAt(unit, 3, sample), -0.714);
            const double timeScale = sd::finiteOr(sd::inputAt(unit, 7, sample), 0.0);
            const double step = timeScale * elapsed / static_cast<double>(unit->substeps);

            auto derivative = [alpha, beta, m0, m1](const std::array<double, 3>& state) {
                const double x = state[0];
                const double h = m1 * x
                    + 0.5 * (m0 - m1) * (std::abs(x + 1.0) - std::abs(x - 1.0));
                return std::array<double, 3> {
                    alpha * (state[1] - x - h),
                    x - state[1] + state[2],
                    -beta * state[1]
                };
            };

            for (int substep = 0; substep < unit->substeps; ++substep)
                unit->state = sd::rk4<3>(unit->state, step, derivative);

            if (!sd::finite(unit->state[0]) || !sd::finite(unit->state[1])
                || !sd::finite(unit->state[2]))
                resetChua(unit, sample);
        }

        outputX[sample] = sd::finiteFloat(unit->state[0]);
        outputY[sample] = sd::finiteFloat(unit->state[1]);
        outputZ[sample] = sd::finiteFloat(unit->state[2]);
    }
}

void Chua_Ctor(Chua* unit)
{
    unit->substeps = sd::clampInt(static_cast<int>(std::lround(IN0(8))), 1, 16);
    resetChua(unit, 0);
    unit->reset.initialize(IN0(9));
    SETCALC(Chua_next);
    OUT0(0) = sd::finiteFloat(unit->state[0]);
    OUT0(1) = sd::finiteFloat(unit->state[1]);
    OUT0(2) = sd::finiteFloat(unit->state[2]);
}

struct Izhikevich : Unit {
    double v = -65.0;
    double u = -13.0;
    sd::RisingEdge reset;
    int substeps = 2;
};

void resetIzhikevich(Izhikevich* unit, int sample)
{
    unit->v = sd::finiteOr(sd::inputAt(unit, 5, sample), -65.0);
    unit->u = sd::finiteOr(sd::inputAt(unit, 6, sample), -13.0);
}

void Izhikevich_next(Izhikevich* unit, int inNumSamples)
{
    float* outputV = OUT(0);
    float* outputU = OUT(1);
    float* outputSpike = OUT(2);
    const double elapsed = SAMPLEDUR;

    for (int sample = 0; sample < inNumSamples; ++sample) {
        bool spike = false;
        if (unit->reset.update(sd::inputAt(unit, 9, sample))) {
            resetIzhikevich(unit, sample);
        } else {
            const double input = sd::finiteOr(sd::inputAt(unit, 0, sample), 0.0);
            const double a = sd::finiteOr(sd::inputAt(unit, 1, sample), 0.02);
            const double b = sd::finiteOr(sd::inputAt(unit, 2, sample), 0.2);
            const double c = sd::finiteOr(sd::inputAt(unit, 3, sample), -65.0);
            const double d = sd::finiteOr(sd::inputAt(unit, 4, sample), 8.0);
            const double timeScale = std::max(
                sd::finiteOr(sd::inputAt(unit, 7, sample), 0.0), 0.0);
            const double step = timeScale * elapsed / static_cast<double>(unit->substeps);

            for (int substep = 0; substep < unit->substeps && !spike; ++substep) {
                const double halfStep = 0.5 * step;
                auto dv = [input](double v, double u) {
                    return 0.04 * v * v + 5.0 * v + 140.0 - u + input;
                };
                unit->v += halfStep * dv(unit->v, unit->u);
                unit->v += halfStep * dv(unit->v, unit->u);
                unit->u += step * a * (b * unit->v - unit->u);
                if (unit->v >= 30.0) {
                    unit->v = c;
                    unit->u += d;
                    spike = true;
                }
            }

            if (!sd::finite(unit->v) || !sd::finite(unit->u)) {
                resetIzhikevich(unit, sample);
                spike = false;
            }
        }

        outputV[sample] = sd::finiteFloat(unit->v);
        outputU[sample] = sd::finiteFloat(unit->u);
        outputSpike[sample] = spike ? 1.0f : 0.0f;
    }
}

void Izhikevich_Ctor(Izhikevich* unit)
{
    unit->substeps = sd::clampInt(static_cast<int>(std::lround(IN0(8))), 1, 16);
    resetIzhikevich(unit, 0);
    unit->reset.initialize(IN0(9));
    SETCALC(Izhikevich_next);
    OUT0(0) = sd::finiteFloat(unit->v);
    OUT0(1) = sd::finiteFloat(unit->u);
    OUT0(2) = 0.0f;
}

struct MackeyGlass : Unit {
    double x = 1.2;
    double* delay = nullptr;
    int delaySize = 0;
    int writeIndex = 0;
    sd::RisingEdge reset;
};

void fillMackeyDelay(MackeyGlass* unit, double value)
{
    unit->x = value;
    unit->writeIndex = 0;
    if (unit->delay)
        std::fill(unit->delay, unit->delay + unit->delaySize, value);
}

double delayAt(const MackeyGlass* unit, double delaySamples)
{
    if (!(delaySamples > 0.0))
        return unit->x;
    delaySamples = sd::clamp(delaySamples, 0.0, static_cast<double>(unit->delaySize - 4));
    double position = static_cast<double>(unit->writeIndex) - delaySamples;
    if (position < 0.0)
        position += static_cast<double>(unit->delaySize);
    if (position >= static_cast<double>(unit->delaySize))
        position -= static_cast<double>(unit->delaySize);

    const int index1 = static_cast<int>(std::floor(position));
    const double fraction = position - static_cast<double>(index1);
    const int index0 = (index1 + unit->delaySize - 1) % unit->delaySize;
    const int index2 = (index1 + 1) % unit->delaySize;
    const int index3 = (index1 + 2) % unit->delaySize;
    const double y0 = unit->delay[index0];
    const double y1 = unit->delay[index1];
    const double y2 = unit->delay[index2];
    const double y3 = unit->delay[index3];
    const double a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
    const double a1 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
    const double a2 = -0.5 * y0 + 0.5 * y2;
    return ((a0 * fraction + a1) * fraction + a2) * fraction + y1;
}

void MackeyGlass_next(MackeyGlass* unit, int inNumSamples)
{
    float* output = OUT(0);
    const double elapsed = SAMPLEDUR;
    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->reset.update(sd::inputAt(unit, 7, sample))) {
            fillMackeyDelay(unit, sd::finiteOr(sd::inputAt(unit, 4, sample), 1.2));
        } else {
            const double beta = sd::finiteOr(sd::inputAt(unit, 0, sample), 0.2);
            const double gamma = sd::finiteOr(sd::inputAt(unit, 1, sample), 0.1);
            const double tau = std::max(sd::finiteOr(sd::inputAt(unit, 2, sample), 0.0), 0.0);
            const double exponent = sd::finiteOr(sd::inputAt(unit, 3, sample), 10.0);
            const double timeScale = sd::finiteOr(sd::inputAt(unit, 5, sample), 0.0);
            if (timeScale > 0.0) {
                const double delayed = delayAt(unit, (tau / timeScale) * SAMPLERATE);
                const double power = std::pow(delayed, exponent);
                const double denominator = 1.0 + power;
                const double first = beta * delayed / denominator - gamma * unit->x;
                const double predicted = unit->x + timeScale * elapsed * first;
                const double second = beta * delayed / denominator - gamma * predicted;
                const double next = unit->x + 0.5 * timeScale * elapsed * (first + second);
                if (sd::finite(next)) {
                    unit->x = next;
                } else {
                    fillMackeyDelay(unit, sd::finiteOr(sd::inputAt(unit, 4, sample), 1.2));
                }
            }
        }
        unit->delay[unit->writeIndex] = unit->x;
        unit->writeIndex = (unit->writeIndex + 1) % unit->delaySize;
        output[sample] = sd::finiteFloat(unit->x);
    }
}

void MackeyGlass_Ctor(MackeyGlass* unit)
{
    const double maxDelaySeconds = sd::clamp(sd::finiteOr(IN0(6), 2.0), 0.001, 60.0);
    unit->delaySize = std::max(8, static_cast<int>(std::ceil(maxDelaySeconds * SAMPLERATE)) + 4);
    unit->delay = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->delaySize));
    unit->reset.initialize(IN0(7));
    if (!unit->delay) {
        SETCALC(ClearUnitOutputs);
        ClearUnitOutputs(unit, 1);
        return;
    }
    fillMackeyDelay(unit, sd::finiteOr(IN0(4), 1.2));
    SETCALC(MackeyGlass_next);
    OUT0(0) = sd::finiteFloat(unit->x);
}

void MackeyGlass_Dtor(MackeyGlass* unit)
{
    sd::freeRT(unit, unit->delay);
}

} // namespace

void registerSDFlows()
{
    DefineDtorUnit(MackeyGlass);
    DefineSimpleUnit(Chua);
    DefineSimpleUnit(Izhikevich);
}
