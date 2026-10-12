#include "SDEventProcesses.hpp"
#include "SDServer.hpp"

namespace {

struct SelfCorrectingTrig : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    double z;
    sd::RisingEdge reset;
};

void SelfCorrectingTrig_next(SelfCorrectingTrig* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        sd::EventResult result { false, 0.0 };
        if (unit->reset.update(sd::inputAt(unit, 4, i))) {
            unit->rng.setSeed(unit->seed);
            unit->z = 0.0;
            result.intensity = std::min(sd::nonnegative(sd::inputAt(unit, 0, i)),
                                        sd::nonnegative(sd::inputAt(unit, 2, i)));
        } else {
            result = sd::selfCorrectingStep(unit->rng, unit->z, sd::inputAt(unit, 0, i),
                sd::inputAt(unit, 1, i), sd::inputAt(unit, 2, i), SAMPLEDUR);
        }
        OUT(0)[i] = result.event ? 1.0f : 0.0f;
        OUT(1)[i] = sd::finiteFloat(result.intensity);
    }
}

void SelfCorrectingTrig_Ctor(SelfCorrectingTrig* unit)
{
    unit->seed = sd::seedForUnit(unit, 3);
    unit->rng.setSeed(unit->seed);
    unit->z = 0.0;
    unit->reset.initialize(IN0(4));
    SETCALC(SelfCorrectingTrig_next);
    OUT0(0) = 0.0f;
    OUT0(1) = sd::finiteFloat(std::min(sd::nonnegative(IN0(0)), sd::nonnegative(IN0(2))));
}

struct RandomTelegraph : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    sd::TelegraphState state;
    sd::RisingEdge reset;
};

void RandomTelegraph_next(RandomTelegraph* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        int events = 0;
        if (unit->reset.update(sd::inputAt(unit, 6, i))) {
            unit->rng.setSeed(unit->seed);
            unit->state.reset(unit->rng, sd::inputAt(unit, 4, i) > 0.0f);
        } else {
            events = unit->state.step(unit->rng, sd::inputAt(unit, 2, i),
                                      sd::inputAt(unit, 3, i), SAMPLEDUR);
        }
        OUT(0)[i] = sd::finiteFloat(sd::finiteOr(sd::inputAt(unit, unit->state.high ? 1 : 0, i), 0.0));
        OUT(1)[i] = events > 0 ? 1.0f : 0.0f;
    }
}

void RandomTelegraph_Ctor(RandomTelegraph* unit)
{
    unit->seed = sd::seedForUnit(unit, 5);
    unit->rng.setSeed(unit->seed);
    unit->state.reset(unit->rng, IN0(4) > 0.0f);
    unit->reset.initialize(IN0(6));
    SETCALC(RandomTelegraph_next);
    OUT0(0) = sd::finiteFloat(sd::finiteOr(IN0(unit->state.high ? 1 : 0), 0.0));
    OUT0(1) = 0.0f;
}

} // namespace

void registerSDAdvancedEvents()
{
    DefineSimpleCantAliasUnit(SelfCorrectingTrig);
    DefineSimpleCantAliasUnit(RandomTelegraph);
}
