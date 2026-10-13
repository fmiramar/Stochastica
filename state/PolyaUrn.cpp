#include "SDAllocation.hpp"
#include "SDSnapshot.hpp"

namespace {
struct PolyaUrn : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int n, state;
    double *storage, *base, *weights, *values;
    double unitScale, probability, entropy;
    sd::RisingEdge trigger, reload, reset;
};
void restoreUrn(PolyaUrn* unit)
{
    std::copy(unit->base,unit->base+unit->n,unit->weights);
    unit->unitScale=1.0; sd::rescaleUrn(unit->weights,unit->n,unit->unitScale);
    unit->state=0;
    unit->probability=unit->weights[0]/sd::weightTotal(unit->weights,unit->n);
    unit->entropy=sd::normalizedEntropy(unit->weights,unit->n);
}
void loadUrn(PolyaUrn* unit, int sample)
{
    bool valid=sd::snapshotMono(unit,sd::inputAt(unit,1,sample),unit->base,unit->n,true);
    for (int i=0; i<unit->n && valid; ++i) valid=unit->base[i]>=0.0;
    if (!valid || !(sd::weightTotal(unit->base,unit->n)>0.0))
        std::fill(unit->base,unit->base+unit->n,1.0);
    if (!sd::snapshotMono(unit,sd::inputAt(unit,4,sample),unit->values,unit->n,true))
        for (int i=0; i<unit->n; ++i) unit->values[i]=i;
    restoreUrn(unit);
}
void writeUrn(PolyaUrn* unit, int i)
{
    OUT(0)[i]=static_cast<float>(unit->state); OUT(1)[i]=static_cast<float>(unit->values[unit->state]);
    OUT(2)[i]=static_cast<float>(unit->probability); OUT(3)[i]=static_cast<float>(unit->entropy);
}
void PolyaUrn_next(PolyaUrn* unit, int count)
{
    for (int i=0; i<count; ++i) {
        const bool reset=unit->reset.update(sd::inputAt(unit,7,i));
        const bool reload=unit->reload.update(sd::inputAt(unit,5,i));
        const bool trigger=unit->trigger.update(sd::inputAt(unit,2,i));
        if (reset) { unit->rng.setSeed(unit->seed); restoreUrn(unit); }
        else if (reload) loadUrn(unit,i);
        else if (trigger) {
            const auto result=sd::urnDraw(unit->rng,unit->weights,unit->n,sd::inputAt(unit,3,i),unit->unitScale);
            unit->state=result.state; unit->probability=result.probability; unit->entropy=result.entropy;
        }
        writeUrn(unit,i);
    }
}
void PolyaUrn_Ctor(PolyaUrn* unit)
{
    unit->storage=unit->base=unit->weights=unit->values=nullptr;
    unit->n=static_cast<int>(sd::clamp(IN0(0),2,sd::kMaxUrnStates)); unit->state=0;
    unit->unitScale=1; unit->probability=unit->entropy=0;
    unit->seed=sd::seedForUnit(unit,6); unit->rng.setSeed(unit->seed);
    unit->trigger.initialize(IN0(2)); unit->reload.initialize(IN0(5)); unit->reset.initialize(IN0(7));
    unit->storage=sd::allocateRT<double>(unit,3*unit->n);
    if (!unit->storage) { SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit,1); return; }
    unit->base=unit->storage; unit->weights=unit->base+unit->n; unit->values=unit->weights+unit->n;
    loadUrn(unit,0); SETCALC(PolyaUrn_next); writeUrn(unit,0);
}
void PolyaUrn_Dtor(PolyaUrn* unit) { sd::freeRT(unit,unit->storage); }
}
void registerSDPolya() { DefineDtorCantAliasUnit(PolyaUrn); }
