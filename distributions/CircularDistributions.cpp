#include "SDCircular.hpp"
#include "SDServer.hpp"

namespace {
struct TVonMisesRand : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    double value;
    sd::RisingEdge trigger, reset;
};

void TVonMisesRand_next(TVonMisesRand* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        const bool reset = unit->reset.update(sd::inputAt(unit, 4, i));
        const bool trigger = unit->trigger.update(sd::inputAt(unit, 2, i));
        if (reset) unit->rng.setSeed(unit->seed);
        if (reset || trigger)
            unit->value = sd::vonMisesSample(unit->rng, sd::inputAt(unit, 0, i), sd::inputAt(unit, 1, i));
        OUT(0)[i] = sd::angleFloat(unit->value);
    }
}

void TVonMisesRand_Ctor(TVonMisesRand* unit)
{
    unit->seed = sd::seedForUnit(unit, 3);
    unit->rng.setSeed(unit->seed);
    unit->trigger.initialize(IN0(2));
    unit->reset.initialize(IN0(4));
    unit->value = sd::vonMisesSample(unit->rng, IN0(0), IN0(1));
    SETCALC(TVonMisesRand_next);
    OUT0(0) = sd::angleFloat(unit->value);
}
} // namespace

void registerSDCircularDistributions() { DefineSimpleCantAliasUnit(TVonMisesRand); }
