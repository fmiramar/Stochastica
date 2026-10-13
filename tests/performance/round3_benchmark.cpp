// Measures actual callbacks, excluding harness construction and output checking.
#include "../server/SDRound3Harness.hpp"
#include <chrono>
#include <iostream>
InterfaceTable* ft=nullptr;
int main()
{
    InterfaceTable table{};sdtest::initialize(table);
    std::cout<<"class,rate,configuration,ns_per_calculation\n";
    for(bool maximum:{false,true})for(bool audio:{true,false})for(auto spec:sdtest::specs) {
        auto buffers=sdtest::standardBuffers();
        if(maximum) {
            if(spec.name==std::string("TDirichletRand")){spec.outputs=32;buffers[0].assign(32,.2f);}
            if(spec.name==std::string("PolyaUrn")){spec.inputs[0]=64;buffers[1].assign(64,1);buffers[2].assign(64,1);}
            if(spec.name==std::string("MarkovModulatedTrig")){spec.inputs[0]=64;buffers[3].assign(4096,1);buffers[4].assign(64,12000);buffers[5].assign(64,12000);}
            if(spec.name==std::string("WilsonCowan"))spec.inputs[13]=16;
            if(spec.name==std::string("Lorenz96")){spec.outputs=64;spec.inputs[2]=16;buffers[6].assign(64,8);buffers[6][0]+=.01f;}
            if(spec.name==std::string("JumpDiffusion"))spec.inputs[2]=100000;
        }
        sdtest::Instance unit(spec,audio,buffers);
        if(spec.trigger>=0)for(int i=0;i<64;++i)unit.ins[spec.trigger][i]=(i%2)?0:1;
        const int block=audio?64:1,blocks=audio?2000:20000;
        const auto allocations=sdtest::allocations;
        const auto start=std::chrono::steady_clock::now();
        for(int j=0;j<blocks;++j) {
            if(!audio && spec.trigger>=0)unit.ins[spec.trigger][0]=(j%2)?0:1;
            unit.unit->mCalcFunc(unit.unit,block);
        }
        const double ns=std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-start).count();
        sdtest::require(allocations==sdtest::allocations,"calculation allocation");
        for(auto& out:unit.outs)for(int j=0;j<block;++j)sdtest::require(std::isfinite(out[j]),"nonfinite benchmark");
        std::cout<<spec.name<<','<<(audio?"ar":"kr")<<','<<(maximum?"maximum":"typical")<<','<<ns/(blocks*block)<<'\n';
    }
}
