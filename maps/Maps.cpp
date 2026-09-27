#include "SC_PlugIn.h"
#include "SDMap.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <cmath>

extern InterfaceTable* ft;

namespace {

struct Map2Unit : Unit {
    double x = 0.0;
    double y = 0.0;
    double initialX = 0.0;
    double initialY = 0.0;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
};

struct Map1Unit : Unit {
    double x = 0.0;
    double initialX = 0.0;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
};

struct HenonMap : Map2Unit {};
struct GbmanMap : Map2Unit {};
struct LatoocarfianMap : Map2Unit {};
struct IkedaMap : Map2Unit {};
struct LoziMap : Map2Unit {};
struct TinkerbellMap : Map2Unit {};
struct CliffordMap : Map2Unit {};
struct DeJongMap : Map2Unit {};
struct CircleMap : Map1Unit {};
struct CoupledLogisticMap : Map2Unit {};
struct RulkovMap : Map2Unit {};

inline double initialAt(Unit* unit, int input, int sample, double fallback)
{
    return sd::finiteOr(static_cast<double>(sd::inputAt(unit, input, sample)), fallback);
}

template <typename UnitType, typename Step>
void processMap2(UnitType* unit, int inNumSamples, int resetIndex,
                 int x0Index, int y0Index, Step step)
{
    float* outputX = OUT(0);
    float* outputY = OUT(1);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, resetIndex, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 0, sample));
        if (resetEdge) {
            unit->initialX = initialAt(unit, x0Index, sample, 0.0);
            unit->initialY = initialAt(unit, y0Index, sample, 0.0);
            unit->x = unit->initialX;
            unit->y = unit->initialY;
        }
        if (triggerEdge) {
            const sd::State2 next = step(sample, unit->x, unit->y);
            if (sd::finite(next.x) && sd::finite(next.y)) {
                unit->x = next.x;
                unit->y = next.y;
            } else {
                unit->initialX = initialAt(unit, x0Index, sample, 0.0);
                unit->initialY = initialAt(unit, y0Index, sample, 0.0);
                unit->x = unit->initialX;
                unit->y = unit->initialY;
            }
        }
        outputX[sample] = sd::finiteFloat(unit->x);
        outputY[sample] = sd::finiteFloat(unit->y);
    }
}

template <typename UnitType>
void initializeMap2(UnitType* unit, int x0Index, int y0Index, int resetIndex)
{
    unit->initialX = initialAt(unit, x0Index, 0, 0.0);
    unit->initialY = initialAt(unit, y0Index, 0, 0.0);
    unit->x = unit->initialX;
    unit->y = unit->initialY;
    unit->trigger.initialize(IN0(0));
    unit->reset.initialize(IN0(resetIndex));
}

void HenonMap_next(HenonMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 5, 3, 4, [unit](int sample, double x, double y) {
        return sd::henonStep(x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample));
    });
}

void HenonMap_Ctor(HenonMap* unit)
{
    initializeMap2(unit, 3, 4, 5);
    SETCALC(HenonMap_next);
    HenonMap_next(unit, 1);
}

void GbmanMap_next(GbmanMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 3, 1, 2,
                [](int, double x, double y) { return sd::gbmanStep(x, y); });
}

void GbmanMap_Ctor(GbmanMap* unit)
{
    initializeMap2(unit, 1, 2, 3);
    SETCALC(GbmanMap_next);
    GbmanMap_next(unit, 1);
}

void LatoocarfianMap_next(LatoocarfianMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 7, 5, 6, [unit](int sample, double x, double y) {
        return sd::latoocarfianStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample), sd::inputAt(unit, 4, sample));
    });
}

void LatoocarfianMap_Ctor(LatoocarfianMap* unit)
{
    initializeMap2(unit, 5, 6, 7);
    SETCALC(LatoocarfianMap_next);
    LatoocarfianMap_next(unit, 1);
}

