#include "SDPopulation.hpp"
#include "SDServer.hpp"

namespace {
struct WilsonCowan : Unit {
    std::array<double,2> state;
    int substeps;
    sd::RisingEdge reset;
};
void resetPopulation(WilsonCowan* unit,int sample)
{
    unit->state={sd::finiteOr(sd::inputAt(unit,14,sample),0.1),sd::finiteOr(sd::inputAt(unit,15,sample),0.1)};
}
void WilsonCowan_next(WilsonCowan* unit,int count)
{
    for (int i=0; i<count; ++i) {
        if (unit->reset.update(sd::inputAt(unit,16,i))) resetPopulation(unit,i);
        else {
            const auto in=[unit,i](int j){return sd::finiteOr(sd::inputAt(unit,j,i),0.0);};
            const sd::PopulationParameters p={in(0),in(1),in(2),in(3),in(4),in(5),
                std::max(in(6),sd::kPopulationTauFloor),std::max(in(7),sd::kPopulationTauFloor),
                in(8),in(9),in(10),in(11)};
            if (!sd::wilsonCowanStep(unit->state,p,unit->substeps,SAMPLEDUR*sd::nonnegative(in(12))))
                resetPopulation(unit,i);
        }
        OUT(0)[i]=sd::finiteFloat(unit->state[0]); OUT(1)[i]=sd::finiteFloat(unit->state[1]);
    }
}
void WilsonCowan_Ctor(WilsonCowan* unit)
{
    unit->substeps=static_cast<int>(sd::clamp(IN0(13),1,sd::kMaxPopulationSubsteps));
    unit->reset.initialize(IN0(16)); resetPopulation(unit,0); SETCALC(WilsonCowan_next);
    OUT0(0)=sd::finiteFloat(unit->state[0]); OUT0(1)=sd::finiteFloat(unit->state[1]);
}
}
void registerSDWilsonCowan() { DefineSimpleCantAliasUnit(WilsonCowan); }
