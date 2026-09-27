#include "SC_PlugIn.h"
#include "SDMap.hpp"
#include "SDRange.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <cmath>

extern InterfaceTable* ft;

namespace {

struct MapSonifier : Unit {
    double x = 0.0;
    double y = 0.0;
    double phase = 0.0;
    double previousLevel = 0.0;
    double currentLevel = 0.0;
    int clockIndex = 0;
    int outputIndex = 1;
    int interpolation = 2;
    sd::RisingEdge reset;
};

struct HenonSonify : MapSonifier {};
struct GbmanSonify : MapSonifier {};
struct LatoocarfianSonify : MapSonifier {};

double coordinate(const MapSonifier* unit, int index)
{
    return index == 0 ? unit->x : unit->y;
}

double mappedCoordinate(double value, double sourceLow, double sourceHigh,
                        double destinationLow, double destinationHigh)
{
    if (!(sourceHigh > sourceLow))
        return destinationLow;
    const double folded = sd::foldValue(value, sourceLow, sourceHigh);
    const double normalized = (folded - sourceLow) / (sourceHigh - sourceLow);
    return destinationLow + normalized * (destinationHigh - destinationLow);
}

double mappedLevel(MapSonifier* unit, int sample)
{
    return mappedCoordinate(
        coordinate(unit, unit->outputIndex),
        sd::inputAt(unit, 4, sample), sd::inputAt(unit, 5, sample), -1.0, 1.0);
}

double mappedFrequency(MapSonifier* unit, int sample)
{
    const double frequency = mappedCoordinate(
        coordinate(unit, unit->clockIndex),
        sd::inputAt(unit, 2, sample), sd::inputAt(unit, 3, sample),
        sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample));
    return sd::clipValue(sd::finiteOr(frequency, 0.0), 0.0, SAMPLERATE);
}

double interpolatedLevel(const MapSonifier* unit)
{
    if (unit->interpolation == 0)
        return unit->currentLevel;
    const double phase = sd::clipValue(unit->phase, 0.0, 1.0);
    const double fraction = unit->interpolation == 1
        ? phase
        : phase * phase * (3.0 - 2.0 * phase);
    return unit->previousLevel
        + fraction * (unit->currentLevel - unit->previousLevel);
}

template <typename UnitType, typename Reset, typename Step>
void processSonifier(UnitType* unit, int inNumSamples, int resetIndex,
                     Reset resetState, Step stepState)
{
    float* output = OUT(0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        if (unit->reset.update(sd::inputAt(unit, resetIndex, sample))) {
            resetState(sample);
            unit->phase = 0.0;
            unit->previousLevel = unit->currentLevel = mappedLevel(unit, sample);
            output[sample] = sd::finiteFloat(unit->currentLevel);
            continue;
        }

        const double frequency = mappedFrequency(unit, sample);
        unit->phase += frequency / SAMPLERATE;
        if (unit->phase >= 1.0) {
            unit->phase -= 1.0;
            unit->previousLevel = unit->currentLevel;
            const sd::State2 next = stepState(sample);
            if (sd::finite(next.x) && sd::finite(next.y)) {
                unit->x = next.x;
                unit->y = next.y;
            } else {
                resetState(sample);
                unit->phase = 0.0;
            }
            unit->currentLevel = mappedLevel(unit, sample);
        }
        output[sample] = sd::finiteFloat(interpolatedLevel(unit));
    }
}

void initializeSonifier(MapSonifier* unit, int xIndex, int yIndex, int resetIndex)
{
    unit->x = sd::finiteOr(IN0(xIndex), 0.0);
    unit->y = sd::finiteOr(IN0(yIndex), 0.0);
    unit->phase = 0.0;
    unit->clockIndex = sd::clampInt(static_cast<int>(std::lround(IN0(6))), 0, 1);
    unit->outputIndex = sd::clampInt(static_cast<int>(std::lround(IN0(7))), 0, 1);
    unit->interpolation = sd::clampInt(static_cast<int>(std::lround(IN0(8))), 0, 2);
    unit->reset.initialize(IN0(resetIndex));
    unit->previousLevel = unit->currentLevel = mappedLevel(unit, 0);
}

void HenonSonify_next(HenonSonify* unit, int inNumSamples)
{
    auto reset = [unit](int sample) {
        unit->x = sd::finiteOr(sd::inputAt(unit, 11, sample), 0.0);
        unit->y = sd::finiteOr(sd::inputAt(unit, 12, sample), 0.0);
    };
    auto step = [unit](int sample) {
        return sd::henonStep(
            unit->x, unit->y, sd::inputAt(unit, 9, sample), sd::inputAt(unit, 10, sample));
    };
    processSonifier(unit, inNumSamples, 13, reset, step);
}

void HenonSonify_Ctor(HenonSonify* unit)
{
    initializeSonifier(unit, 11, 12, 13);
    SETCALC(HenonSonify_next);
    OUT0(0) = sd::finiteFloat(unit->currentLevel);
}

void GbmanSonify_next(GbmanSonify* unit, int inNumSamples)
{
    auto reset = [unit](int sample) {
        unit->x = sd::finiteOr(sd::inputAt(unit, 9, sample), 1.2);
        unit->y = sd::finiteOr(sd::inputAt(unit, 10, sample), 2.1);
    };
    auto step = [unit](int) { return sd::gbmanStep(unit->x, unit->y); };
    processSonifier(unit, inNumSamples, 11, reset, step);
}

void GbmanSonify_Ctor(GbmanSonify* unit)
{
    initializeSonifier(unit, 9, 10, 11);
    SETCALC(GbmanSonify_next);
    OUT0(0) = sd::finiteFloat(unit->currentLevel);
}

void LatoocarfianSonify_next(LatoocarfianSonify* unit, int inNumSamples)
{
    auto reset = [unit](int sample) {
        unit->x = sd::finiteOr(sd::inputAt(unit, 13, sample), 0.1);
        unit->y = sd::finiteOr(sd::inputAt(unit, 14, sample), 0.1);
    };
    auto step = [unit](int sample) {
        return sd::latoocarfianStep(
            unit->x, unit->y, sd::inputAt(unit, 9, sample), sd::inputAt(unit, 10, sample),
            sd::inputAt(unit, 11, sample), sd::inputAt(unit, 12, sample));
    };
    processSonifier(unit, inNumSamples, 15, reset, step);
}

void LatoocarfianSonify_Ctor(LatoocarfianSonify* unit)
{
    initializeSonifier(unit, 13, 14, 15);
    SETCALC(LatoocarfianSonify_next);
    OUT0(0) = sd::finiteFloat(unit->currentLevel);
}

} // namespace

void registerSDMapSonifiers()
{
    DefineSimpleUnit(HenonSonify);
    DefineSimpleUnit(GbmanSonify);
    DefineSimpleUnit(LatoocarfianSonify);
}
