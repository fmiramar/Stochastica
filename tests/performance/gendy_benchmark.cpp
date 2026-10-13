#include "../server/SDGendyHarness.hpp"
#include <chrono>
#include <iostream>
#include <memory>
InterfaceTable* ft;
using namespace sdtest;
int main(){InterfaceTable table{};initializeGendy(table);std::cout<<"class,rate,dimension,setting,instances,ns_per_sample,percent_core\n";
    for(double fs:{44100.,48000.,96000.})for(int dim:{12,32,64})for(int stress:{0,1})for(int j=0;j<6;++j){
        Spec spec=gendySpecs[j];const int pointIndex[]={8,10,11,8,7,8};spec.inputs[pointIndex[j]]=static_cast<float>(dim);
        if(stress){const int freqIndex[]={4,4,2,2,3,4};spec.inputs[freqIndex[j]]=spec.inputs[freqIndex[j]+1]=static_cast<float>(fs/dim);}
        // Four separate callback instances; construction is outside timing.
        std::array<std::unique_ptr<Instance>,4> voices;
        for(auto& v:voices){v=std::make_unique<Instance>(spec,true);v->rate.mSampleRate=fs;v->rate.mSampleDur=1/fs;
            // Reconstruct once after setting the rate so internal duration bounds use fs.
            v->definition.dtor(v->unit);v->definition.ctor(v->unit);}
        auto begin=std::chrono::steady_clock::now();const auto before=allocations;
        for(int block=0;block<2000;++block)for(auto& v:voices)v->unit->mCalcFunc(v->unit,64);
        const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
        require(before==allocations,"benchmark callback allocated");
        std::cout<<spec.name<<','<<fs<<','<<dim<<','<<stress<<",4,"<<seconds*1e9/(2000*64*4)<<','<<100*seconds/(2000*64/fs)<<'\n';
    }
    for(int n:{4,16,32}){Spec spec=gendySpecs[6];spec.outputs=n;Instance v(spec,false);auto start=std::chrono::steady_clock::now();
        for(int i=0;i<100000;++i)v.unit->mCalcFunc(v.unit,1);
        double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        std::cout<<spec.name<<",750,"<<n<<",0,1,"<<seconds*1e9/100000<<','<<100*seconds/(100000/750.)<<'\n';}
}
