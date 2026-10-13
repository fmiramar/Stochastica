#include "SDMatern.hpp"
#include "SDServer.hpp"

namespace {
struct MaternNoise : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    int mode;
    double state[3], variance, correlation, timeScale;
    sd::MaternCoefficients coefficients;
    bool valid;
    sd::RisingEdge reset;
};
void resetMatern(MaternNoise* unit, int sample)
{
    unit->state[0]=sd::finiteOr(sd::inputAt(unit,5,sample),0.0);
    unit->state[1]=unit->state[2]=0.0;
}
void cookMatern(MaternNoise* unit)
{
    const double variance=sd::nonnegative(IN0(1));
    const double correlation=std::max(sd::finiteOr(IN0(2),sd::kMaternCorrelationFloor),sd::kMaternCorrelationFloor);
    const double timeScale=sd::nonnegative(IN0(4));
    if (variance==unit->variance && correlation==unit->correlation && timeScale==unit->timeScale) return;
    const double oldLambda=unit->coefficients.lambda;
    unit->variance=variance; unit->correlation=correlation; unit->timeScale=timeScale;
    unit->valid=sd::maternCoefficients(unit->coefficients,unit->mode,variance,correlation,SAMPLEDUR*timeScale);
    if (oldLambda>0 && unit->valid) {
        const double ratio=oldLambda/unit->coefficients.lambda;
        unit->state[1]*=ratio; unit->state[2]*=ratio*ratio;
    }
}
void MaternNoise_next(MaternNoise* unit, int count)
{
    cookMatern(unit);
    for (int i=0; i<count; ++i) {
        if (unit->reset.update(sd::inputAt(unit,7,i))) {
            unit->rng.setSeed(unit->seed); resetMatern(unit,i);
        } else if (!unit->valid || !sd::maternStep(unit->rng,unit->state,sd::inputAt(unit,0,i),unit->coefficients))
            resetMatern(unit,i);
        OUT(0)[i]=sd::finiteFloat(unit->state[0]);
    }
}
void MaternNoise_Ctor(MaternNoise* unit)
{
    unit->seed=sd::seedForUnit(unit,6); unit->rng.setSeed(unit->seed);
    unit->mode=static_cast<int>(sd::clamp(IN0(3),0,2)); unit->coefficients={}; unit->valid=false;
    unit->variance=unit->correlation=unit->timeScale=-1;
    unit->reset.initialize(IN0(7)); resetMatern(unit,0); cookMatern(unit);
    SETCALC(MaternNoise_next); OUT0(0)=sd::finiteFloat(unit->state[0]);
}
}
void registerSDMatern() { DefineSimpleCantAliasUnit(MaternNoise); }
