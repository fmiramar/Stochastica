#include "SDLatent.hpp"
#include "SDSnapshot.hpp"

namespace {
struct MarkovModulatedTrig : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int n, state;
    double *storage, *transitions, *eventRates, *stateRates;
    sd::RisingEdge reload, reset;
};
void loadModulated(MarkovModulatedTrig* unit, int sample)
{
    const int n=unit->n;
    const bool t=sd::snapshotMono(unit,sd::inputAt(unit,1,sample),unit->transitions,n*n);
    const bool e=sd::snapshotMono(unit,sd::inputAt(unit,2,sample),unit->eventRates,n);
    const bool s=sd::snapshotMono(unit,sd::inputAt(unit,3,sample),unit->stateRates,n);
    if (!(t && e && s)) std::fill(unit->storage,unit->storage+n*n+2*n,0.0);
    for (int i=0; i<n*n+2*n; ++i) unit->storage[i]=sd::nonnegative(unit->storage[i]);
}
void writeModulated(MarkovModulatedTrig* unit, int i, bool event, double intensity, bool changed)
{
    OUT(0)[i]=event?1.0f:0.0f; OUT(1)[i]=static_cast<float>(unit->state);
    OUT(2)[i]=static_cast<float>(intensity); OUT(3)[i]=changed?1.0f:0.0f;
}
void MarkovModulatedTrig_next(MarkovModulatedTrig* unit, int count)
{
    for (int i=0; i<count; ++i) {
        const bool reset=unit->reset.update(sd::inputAt(unit,9,i));
        const bool reload=unit->reload.update(sd::inputAt(unit,7,i));
        const double maximum=sd::nonnegative(sd::inputAt(unit,5,i));
        if (reset) {
            unit->rng.setSeed(unit->seed);
            unit->state=static_cast<int>(sd::clamp(sd::inputAt(unit,4,i),0,unit->n-1));
            writeModulated(unit,i,false,std::min(unit->eventRates[unit->state],maximum),false);
        } else {
            if (reload) loadModulated(unit,i);
            const auto event=sd::modulatedEventStep(unit->rng,unit->state,unit->transitions,
                unit->eventRates,unit->stateRates,unit->n,maximum,sd::inputAt(unit,6,i),SAMPLEDUR);
            unit->state=event.state; writeModulated(unit,i,event.event,event.intensity,event.changed);
        }
    }
}
void MarkovModulatedTrig_Ctor(MarkovModulatedTrig* unit)
{
    unit->storage=unit->transitions=unit->eventRates=unit->stateRates=nullptr;
    unit->n=static_cast<int>(sd::clamp(IN0(0),2,sd::kMaxModulatedStates));
    unit->state=static_cast<int>(sd::clamp(IN0(4),0,unit->n-1));
    unit->seed=sd::seedForUnit(unit,8); unit->rng.setSeed(unit->seed);
    unit->reload.initialize(IN0(7)); unit->reset.initialize(IN0(9));
    unit->storage=sd::allocateRT<double>(unit,unit->n*unit->n+2*unit->n);
    if (!unit->storage) { SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit,1); return; }
    unit->transitions=unit->storage; unit->eventRates=unit->transitions+unit->n*unit->n;
    unit->stateRates=unit->eventRates+unit->n;
    loadModulated(unit,0); SETCALC(MarkovModulatedTrig_next);
    writeModulated(unit,0,false,std::min(unit->eventRates[unit->state],sd::nonnegative(IN0(5))),false);
}
void MarkovModulatedTrig_Dtor(MarkovModulatedTrig* unit) { sd::freeRT(unit,unit->storage); }
}
void registerSDModulated() { DefineDtorCantAliasUnit(MarkovModulatedTrig); }
