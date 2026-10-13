#include "SDCollective.hpp"
#include "SDServer.hpp"

namespace {
struct DoubleWellProcess : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int substeps;
    double state;
    sd::RisingEdge reset;
};

void DoubleWellProcess_next(DoubleWellProcess* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        const double initial = sd::finiteOr(sd::inputAt(unit, 5, i), 0.0);
        if (unit->reset.update(sd::inputAt(unit, 8, i))) {
            unit->rng.setSeed(unit->seed);
            unit->state = initial;
        } else {
            unit->state = sd::doubleWellStep(unit->rng, unit->state, sd::inputAt(unit, 0, i),
                sd::inputAt(unit, 1, i), sd::inputAt(unit, 2, i), sd::inputAt(unit, 3, i),
                sd::inputAt(unit, 4, i), initial, unit->substeps, SAMPLEDUR);
        }
        OUT(0)[i] = static_cast<float>(unit->state);
    }
}

void DoubleWellProcess_Ctor(DoubleWellProcess* unit)
{
    unit->seed = sd::seedForUnit(unit, 7); unit->rng.setSeed(unit->seed);
    unit->substeps = static_cast<int>(sd::clamp(IN0(6), 1, sd::kMaxProcessSubsteps));
    unit->state = sd::finiteOr(IN0(5), 0.0);
    unit->reset.initialize(IN0(8));
    SETCALC(DoubleWellProcess_next);
    OUT0(0) = static_cast<float>(unit->state);
}
} // namespace

void registerSDDoubleWell() { DefineSimpleCantAliasUnit(DoubleWellProcess); }
