#include "SDGendyHarness.hpp"
#include <limits>
#include <iostream>
InterfaceTable* ft;
void registerSDGendy();
using namespace sdtest;

int main(){try{
    InterfaceTable table{};initializeGendy(table);
    for(const auto& spec:gendySpecs){const bool audio=std::string(spec.name)!="GendyField";
        Instance a(spec,audio),b(spec,audio);require(a.definition.flags&kUnitDef_CantAliasInputsToOutputs,"alias guard");
        const auto parent=a.parent.s1;for(int j=0;j<4096;++j)require(a.step()==b.step(),"seed reproducibility");
        require(a.parent.s1==parent,"parent RNG used at calculation");
        a.input(spec.reset,1);std::vector<float> reset=a.step();a.input(spec.reset,0);
        Instance fresh(spec,audio);require(reset==fresh.step(),"atomic reset first sample");
        for(int j=0;j<4096;++j)require(a.step()==fresh.step(),"atomic reset history");
        // All edge inputs and raw-storage construction checked under sanitizers.
        for(float bad:{0.f,-1.f,std::numeric_limits<float>::max(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}) {
            Spec edge=spec;for(auto& x:edge.inputs)x=bad;edge.inputs[spec.seed]=123;
            Instance extreme(edge,audio);for(int k=0;k<256;++k)extreme.step(audio?64:1);
        }
        failAllocation=true;{Instance failed(spec,audio);for(auto x:failed.step())require(x==0,"allocation failure clear");}failAllocation=false;
        std::cout<<spec.name<<" unit checks passed\n";
    }
    auto equalToFirst=[&](Spec s){Instance baseline(gendySpecs[0],true),variant(s,true);
        for(int k=0;k<256;++k)require(baseline.step(64)==variant.step(64),"zero extension baseline");};
    auto s=gendySpecs[5];s.inputs[0]=0;equalToFirst(s);equalToFirst(gendySpecs[4]);
    s=gendySpecs[2];s.inputs[8]=s.inputs[9]=s.inputs[10]=0;equalToFirst(s);
    s=gendySpecs[2];s.inputs[6]=0;equalToFirst(s);
    // Constant percussion mobility is exactly the specified two-level engine.
    {auto second=gendySpecs[1],perc=gendySpecs[3];second.inputs[5]=2000;second.inputs[6]=.5f;
        perc.inputs[5]=perc.inputs[7]=.5f;Instance a(second,true),b(perc,true);
        for(int k=0;k<256;++k)require(a.step(64)==b.step(64),"percussion second-order mapping");}
    // Audio-rate reset inside a block must equal a fresh stream from that sample.
    {Instance a(gendySpecs[0],true),b(gendySpecs[0],true);a.step(64);a.ins[9].fill(0);a.ins[9][17]=1;
        auto x=a.step(64),y=b.step(47);for(int i=17;i<64;++i)require(x[i]==y[i-17],"in-block reset timing");}
    // Holding reset high does not keep restarting.
    {Instance a(gendySpecs[0],true),b(gendySpecs[0],true);a.input(9,1);require(a.step(64)==b.step(64),"held reset first block");require(a.step(64)==b.step(64),"held reset edge");}
    // A no-reset percussion trigger restarts mobility while preserving the running waveform.
    {s=gendySpecs[3];s.inputs[1]=0;s.inputs[4]=s.inputs[5]=.2f;s.inputs[6]=s.inputs[7]=.2f;s.inputs[9]=0;
        Instance a(s,true),b(s,true);a.step(64);b.step(64);a.input(0,1);require(a.step(64)==b.step(64),"percussion keep-state trigger");}
    // Voice-count changes preserve every existing independently seeded stream.
    {auto small=gendySpecs[6],large=small;small.outputs=1;large.outputs=32;
        Instance a(small,false),b(large,false);for(int k=0;k<20000;++k)require(a.step()[0]==b.step()[0],"field prefix invariance");}
    // Seed zero obtains its seed only in the constructor, including on reset.
    for(auto spec:gendySpecs){spec.inputs[spec.seed]=0;Instance a(spec,std::string(spec.name)!="GendyField");
        const auto initial=a.parent;for(int k=0;k<100;++k){a.input(spec.reset,k%2);a.step();}
        require(a.parent.s1==initial.s1&&a.parent.s2==initial.s2&&a.parent.s3==initial.s3,"zero seed reset consulted parent");}
    require(alive==0,"RT storage leak");
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
