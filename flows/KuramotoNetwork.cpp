#include "SDCollective.hpp"
#include "SDSnapshot.hpp"

namespace {
struct KuramotoNetwork : Unit {
    sd::PCG32 rng, initialRng;
    uint32_t seed;
    int n, substeps;
    double *storage, *omega, *initial, *phases, *next;
    sd::RisingEdge reload, reset;
};

void loadKuramoto(KuramotoNetwork* unit, int sample, bool constructing)
{
    const int n = unit->n;
    std::fill(unit->omega, unit->omega+n, 0.0);
    sd::snapshotMono(unit, sd::inputAt(unit, 0, sample), unit->omega, n);
    for (int i = 0; i < n; ++i) unit->omega[i] *= sd::kTwoPi;
    if (!sd::snapshotMono(unit, sd::inputAt(unit, 1, sample), unit->initial, n)) {
        sd::PCG32 phaseRng; phaseRng.setSeed(unit->seed);
        for (int i = 0; i < n; ++i) unit->initial[i] = sd::kTwoPi*(sd::openUniformDouble(phaseRng)-0.5);
        if (constructing) unit->rng = phaseRng;
    }
    for (int i = 0; i < n; ++i) unit->initial[i] = sd::wrapAngle(unit->initial[i]);
}

void KuramotoNetwork_next(KuramotoNetwork* unit, int count)
{
    const int n = unit->n;
    for (int i = 0; i < count; ++i) {
        const bool reset = unit->reset.update(sd::inputAt(unit, 8, i));
        const bool reload = unit->reload.update(sd::inputAt(unit, 6, i));
        if (reset) {
            unit->rng = unit->initialRng;
            std::copy(unit->initial, unit->initial+n, unit->phases);
        } else {
            if (reload) loadKuramoto(unit, i, false);
            if (!sd::kuramotoStep(unit->rng, unit->phases, unit->omega, unit->next, n,
                    sd::inputAt(unit, 2, i), sd::inputAt(unit, 3, i), sd::inputAt(unit, 4, i),
                    unit->substeps, SAMPLEDUR)) {
                unit->rng = unit->initialRng;
                std::copy(unit->initial, unit->initial+n, unit->phases);
            }
        }
        const auto order = sd::kuramotoOrder(unit->phases, n);
        for (int j = 0; j < n; ++j) OUT(j)[i] = sd::angleFloat(unit->phases[j]);
        OUT(n)[i] = static_cast<float>(order.r());
        OUT(n+1)[i] = sd::angleFloat(order.psi());
    }
}

void KuramotoNetwork_Ctor(KuramotoNetwork* unit)
{
    unit->storage = unit->omega = unit->initial = unit->phases = unit->next = nullptr;
    unit->n = sd::clampInt(unit->mNumOutputs-2, 1, sd::kMaxKuramotoOscillators);
    unit->substeps = static_cast<int>(sd::clamp(IN0(5), 1, sd::kMaxProcessSubsteps));
    unit->seed = sd::seedForUnit(unit, 7); unit->rng.setSeed(unit->seed);
    unit->initialRng = unit->rng;
    unit->reload.initialize(IN0(6)); unit->reset.initialize(IN0(8));
    const int n = unit->n;
    unit->storage = sd::allocateRT<double>(unit, 4*n);
    if (!unit->storage || unit->mNumOutputs != static_cast<uint32_t>(n+2)) {
        SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit, 1); return;
    }
    unit->omega = unit->storage;
    unit->initial = unit->omega+n;
    unit->phases = unit->initial+n;
    unit->next = unit->phases+n;
    loadKuramoto(unit, 0, true);
    unit->initialRng = unit->rng;
    std::copy(unit->initial, unit->initial+n, unit->phases);
    const auto order = sd::kuramotoOrder(unit->phases, n);
    SETCALC(KuramotoNetwork_next);
    for (int i = 0; i < n; ++i) OUT0(i) = sd::angleFloat(unit->phases[i]);
    OUT0(n) = static_cast<float>(order.r()); OUT0(n+1) = sd::angleFloat(order.psi());
}
void KuramotoNetwork_Dtor(KuramotoNetwork* unit) { sd::freeRT(unit, unit->storage); }
} // namespace

void registerSDKuramoto() { DefineDtorCantAliasUnit(KuramotoNetwork); }