void IkedaMap_next(IkedaMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 4, 2, 3, [unit](int sample, double x, double y) {
        return sd::ikedaStep(x, y, sd::inputAt(unit, 1, sample));
    });
}

void IkedaMap_Ctor(IkedaMap* unit)
{
    initializeMap2(unit, 2, 3, 4);
    SETCALC(IkedaMap_next);
    IkedaMap_next(unit, 1);
}

void LoziMap_next(LoziMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 5, 3, 4, [unit](int sample, double x, double y) {
        return sd::loziStep(x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample));
    });
}

void LoziMap_Ctor(LoziMap* unit)
{
    initializeMap2(unit, 3, 4, 5);
    SETCALC(LoziMap_next);
    LoziMap_next(unit, 1);
}

void TinkerbellMap_next(TinkerbellMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 7, 5, 6, [unit](int sample, double x, double y) {
        return sd::tinkerbellStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample), sd::inputAt(unit, 4, sample));
    });
}

void TinkerbellMap_Ctor(TinkerbellMap* unit)
{
    initializeMap2(unit, 5, 6, 7);
    SETCALC(TinkerbellMap_next);
    TinkerbellMap_next(unit, 1);
}

void CliffordMap_next(CliffordMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 7, 5, 6, [unit](int sample, double x, double y) {
        return sd::cliffordStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample), sd::inputAt(unit, 4, sample));
    });
}

void CliffordMap_Ctor(CliffordMap* unit)
{
    initializeMap2(unit, 5, 6, 7);
    SETCALC(CliffordMap_next);
    CliffordMap_next(unit, 1);
}

void DeJongMap_next(DeJongMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 7, 5, 6, [unit](int sample, double x, double y) {
        return sd::deJongStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample), sd::inputAt(unit, 4, sample));
    });
}

void DeJongMap_Ctor(DeJongMap* unit)
{
    initializeMap2(unit, 5, 6, 7);
    SETCALC(DeJongMap_next);
    DeJongMap_next(unit, 1);
}

void CircleMap_next(CircleMap* unit, int inNumSamples)
{
    float* output = OUT(0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, 4, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 0, sample));
        if (resetEdge) {
            unit->initialX = initialAt(unit, 3, sample, 0.0);
            unit->x = sd::wrapValue(unit->initialX, 0.0, 1.0);
        }
        if (triggerEdge) {
            const double next = sd::circleStep(
                unit->x, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample));
            unit->x = sd::finite(next) ? next : sd::wrapValue(unit->initialX, 0.0, 1.0);
        }
        output[sample] = sd::finiteFloat(unit->x);
    }
}

void CircleMap_Ctor(CircleMap* unit)
{
    unit->initialX = initialAt(unit, 3, 0, 0.0);
    unit->x = sd::wrapValue(unit->initialX, 0.0, 1.0);
    unit->trigger.initialize(IN0(0));
    unit->reset.initialize(IN0(4));
    SETCALC(CircleMap_next);
    CircleMap_next(unit, 1);
}

void CoupledLogisticMap_next(CoupledLogisticMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 6, 4, 5, [unit](int sample, double x, double y) {
        return sd::coupledLogisticStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample));
    });
}

void CoupledLogisticMap_Ctor(CoupledLogisticMap* unit)
{
    initializeMap2(unit, 4, 5, 6);
    SETCALC(CoupledLogisticMap_next);
    CoupledLogisticMap_next(unit, 1);
}

void RulkovMap_next(RulkovMap* unit, int inNumSamples)
{
    processMap2(unit, inNumSamples, 6, 4, 5, [unit](int sample, double x, double y) {
        return sd::rulkovStep(
            x, y, sd::inputAt(unit, 1, sample), sd::inputAt(unit, 2, sample),
            sd::inputAt(unit, 3, sample));
    });
}

void RulkovMap_Ctor(RulkovMap* unit)
{
    initializeMap2(unit, 4, 5, 6);
    SETCALC(RulkovMap_next);
    RulkovMap_next(unit, 1);
}

