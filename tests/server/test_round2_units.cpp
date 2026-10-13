#include "SC_PlugIn.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

InterfaceTable* ft;
void registerSDAdvancedEvents(); void registerSDShotNoise();
void registerSDCircularDistributions(); void registerSDMultiOU();
void registerSDMultiHawkes(); void registerSDDoubleWell(); void registerSDKuramoto();

struct Definition { size_t bytes; UnitCtorFunc ctor; UnitDtorFunc dtor; uint32 flags; };
std::map<std::string,Definition> definitions;
size_t allocations=0, alive=0; bool failAllocation=false;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
bool define(const char* name,size_t bytes,UnitCtorFunc ctor,UnitDtorFunc dtor,uint32 flags)
{definitions.emplace(name,Definition{bytes,ctor,dtor,flags});return true;}
void* allocate(World*,size_t size){++allocations;if(failAllocation)return nullptr;++alive;return std::malloc(size);}
void release(World*,void* data){if(data){--alive;std::free(data);}}
void clear(Unit* unit,int count){for(uint32 j=0;j<unit->mNumOutputs;++j)std::fill(unit->mOutBuf[j],unit->mOutBuf[j]+count,0.0f);}

struct Spec { const char* name; std::vector<float> inputs; int outputs,seed,reset,reload,trig; };
const std::vector<Spec> specs={
    {"SelfCorrectingTrig",{6,1.5,20000,123,0},2,3,4,-1,-1},
    {"RandomTelegraph",{-1,1,3,1,0,123,0},2,5,6,-1,-1},
    {"ShotNoise",{20,.05f,.4f,.2f,123,0},2,4,5,-1,-1},
    {"TVonMisesRand",{3.13f,2,0,123,0},1,3,4,-1,2},
    {"MultiOUProcess",{0,1,2,0,123,0},2,4,5,3,-1},
    {"MultiHawkesTrig",{3,4,5,20000,0,123,0},4,5,6,4,-1},
    {"DoubleWellProcess",{0,1,1,.8f,4,-1,4,123,0},1,7,8,-1,-1},
    {"KuramotoNetwork",{6,7,3,.2f,1,4,0,123,0},4,7,8,6,-1}
};

struct Instance {
    Definition def;
    Spec spec;
    World world{}; Graph graph{}; RGen parent{}; Rate rate{};
    std::array<SndBuf,8> buffers{};
    std::array<std::vector<float>,8> data;
    std::vector<Wire> wires;
    std::vector<Wire*> wirePointers;
    std::vector<std::array<float,64>> ins,outs;
    std::vector<float*> inPointers,outPointers;
    Unit* unit;
    Instance(const Spec& s,bool audio):def(definitions.at(s.name)),spec(s),
        data{{{1,.7f,.7f,1},{.1f,-.2f},{1,2},{2,.2f},{0,0,12,0},{10,10,10,10},{.9f,1.1f},{-1,1}}},
        wires(s.inputs.size()),wirePointers(s.inputs.size()),ins(s.inputs.size()),outs(s.outputs),
        inPointers(s.inputs.size()),outPointers(s.outputs)
    {
        rate.mSampleRate=audio?48000:750;rate.mSampleDur=1/rate.mSampleRate;rate.mBufLength=audio?64:1;
        parent.init(199);graph.mRGen=&parent;graph.localBufNum=-1;
        for(size_t i=0;i<buffers.size();++i){buffers[i].data=data[i].data();buffers[i].channels=1;buffers[i].samples=static_cast<int>(data[i].size());buffers[i].frames=buffers[i].samples;}
        world.mNumSndBufs=8;world.mSndBufs=buffers.data();world.mVerbosity=-1;
        for(size_t i=0;i<ins.size();++i){ins[i].fill(s.inputs[i]);wires[i].mCalcRate=audio?calc_FullRate:calc_BufRate;wirePointers[i]=&wires[i];inPointers[i]=ins[i].data();}
        for(size_t i=0;i<outs.size();++i){outs[i].fill(0);outPointers[i]=outs[i].data();}
        unit=static_cast<Unit*>(std::malloc(def.bytes));std::memset(unit,0xa5,def.bytes);
        unit->mWorld=&world;unit->mParent=&graph;unit->mRate=&rate;
        unit->mNumInputs=static_cast<uint32>(ins.size());unit->mNumOutputs=static_cast<uint32>(outs.size());
        unit->mInput=wirePointers.data();unit->mInBuf=inPointers.data();unit->mOutBuf=outPointers.data();
        unit->mCalcRate=audio?calc_FullRate:calc_BufRate;unit->mBufLength=rate.mBufLength;
        def.ctor(unit);
    }
    ~Instance(){if(def.dtor)def.dtor(unit);std::free(unit);}
    void input(int i,float value){ins[i].fill(value);}
    std::vector<float> step(int count=1){
        const size_t before=allocations; unit->mCalcFunc(unit,count);
        require(allocations==before,"allocation in calculation function");
        std::vector<float> result;
        for(const auto& output:outs)for(int i=0;i<count;++i){require(std::isfinite(output[i]),"non-finite server output");result.push_back(output[i]);}
        return result;
    }
    void reset(){input(spec.reset,0);step();input(spec.reset,1);step();input(spec.reset,0);}
    std::vector<float> trace(int n=4096){
        std::vector<float> result;
        for(int i=0;i<n;++i){if(spec.trig>=0)input(spec.trig,i%7==0?1.0f:0.0f);auto frame=step();if(i==17||i==257||i==1023||i==4095)result.insert(result.end(),frame.begin(),frame.end());}
        return result;
    }
};

