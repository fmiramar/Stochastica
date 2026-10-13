#pragma once
#include "SC_PlugIn.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

extern InterfaceTable* ft;
void registerSDDirichlet(); void registerSDPolya(); void registerSDModulated(); void registerSDMatern();
void registerSDLevyFlight(); void registerSDJumpDiffusion(); void registerSDWilsonCowan(); void registerSDLorenz96();

namespace sdtest {
struct Definition { size_t bytes; UnitCtorFunc ctor; UnitDtorFunc dtor; uint32 flags; };
inline std::map<std::string,Definition> definitions;
inline size_t allocations=0,alive=0;
inline bool failAllocation=false;
inline void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
inline bool define(const char* name,size_t size,UnitCtorFunc ctor,UnitDtorFunc dtor,uint32 flags)
{ definitions.emplace(name,Definition{size,ctor,dtor,flags});return true; }
inline void* allocate(World*,size_t bytes){++allocations;if(failAllocation)return nullptr;++alive;return std::malloc(bytes);}
inline void release(World*,void* memory){if(memory){--alive;std::free(memory);}}
inline void clear(Unit* unit,int count){for(uint32 i=0;i<unit->mNumOutputs;++i)std::fill(unit->mOutBuf[i],unit->mOutBuf[i]+count,0.0f);}
inline void initialize(InterfaceTable& table)
{
    ft=&table;table.fDefineUnit=define;table.fRTAlloc=allocate;table.fRTFree=release;table.fClearUnitOutputs=clear;
    registerSDDirichlet();registerSDPolya();registerSDModulated();registerSDMatern();
    registerSDLevyFlight();registerSDJumpDiffusion();registerSDWilsonCowan();registerSDLorenz96();
}
struct Spec { const char* name;std::vector<float> inputs;int outputs,seed,reset,reload,trigger; };
inline const std::vector<Spec> specs={
    {"TDirichletRand",{0,0,0,123,0},3,3,4,2,1},
    {"PolyaUrn",{3,1,0,1,2,0,123,0},4,6,7,5,2},
    {"MarkovModulatedTrig",{3,3,4,5,0,20000,20000,0,123,0},4,8,9,7,-1},
    {"MaternNoise",{.3f,2,.02f,2,1,0,123,0},1,6,7,-1,-1},
    {"LevyFlight",{1.3f,.5f,.1f,1,0,123,0},2,5,6,-1,-1},
    {"JumpDiffusion",{.1f,.5f,5,.3f,.8f,1,0,123,0},4,7,8,-1,-1},
    {"WilsonCowan",{0,0,10,12,10,2,.02f,.01f,1,0,1,0,1,4,.1f,.1f,0},2,-1,16,-1,-1},
    {"Lorenz96",{8,1,4,6,0,0},8,-1,5,4,-1}
};
inline std::vector<std::vector<float>> standardBuffers()
{
    return {{1,2,3},{1,1,1},{110,220,330},{0,1,0,.3f,0,.7f,1,0,0},
        {1,8,25},{2,1,3},{8.01f,8,8,8,8,8,8,8}};
}
struct Instance {
    Definition definition;
    Spec spec;
    World world{};Graph graph{};RGen parent{};Rate rate{};
    std::vector<SndBuf> buffers;
    std::vector<std::vector<float>> data;
    std::vector<Wire> wires;
    std::vector<Wire*> wirePointers;
    std::vector<std::array<float,64>> ins,outs;
    std::vector<float*> inPointers,outPointers;
    Unit* unit;
    Instance(const Spec& s,bool audio,std::vector<std::vector<float>> initialData=standardBuffers()):
        definition(definitions.at(s.name)),spec(s),buffers(initialData.size()),data(std::move(initialData)),
        wires(s.inputs.size()),wirePointers(s.inputs.size()),ins(s.inputs.size()),outs(s.outputs),
        inPointers(s.inputs.size()),outPointers(s.outputs)
    {
        rate.mSampleRate=audio?48000:750;rate.mSampleDur=1/rate.mSampleRate;rate.mBufLength=audio?64:1;
        parent.init(199);graph.mRGen=&parent;graph.localBufNum=-1;
        for(size_t i=0;i<buffers.size();++i) refresh(i);
        world.mNumSndBufs=static_cast<uint32>(buffers.size());world.mSndBufs=buffers.data();world.mVerbosity=-1;
        for(size_t i=0;i<ins.size();++i){ins[i].fill(s.inputs[i]);wires[i].mCalcRate=audio?calc_FullRate:calc_BufRate;wirePointers[i]=&wires[i];inPointers[i]=ins[i].data();}
        for(size_t i=0;i<outs.size();++i){outs[i].fill(0);outPointers[i]=outs[i].data();}
        unit=static_cast<Unit*>(std::malloc(definition.bytes));require(unit!=nullptr,"harness allocation");
        std::memset(unit,0xa5,definition.bytes);
        unit->mWorld=&world;unit->mParent=&graph;unit->mRate=&rate;
        unit->mNumInputs=static_cast<uint32>(ins.size());unit->mNumOutputs=static_cast<uint32>(outs.size());
        unit->mInput=wirePointers.data();unit->mInBuf=inPointers.data();unit->mOutBuf=outPointers.data();
        unit->mCalcRate=audio?calc_FullRate:calc_BufRate;unit->mBufLength=rate.mBufLength;
        definition.ctor(unit);
    }
    Instance(const Instance&)=delete;
    Instance& operator=(const Instance&)=delete;
    ~Instance(){if(definition.dtor)definition.dtor(unit);std::free(unit);}
    void refresh(size_t i){buffers[i].data=data[i].data();buffers[i].channels=1;buffers[i].samples=static_cast<int>(data[i].size());buffers[i].frames=buffers[i].samples;}
    void input(int i,float value){ins[i].fill(value);}
    std::vector<float> step(int count=1)
    {
        const size_t before=allocations;unit->mCalcFunc(unit,count);
        require(before==allocations,"RT allocation in calculation");
        std::vector<float> result;
        for(const auto& out:outs)for(int i=0;i<count;++i){require(std::isfinite(out[i]),"non-finite server output");result.push_back(out[i]);}
        return result;
    }
    std::vector<float> trace(int samples=4096)
    {
        std::vector<float> result;
        for(int i=0;i<samples;++i){if(spec.trigger>=0)input(spec.trigger,i%11==0?1.0f:0.0f);auto x=step();if(i==17||i==257||i==1023||i==4095)result.insert(result.end(),x.begin(),x.end());}
        return result;
    }
    void reset(){input(spec.reset,0);step();input(spec.reset,1);step();input(spec.reset,0);if(spec.trigger>=0)input(spec.trigger,0);}
};
} // namespace sdtest
