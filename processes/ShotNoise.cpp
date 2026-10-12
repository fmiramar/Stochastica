#include "SDEventProcesses.hpp"
#include "SDServer.hpp"

namespace {
struct ShotNoise : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    sd::ShotState state;
    sd::RisingEdge reset;
};

void ShotNoise_next(ShotNoise* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        int events = 0;
        if (unit->reset.update(sd::inputAt(unit, 5, i))) {
            unit->rng.setSeed(unit->seed);
            unit->state.reset(unit->rng);
        } else {
            events = unit->state.step(unit->rng, sd::inputAt(unit, 0, i),
                sd::inputAt(unit, 1, i), sd::inputAt(unit, 2, i), sd::inputAt(unit, 3, i), SAMPLEDUR);
        }
        OUT(0)[i] = static_cast<float>(unit->state.value);
        OUT(1)[i] = events > 0 ? 1.0f : 0.0f;
    }
}

void ShotNoise_Ctor(ShotNoise* unit)
{
    unit->seed = sd::seedForUnit(unit, 4);
    unit->rng.setSeed(unit->seed);
    unit->state.reset(unit->rng);
    unit->reset.initialize(IN0(5));
    SETCALC(ShotNoise_next);
    OUT0(0) = OUT0(1) = 0.0f;
}
} // namespace

void registerSDShotNoise()
{
    DefineSimpleCantAliasUnit(ShotNoise);
}