struct CoupledMapLattice : Unit {
    int cells = 2;
    double* state = nullptr;
    double* next = nullptr;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
    bool warned = false;
};

bool loadLatticeInitial(CoupledMapLattice* unit)
{
    SndBuf* initialBuffer = sd::resolveBuffer(unit, IN0(3));
    if (!initialBuffer || !initialBuffer->data || initialBuffer->samples < unit->cells)
        return false;
    LOCK_SNDBUF_SHARED(initialBuffer);
    for (int cell = 0; cell < unit->cells; ++cell)
        unit->state[cell] = sd::finiteOr(initialBuffer->data[cell], 0.5);
    return true;
}

void fallbackLattice(CoupledMapLattice* unit)
{
    std::fill(unit->state, unit->state + unit->cells, 0.5);
}

void CoupledMapLattice_next(CoupledMapLattice* unit, int inNumSamples)
{
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, 4, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, 0, sample));
        if (resetEdge && !loadLatticeInitial(unit))
            fallbackLattice(unit);

        if (triggerEdge) {
            const double r = sd::finiteOr(sd::inputAt(unit, 1, sample), 3.8);
            const double coupling = sd::finiteOr(sd::inputAt(unit, 2, sample), 0.1);
            bool valid = true;
            for (int cell = 0; cell < unit->cells; ++cell) {
                const int left = cell == 0 ? unit->cells - 1 : cell - 1;
                const int right = cell + 1 == unit->cells ? 0 : cell + 1;
                const double f = r * unit->state[cell] * (1.0 - unit->state[cell]);
                const double fLeft = r * unit->state[left] * (1.0 - unit->state[left]);
                const double fRight = r * unit->state[right] * (1.0 - unit->state[right]);
                unit->next[cell] = (1.0 - coupling) * f
                    + 0.5 * coupling * (fLeft + fRight);
                valid = valid && sd::finite(unit->next[cell]);
            }
            if (valid)
                std::swap(unit->state, unit->next);
            else if (!loadLatticeInitial(unit))
                fallbackLattice(unit);
        }

        for (int cell = 0; cell < unit->cells; ++cell)
            OUT(cell)[sample] = sd::finiteFloat(unit->state[cell]);
    }
}

void CoupledMapLattice_Ctor(CoupledMapLattice* unit)
{
    unit->cells = sd::clampInt(static_cast<int>(unit->mNumOutputs), 2, 64);
    unit->warned = false;
    unit->state = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->cells));
    unit->next = sd::allocateRT<double>(unit, static_cast<std::size_t>(unit->cells));
    unit->trigger.initialize(IN0(0));
    unit->reset.initialize(IN0(4));
    if (!unit->state || !unit->next) {
        SETCALC(ClearUnitOutputs);
        ClearUnitOutputs(unit, 1);
        return;
    }
    if (!loadLatticeInitial(unit)) {
        fallbackLattice(unit);
        sd::warnOnce(unit, unit->warned,
                     "CoupledMapLattice: initial buffer is missing or too short; using 0.5.");
    }
    SETCALC(CoupledMapLattice_next);
    CoupledMapLattice_next(unit, 1);
}

void CoupledMapLattice_Dtor(CoupledMapLattice* unit)
{
    sd::freeRT(unit, unit->state);
    sd::freeRT(unit, unit->next);
}

} // namespace

void registerSDMaps()
{
    DefineSimpleUnit(HenonMap);
    DefineSimpleUnit(GbmanMap);
    DefineSimpleUnit(LatoocarfianMap);
    DefineSimpleUnit(IkedaMap);
    DefineSimpleUnit(LoziMap);
    DefineSimpleUnit(TinkerbellMap);
    DefineSimpleUnit(CliffordMap);
    DefineSimpleUnit(DeJongMap);
    DefineSimpleUnit(CircleMap);
    DefineSimpleUnit(CoupledLogisticMap);
    DefineDtorUnit(CoupledMapLattice);
    DefineSimpleUnit(RulkovMap);
}
