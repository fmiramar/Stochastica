#include "SDServer.hpp"
#include "SDGendy.hpp"

namespace {
using namespace sd::gendy;
enum class Model { First, Second, Modulated, Percussion, Interpolation, Coupled };
struct GendyUnit : Unit {
    Engine engine;
    Parameters parameters;
    sd::RisingEdge reset;
    Model model;
    int resetIndex;
    double modPhase, gestureTime;
};
struct Gendy1977 : GendyUnit {};
struct Gendy1991 : GendyUnit {};
struct GendyS709 : GendyUnit {};
struct GendyPerc : GendyUnit {};
struct GendyInterp : GendyUnit {};
struct GendyCoupled : GendyUnit {};

Parameters parametersAt(GendyUnit* unit,int i)
{
    const auto in=[&](int k){return static_cast<double>(sd::inputAt(unit,k,i));};
    Parameters p;
    if(unit->model==Model::First || unit->model==Model::Second) {
        p.ampDist=selector(in(0),0,5);p.durDist=selector(in(1),0,5);
        p.ampShape=in(2);p.durShape=in(3);p.minFreq=in(4);p.maxFreq=in(5);
        if(unit->model==Model::First) {p.ampStep=in(6);p.durStep=in(7);}
        else {p.secondOrder=true;p.ampRange=in(6);p.durRange=in(7);p.ampStep=in(8);p.durStep=in(9);}
    } else if(unit->model==Model::Modulated) {
        p.ampDist=selector(in(0),0,5);p.durDist=selector(in(1),0,5);
        const double m=selector(in(7),0,1)==0 ? std::sin(sd::kTwoPi*unit->modPhase) : 2*unit->modPhase-1;
        const double shift=std::exp2(bounded(in(10),0,4,0)*m);
        p.minFreq=in(2)*shift;p.maxFreq=in(3)*shift;
        p.ampStep=bounded(in(4),0,1,0)*std::exp2(bounded(in(8),0,4,0)*m);
        p.durStep=bounded(in(5),0,1,0)*std::exp2(bounded(in(9),0,4,0)*m);
    } else if(unit->model==Model::Percussion) {
        p.secondOrder=true;p.minFreq=in(2);p.maxFreq=in(3);
        const double decay=bounded(in(1),0,1e9,1);
        p.ampRange=mobility(in(4),in(5),unit->gestureTime,decay);
        p.durRange=mobility(in(6),in(7),unit->gestureTime,decay);
        p.ampStep=p.durStep=.1;
    } else if(unit->model==Model::Interpolation) {
        p.interp=selector(in(0),0,3);p.ampDist=selector(in(1),0,5);p.durDist=selector(in(2),0,5);
        p.minFreq=in(3);p.maxFreq=in(4);p.ampStep=in(5);p.durStep=in(6);
    } else {
        p.ampCoupling=in(0);p.durCoupling=in(1);p.ampDist=selector(in(2),0,5);p.durDist=selector(in(3),0,5);
        p.minFreq=in(4);p.maxFreq=in(5);p.ampStep=in(6);p.durStep=in(7);
    }
    return p;
}

void Gendy_next(GendyUnit* unit,int count)
{
    auto& e=unit->engine;
    for(int i=0;i<count;++i) {
        if(unit->reset.update(sd::inputAt(unit,unit->resetIndex,i))) {
            unit->gestureTime=0;
            if(unit->model!=Model::Percussion || sd::inputAt(unit,9,i)>0) {
                unit->modPhase=0;unit->parameters=parametersAt(unit,i);e.reset(unit->parameters);
            }
        }
        const bool boundary=!e.ready || e.elapsed>=e.length;
        if(boundary) {
            if(!e.ready || e.index==e.n-1) unit->parameters=parametersAt(unit,i);
            else if(unit->model==Model::Interpolation)
                unit->parameters.interp=selector(sd::inputAt(unit,0,i),0,3);
        }
        OUT(0)[i]=static_cast<float>(e.sample(unit->parameters));
        if(unit->model==Model::Modulated) {
            unit->modPhase+=bounded(sd::inputAt(unit,6,i),0,e.fs/2,0)/e.fs;
            unit->modPhase-=std::floor(unit->modPhase);
        }
        if(unit->model==Model::Percussion) unit->gestureTime=std::min(1e9,unit->gestureTime+1/e.fs);
    }
}

void construct(GendyUnit* unit,Model model,int pointIndex,int resetIndex,int seedIndex)
{
    unit->engine=Engine{};unit->parameters=Parameters{};unit->model=model;
    unit->resetIndex=resetIndex;unit->modPhase=0;unit->gestureTime=0;
    unit->reset.initialize(0); // A positive construction trigger starts the same fresh gesture.
    auto& e=unit->engine;e.n=selector(IN0(pointIndex),2,maxPoints);e.fs=SAMPLERATE;
    e.seed=sd::seedForUnit(unit,seedIndex);e.points=sd::allocateRT<Point>(unit,e.n);
    if(!e.points || unit->mCalcRate!=calc_FullRate) {
        SETCALC(ClearUnitOutputs);ClearUnitOutputs(unit,1);return;
    }
    unit->parameters=parametersAt(unit,0);e.reset(unit->parameters);
    SETCALC(Gendy_next);OUT0(0)=0; // Constructor consumes no samples or random draws.
}
void Gendy1977_Ctor(Gendy1977* u){construct(u,Model::First,8,9,10);}
void Gendy1991_Ctor(Gendy1991* u){construct(u,Model::Second,10,11,12);}
void GendyS709_Ctor(GendyS709* u){construct(u,Model::Modulated,11,12,13);}
void GendyPerc_Ctor(GendyPerc* u){construct(u,Model::Percussion,8,0,10);}
void GendyInterp_Ctor(GendyInterp* u){construct(u,Model::Interpolation,7,8,9);}
void GendyCoupled_Ctor(GendyCoupled* u){construct(u,Model::Coupled,8,9,10);}
void Gendy1977_Dtor(Gendy1977* u){sd::freeRT(u,u->engine.points);}
void Gendy1991_Dtor(Gendy1991* u){sd::freeRT(u,u->engine.points);}
void GendyS709_Dtor(GendyS709* u){sd::freeRT(u,u->engine.points);}
void GendyPerc_Dtor(GendyPerc* u){sd::freeRT(u,u->engine.points);}
void GendyInterp_Dtor(GendyInterp* u){sd::freeRT(u,u->engine.points);}
void GendyCoupled_Dtor(GendyCoupled* u){sd::freeRT(u,u->engine.points);}

struct GendyField : Unit { Field* voices;uint32_t seed;int n;sd::RisingEdge reset; };
void resetField(GendyField* unit)
{for(int j=0;j<unit->n;++j)unit->voices[j].reset(voiceSeed(unit->seed,j));}
void GendyField_next(GendyField* unit,int count)
{
    for(int i=0;i<count;++i) {
        if(unit->reset.update(sd::inputAt(unit,2,i)))resetField(unit);
        for(int j=0;j<unit->n;++j) {
            unit->voices[j].step(sd::inputAt(unit,0,i),sd::inputAt(unit,1,i),SAMPLEDUR);
            OUT(j)[i]=unit->voices[j].gate ? 1.f : 0.f;
        }
    }
}
void GendyField_Ctor(GendyField* unit)
{
    unit->voices=nullptr;unit->n=sd::clampInt(unit->mNumOutputs,1,maxVoices);
    unit->seed=sd::seedForUnit(unit,3);unit->reset.initialize(0);
    unit->voices=sd::allocateRT<Field>(unit,unit->n);
    if(!unit->voices || unit->mNumOutputs!=static_cast<uint32_t>(unit->n) || unit->mCalcRate!=calc_BufRate) {
        SETCALC(ClearUnitOutputs);ClearUnitOutputs(unit,1);return;
    }
    resetField(unit);SETCALC(GendyField_next);ClearUnitOutputs(unit,1);
}
void GendyField_Dtor(GendyField* unit){sd::freeRT(unit,unit->voices);}
}
void registerSDGendy()
{
    DefineDtorCantAliasUnit(Gendy1977);DefineDtorCantAliasUnit(Gendy1991);
    DefineDtorCantAliasUnit(GendyS709);DefineDtorCantAliasUnit(GendyField);
    DefineDtorCantAliasUnit(GendyPerc);DefineDtorCantAliasUnit(GendyInterp);
    DefineDtorCantAliasUnit(GendyCoupled);
}
