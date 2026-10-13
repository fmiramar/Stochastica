#include "SDSnapshot.hpp"
#include "SDVectorProcesses.hpp"

namespace {
struct MultiHawkesTrig : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int n;
    double *storage, *base, *alpha, *decay, *history, *intensity, *events;
    sd::RisingEdge reload, reset;
};

void loadMultiHawkes(MultiHawkesTrig* unit, int sample)
{
    const int n = unit->n;
    const bool base = sd::snapshotMono(unit, sd::inputAt(unit, 0, sample), unit->base, n);
    const bool alpha = sd::snapshotMono(unit, sd::inputAt(unit, 1, sample), unit->alpha, n*n);
    const bool beta = sd::snapshotMono(unit, sd::inputAt(unit, 2, sample), unit->decay, n*n);
    if (!(base && alpha && beta)) {
        std::fill(unit->base, unit->base+n, 0.0);
        std::fill(unit->alpha, unit->alpha+n*n, 0.0);
        std::fill(unit->decay, unit->decay+n*n, 0.0);
        std::fill(unit->history, unit->history+n*n, 0.0);
    }
    for (int i = 0; i < n; ++i) unit->base[i] = sd::nonnegative(unit->base[i]);
    for (int i = 0; i < n*n; ++i) {
        unit->alpha[i] = sd::nonnegative(unit->alpha[i]);
        unit->decay[i] = std::exp(-sd::nonnegative(unit->decay[i])*SAMPLEDUR);
    }
}

void MultiHawkesTrig_next(MultiHawkesTrig* unit, int count)
{
    const int n = unit->n;
    for (int i = 0; i < count; ++i) {
        const bool reset = unit->reset.update(sd::inputAt(unit, 6, i));
        const bool reload = unit->reload.update(sd::inputAt(unit, 4, i));
        const double maximum = sd::nonnegative(sd::inputAt(unit, 3, i));
        if (reset) {
            unit->rng.setSeed(unit->seed);
            std::fill(unit->history, unit->history+n*n, 0.0);
            std::fill(unit->events, unit->events+n, 0.0);
            for (int j = 0; j < n; ++j) unit->intensity[j] = std::min(unit->base[j], maximum);
        } else {
            if (reload) loadMultiHawkes(unit, i);
            sd::multiHawkesStep(unit->rng, unit->history, unit->base, unit->alpha,
                               unit->decay, unit->intensity, unit->events, n, maximum, SAMPLEDUR);
        }
        for (int j = 0; j < n; ++j) {
            OUT(j)[i] = static_cast<float>(unit->events[j]);
            OUT(n+j)[i] = static_cast<float>(unit->intensity[j]);
        }
    }
}

void MultiHawkesTrig_Ctor(MultiHawkesTrig* unit)
{
    unit->storage = unit->base = unit->alpha = unit->decay = nullptr;
    unit->history = unit->intensity = unit->events = nullptr;
    unit->n = sd::clampInt(unit->mNumOutputs/2, 1, sd::kMaxHawkesProcesses);
    unit->seed = sd::seedForUnit(unit, 5); unit->rng.setSeed(unit->seed);
    unit->reload.initialize(IN0(4)); unit->reset.initialize(IN0(6));
    const int n = unit->n;
    unit->storage = sd::allocateRT<double>(unit, 3*n*n+3*n);
    if (!unit->storage || unit->mNumOutputs != static_cast<uint32_t>(2*n)) {
        SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit, 1); return;
    }
    unit->base = unit->storage;
    unit->alpha = unit->base+n;
    unit->decay = unit->alpha+n*n;
    unit->history = unit->decay+n*n;
    unit->intensity = unit->history+n*n;
    unit->events = unit->intensity+n;
    loadMultiHawkes(unit, 0);
    SETCALC(MultiHawkesTrig_next);
    for (int i = 0; i < n; ++i) {
        OUT0(i) = 0.0f;
        OUT0(n+i) = sd::finiteFloat(std::min(unit->base[i], sd::nonnegative(IN0(3))));
    }
}
void MultiHawkesTrig_Dtor(MultiHawkesTrig* unit) { sd::freeRT(unit, unit->storage); }
} // namespace

void registerSDMultiHawkes() { DefineDtorCantAliasUnit(MultiHawkesTrig); }
