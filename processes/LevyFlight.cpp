#include "SDTransport.hpp"
#include "SDServer.hpp"

namespace {
struct LevyFlight : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    double state;
    sd::RisingEdge reset;
};
void LevyFlight_next(LevyFlight* unit, int count)
{
    for (int i=0; i<count; ++i) {
        double increment=0;
        if (unit->reset.update(sd::inputAt(unit,6,i))) {
            unit->rng.setSeed(unit->seed); unit->state=sd::finiteOr(sd::inputAt(unit,4,i),0.0);
        } else if (!sd::levyFlightStep(unit->rng,unit->state,increment,
                sd::inputAt(unit,0,i),sd::inputAt(unit,1,i),sd::inputAt(unit,2,i),
                SAMPLEDUR*sd::nonnegative(sd::inputAt(unit,3,i))))
            unit->state=sd::finiteOr(sd::inputAt(unit,4,i),0.0);
        OUT(0)[i]=sd::finiteFloat(unit->state); OUT(1)[i]=sd::finiteFloat(increment);
    }
}
void LevyFlight_Ctor(LevyFlight* unit)
{
    unit->seed=sd::seedForUnit(unit,5); unit->rng.setSeed(unit->seed);
    unit->state=sd::finiteOr(IN0(4),0.0); unit->reset.initialize(IN0(6));
    SETCALC(LevyFlight_next); OUT0(0)=sd::finiteFloat(unit->state); OUT0(1)=0;
}
}
void registerSDLevyFlight() { DefineSimpleCantAliasUnit(LevyFlight); }
