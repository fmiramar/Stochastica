#include "SDAllocation.hpp"
#include "SDSnapshot.hpp"

namespace {
struct TDirichletRand : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int n;
    double *storage, *alpha, *values;
    bool valid;
    sd::RisingEdge trigger, reload, reset;
};

void loadDirichlet(TDirichletRand* unit, int sample)
{
    unit->valid = sd::snapshotMono(unit, sd::inputAt(unit,0,sample), unit->alpha, unit->n, true);
    for (int i=0; i<unit->n && unit->valid; ++i) unit->valid = unit->alpha[i] > 0.0;
    if (!unit->valid) std::fill(unit->alpha,unit->alpha+unit->n,1.0);
}
void TDirichletRand_next(TDirichletRand* unit, int count)
{
    for (int i=0; i<count; ++i) {
        const bool reset = unit->reset.update(sd::inputAt(unit,4,i));
        const bool reload = unit->reload.update(sd::inputAt(unit,2,i));
        const bool trigger = unit->trigger.update(sd::inputAt(unit,1,i));
        if (reset) {
            unit->rng.setSeed(unit->seed);
            sd::concentrationMean(unit->alpha,unit->values,unit->n);
        } else {
            if (reload) loadDirichlet(unit,i);
            if (trigger) {
                if (unit->valid) sd::dirichletSample(unit->rng,unit->alpha,unit->values,unit->n);
                else sd::concentrationMean(unit->alpha,unit->values,unit->n);
            }
        }
        for (int j=0; j<unit->n; ++j) OUT(j)[i] = static_cast<float>(unit->values[j]);
    }
}
void TDirichletRand_Ctor(TDirichletRand* unit)
{
    unit->storage=unit->alpha=unit->values=nullptr; unit->valid=false;
    unit->n=sd::clampInt(unit->mNumOutputs,2,sd::kMaxDirichletChannels);
    unit->seed=sd::seedForUnit(unit,3); unit->rng.setSeed(unit->seed);
    unit->trigger.initialize(IN0(1)); unit->reload.initialize(IN0(2)); unit->reset.initialize(IN0(4));
    unit->storage=sd::allocateRT<double>(unit,2*unit->n);
    if (!unit->storage || unit->mNumOutputs!=static_cast<uint32_t>(unit->n)) {
        SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit,1); return;
    }
    unit->alpha=unit->storage; unit->values=unit->alpha+unit->n;
    loadDirichlet(unit,0); sd::concentrationMean(unit->alpha,unit->values,unit->n);
    SETCALC(TDirichletRand_next);
    for (int i=0; i<unit->n; ++i) OUT0(i)=static_cast<float>(unit->values[i]);
}
void TDirichletRand_Dtor(TDirichletRand* unit) { sd::freeRT(unit,unit->storage); }
}
void registerSDDirichlet() { DefineDtorCantAliasUnit(TDirichletRand); }