int main(int argc,char**argv)
{
    try {
        InterfaceTable table{};ft=&table;table.fDefineUnit=define;table.fRTAlloc=allocate;table.fRTFree=release;table.fClearUnitOutputs=clear;
        registerSDAdvancedEvents();registerSDShotNoise();registerSDCircularDistributions();registerSDMultiOU();registerSDMultiHawkes();registerSDDoubleWell();registerSDKuramoto();
        require(definitions.size()==8,"registration count");
        const bool write=argc>2 && std::string(argv[2])=="--write-fixtures";
        std::fstream fixtures(argv[1],write?std::ios::out:std::ios::in);
        require(bool(fixtures),"open stored server reference sequences");fixtures<<std::setprecision(9);
        for(const auto& spec:specs)for(bool audio:{false,true}){
            Instance a(spec,audio),b(spec,audio);
            require(a.def.flags&kUnitDef_CantAliasInputsToOutputs,"input/output alias protection");
            auto first=a.trace();auto second=b.trace();require(first==second,"explicit seed pair");
            for(float value:first){
                if(write)fixtures<<value<<'\n';
                else{double expected;require(bool(fixtures>>expected),"reference fixture length");require(std::abs(value-expected)<=2e-6*std::max(1.0,std::abs(expected)),"stored seeded reference mismatch");}
            }
            a.reset();auto resetFirst=a.trace();a.reset();require(resetFirst==a.trace(),"seeded reset replay");
            auto parent=a.parent;
            a.trace(500);require(a.parent.s1==parent.s1&&a.parent.s2==parent.s2&&a.parent.s3==parent.s3,"parent RNG used after construction");
            // A zero seed derives from the parent only once, then has local state.
            Spec derived=spec;derived.inputs[spec.seed]=0;Instance zero(derived,audio);auto zeroParent=zero.parent;
            zero.trace(500);require(zero.parent.s1==zeroParent.s1&&zero.parent.s2==zeroParent.s2&&zero.parent.s3==zeroParent.s3,"zero-seed runtime parent RNG consumption");
            if(spec.reload>=0){
                // Reset wins even if buffers change and reload rises simultaneously.
                Instance c(spec,audio),d(spec,audio);c.trace(100);d.trace(100);
                for(auto& values:c.data)std::fill(values.begin(),values.end(),0);
                c.input(spec.reset,1);d.input(spec.reset,1);c.input(spec.reload,1);
                require(c.step()==d.step(),"atomic reset priority over reload");
                c.input(spec.reset,0);d.input(spec.reset,0);c.input(spec.reload,0);
                require(c.trace(500)==d.trace(500),"suppressed reload must not be deferred");
                for(float bad:{-1.0f,9999.0f,1e30f,std::numeric_limits<float>::quiet_NaN()}){
                    a.input(0,bad);a.input(spec.reload,0);a.step();a.input(spec.reload,1);a.step(64);
                }
                // Snapshots never mutate external buffers; held reload has one edge.
                Instance e(spec,audio);auto saved=e.data;e.input(spec.reload,1);e.step();require(e.data==saved,"buffer mutation");
                e.buffers[0].channels=2;e.input(spec.reload,0);e.step();e.input(spec.reload,1);e.step();
                for(auto& buf:e.buffers){buf.samples=0;buf.data=nullptr;}
                e.input(spec.reload,0);e.step();e.input(spec.reload,1);e.step();
            }
            // All documented scalar inputs accept both rates and hostile numeric input safely.
            a.input(spec.reset,0);
            for(float extreme:{-1e30f,1e30f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){
                for(size_t i=0;i<a.ins.size();++i)if(int(i)!=spec.seed&&int(i)!=spec.reset)a.input(static_cast<int>(i),extreme);
                a.step(64);a.input(spec.reset,1);a.step();a.input(spec.reset,0);
            }
            std::cout<<spec.name<<(audio?" ar":" kr")<<" reference/reset/rates/buffers OK\n";
        }
        // Repeated allocation/free and forced RT allocation failure, including raw
        // memory filled with nonzero bytes before each server constructor.
        for(int i=0;i<200;++i)for(const auto& spec:specs){Instance unit(spec,i%2);unit.step(64);}
        require(alive==0,"RT memory leak after create/free cycles");
        failAllocation=true;
        for(const auto& spec:specs){Instance unit(spec,true);unit.step(64);}
        require(alive==0,"RT memory leak on constructor failure");
        std::cout<<"Round 2 server ABI tests passed; no calculation RT allocations or leaked RT blocks\n";
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
