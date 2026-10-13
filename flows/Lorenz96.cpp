#include "SDPopulation.hpp"
#include "SDSnapshot.hpp"

namespace {
struct Lorenz96 : Unit {
    int n,substeps;
    double *storage,*state,*initial,*scratch;
    bool buffered;
    sd::RisingEdge reload,reset;
};
void loadLorenz(Lorenz96* unit,int sample)
{
    unit->buffered=sd::snapshotMono(unit,sd::inputAt(unit,3,sample),unit->initial,unit->n,true);
}
void restoreLorenz(Lorenz96* unit,double forcing)
{
    if (!unit->buffered) {
        std::fill(unit->initial,unit->initial+unit->n,forcing);
        unit->initial[0]+=0.01;
    }
    std::copy(unit->initial,unit->initial+unit->n,unit->state);
}
void Lorenz96_next(Lorenz96* unit,int count)
{
    const double blockForcing=sd::finiteOr(IN0(0),8.0);
    for (int i=0; i<count; ++i) {
        const bool reset=unit->reset.update(sd::inputAt(unit,5,i));
        const bool reload=unit->reload.update(sd::inputAt(unit,4,i));
        if (reset) restoreLorenz(unit,blockForcing);
        else {
            if (reload) loadLorenz(unit,i);
            if (!sd::lorenz96Step(unit->state,unit->scratch,unit->n,sd::finiteOr(sd::inputAt(unit,0,i),8.0),
                    unit->substeps,SAMPLEDUR*sd::nonnegative(sd::inputAt(unit,1,i))))
                restoreLorenz(unit,blockForcing);
        }
        for (int j=0; j<unit->n; ++j) OUT(j)[i]=sd::finiteFloat(unit->state[j]);
    }
}
void Lorenz96_Ctor(Lorenz96* unit)
{
    unit->storage=unit->state=unit->initial=unit->scratch=nullptr; unit->buffered=false;
    unit->n=sd::clampInt(unit->mNumOutputs,4,sd::kMaxLorenzCells);
    unit->substeps=static_cast<int>(sd::clamp(IN0(2),1,sd::kMaxPopulationSubsteps));
    unit->reload.initialize(IN0(4)); unit->reset.initialize(IN0(5));
    unit->storage=sd::allocateRT<double>(unit,7*unit->n);
    if (!unit->storage || unit->mNumOutputs!=static_cast<uint32_t>(unit->n)) {
        SETCALC(ClearUnitOutputs); ClearUnitOutputs(unit,1); return;
    }
    unit->state=unit->storage; unit->initial=unit->state+unit->n; unit->scratch=unit->initial+unit->n;
    loadLorenz(unit,0); restoreLorenz(unit,sd::finiteOr(IN0(0),8.0)); SETCALC(Lorenz96_next);
    for (int i=0; i<unit->n; ++i) OUT0(i)=sd::finiteFloat(unit->state[i]);
}
void Lorenz96_Dtor(Lorenz96* unit) { sd::freeRT(unit,unit->storage); }
}
void registerSDLorenz96() { DefineDtorCantAliasUnit(Lorenz96); }
