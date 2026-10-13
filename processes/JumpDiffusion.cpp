#include "SDTransport.hpp"
#include "SDServer.hpp"

namespace {
struct JumpDiffusion : Unit {
    sd::PCG32 rng;
    uint32_t seed;
    double state;
    sd::RisingEdge reset;
};
void JumpDiffusion_next(JumpDiffusion* unit, int count)
{
    for (int i=0; i<count; ++i) {
        sd::JumpResult jump{};
        if (unit->reset.update(sd::inputAt(unit,8,i))) {
            unit->rng.setSeed(unit->seed); unit->state=sd::finiteOr(sd::inputAt(unit,6,i),0.0);
        } else if (!sd::jumpDiffusionStep(unit->rng,unit->state,jump,
                sd::inputAt(unit,0,i),sd::inputAt(unit,1,i),sd::inputAt(unit,2,i),
                sd::inputAt(unit,3,i),sd::inputAt(unit,4,i),SAMPLEDUR*sd::nonnegative(sd::inputAt(unit,5,i))))
            unit->state=sd::finiteOr(sd::inputAt(unit,6,i),0.0);
        OUT(0)[i]=sd::finiteFloat(unit->state); OUT(1)[i]=jump.count?1.0f:0.0f;
        OUT(2)[i]=sd::finiteFloat(jump.size); OUT(3)[i]=static_cast<float>(jump.count);
    }
}
void JumpDiffusion_Ctor(JumpDiffusion* unit)
{
    unit->seed=sd::seedForUnit(unit,7); unit->rng.setSeed(unit->seed);
    unit->state=sd::finiteOr(IN0(6),0.0); unit->reset.initialize(IN0(8));
    SETCALC(JumpDiffusion_next); OUT0(0)=sd::finiteFloat(unit->state);
    OUT0(1)=OUT0(2)=OUT0(3)=0;
}
}
void registerSDJumpDiffusion() { DefineSimpleCantAliasUnit(JumpDiffusion); }
