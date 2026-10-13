#include "SDSnapshot.hpp"
#include "SDVectorProcesses.hpp"

namespace {
struct MultiOUProcess : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int n;
    double *storage, *covariance, *factor, *means, *theta, *decay, *normals, *values;
    sd::RisingEdge reload, reset;
};

void loadMultiOU(MultiOUProcess* unit, int sample)
{
    const int n = unit->n;
    if (!sd::snapshotMono(unit, sd::inputAt(unit, 0, sample), unit->covariance, n*n)) {
        std::fill(unit->covariance, unit->covariance+n*n, 0.0);
        for (int i = 0; i < n; ++i) unit->covariance[i*n+i] = 1.0;
    }
    std::fill(unit->means, unit->means+n, 0.0);
    std::fill(unit->theta, unit->theta+n, 1.0);
    sd::snapshotMono(unit, sd::inputAt(unit, 1, sample), unit->means, n);
    sd::snapshotMono(unit, sd::inputAt(unit, 2, sample), unit->theta, n);
    for (int i = 0; i < n; ++i) unit->theta[i] = sd::nonnegative(unit->theta[i]);
    sd::ouInnovationCovariance(unit->covariance, unit->theta, unit->decay, n, SAMPLEDUR);
    sd::factorCovariance(unit->covariance, unit->factor, n);
}

void MultiOUProcess_next(MultiOUProcess* unit, int count)
{
    for (int i = 0; i < count; ++i) {
        const bool reset = unit->reset.update(sd::inputAt(unit, 5, i));
        const bool reload = unit->reload.update(sd::inputAt(unit, 3, i));
        if (reset) {
            unit->rng.setSeed(unit->seed);
            std::copy(unit->means, unit->means+unit->n, unit->values);
        } else {
            if (reload) loadMultiOU(unit, i);
            sd::multiOUStep(unit->rng, unit->values, unit->means, unit->decay,
                           unit->factor, unit->normals, unit->n);
        }
        for (int j = 0; j < unit->n; ++j) OUT(j)[i] = static_cast<float>(unit->values[j]);
    }
}

void MultiOUProcess_Ctor(MultiOUProcess* unit)
{
    unit->storage = unit->covariance = unit->factor = unit->means = unit->theta = nullptr;
    unit->decay = unit->normals = unit->values = nullptr;
    unit->n = sd::clampInt(unit->mNumOutputs, 1, sd::kMaxMultiOUChannels);
    unit->seed = sd::seedForUnit(unit, 4);
    unit->rng.setSeed(unit->seed);
    unit->reload.initialize(IN0(3)); unit->reset.initialize(IN0(5));
    const int n = unit->n;
    unit->storage = sd::allocateRT<double>(unit, 2*n*n+5*n);
    if (!unit->storage || unit->mNumOutputs != static_cast<uint32_t>(n)) {
        SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit, 1); return;
    }
    unit->covariance = unit->storage;
    unit->factor = unit->covariance+n*n;
    unit->means = unit->factor+n*n;
    unit->theta = unit->means+n;
    unit->decay = unit->theta+n;
    unit->normals = unit->decay+n;
    unit->values = unit->normals+n;
    loadMultiOU(unit, 0);
    std::copy(unit->means, unit->means+n, unit->values);
    SETCALC(MultiOUProcess_next);
    for (int i = 0; i < n; ++i) OUT0(i) = static_cast<float>(unit->values[i]);
}
void MultiOUProcess_Dtor(MultiOUProcess* unit) { sd::freeRT(unit, unit->storage); }
} // namespace

void registerSDMultiOU() { DefineDtorCantAliasUnit(MultiOUProcess); }
