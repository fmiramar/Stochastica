#include "SDRound3Harness.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

InterfaceTable* ft;
int main(int argc,char** argv)
{
    using namespace sdtest;
    try {
        require(argc>1,"usage: sd_round3_unit_tests REFERENCE_FILE [--write-fixtures]");
        InterfaceTable table{};initialize(table);require(definitions.size()==8,"registration count");
        const bool write=argc>2 && std::string(argv[2])=="--write-fixtures";
        std::fstream fixtures(argv[1],write?std::ios::out:std::ios::in);require(bool(fixtures),"fixture file");
        fixtures<<std::setprecision(9);
        for(const auto& spec:specs)for(bool audio:{false,true}) {
            Instance a(spec,audio),b(spec,audio);
            require(a.definition.flags&kUnitDef_CantAliasInputsToOutputs,"input aliasing protection");
            const auto first=a.trace();require(first==b.trace(),"seeded pair / deterministic flow");
            for(float value:first) {
                if(write)fixtures<<value<<'\n';
                else {double expected;require(bool(fixtures>>expected),"fixture length");require(std::abs(value-expected)<3e-6*std::max(1.0,std::abs(expected)),"seeded reference");}
            }
            a.reset();const auto replay=a.trace();require(first==replay,"reset must replay construction trajectory");
            a.reset();require(replay==a.trace(),"reset replay");
            const auto parent=a.parent;a.trace(500);
            require(a.parent.s1==parent.s1&&a.parent.s2==parent.s2&&a.parent.s3==parent.s3,"parent RNG consumption");
            if(spec.seed>=0){auto zero=spec;zero.inputs[spec.seed]=0;Instance derived(zero,audio);const auto start=derived.parent;derived.trace(500);require(start.s1==derived.parent.s1&&start.s2==derived.parent.s2&&start.s3==derived.parent.s3,"seed zero parent isolation");}
            if(spec.reload>=0) {
                Instance c(spec,audio),d(spec,audio);c.trace(100);d.trace(100);
                for(auto& buffer:c.data)std::fill(buffer.begin(),buffer.end(),0);
                c.input(spec.reset,1);d.input(spec.reset,1);c.input(spec.reload,1);
                require(c.step()==d.step(),"reset priority over reload");
                c.input(spec.reset,0);d.input(spec.reset,0);c.input(spec.reload,0);
                require(c.trace(300)==d.trace(300),"suppressed reload not deferred");
                Instance e(spec,audio);const auto original=e.data;
                e.input(spec.reload,1);e.step();require(original==e.data,"external buffer mutation");
                for(auto& buffer:e.buffers){buffer.channels=2;buffer.samples=0;buffer.data=nullptr;}
                e.input(spec.reload,0);e.step();e.input(spec.reload,1);e.step(64);
            }
            for(float bad:{-1e30f,1e30f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
                for(size_t j=0;j<a.ins.size();++j)if(int(j)!=spec.seed&&int(j)!=spec.reset)a.input(static_cast<int>(j),bad);
                a.step(64);a.input(spec.reset,1);a.step();a.input(spec.reset,0);
                // Constructor robustness includes init-only selectors and sizes.
                auto extreme=spec;for(size_t j=0;j<extreme.inputs.size();++j)if(int(j)!=spec.seed)extreme.inputs[j]=bad;
                Instance initialized(extreme,audio);initialized.step(64);
            }
            std::cout<<spec.name<<(audio?" ar":" kr")<<" seed/reset/buffers/rates OK\n";
        }
        for(bool audio:{false,true}) {
            Instance dir(specs[0],audio);
            dir.input(1,1);const auto draw=dir.step();require(dir.step()==draw,"held Dirichlet trigger");
            dir.data[0]={9,1,2};dir.refresh(0);dir.input(2,1);
            require(dir.step()==draw,"Dirichlet reload changed current draw");
            dir.input(4,1);auto mean=dir.step();require(mean==std::vector<float>({.75f,1.0f/12,1.0f/6}),"Dirichlet reset mean");
            dir.input(4,0);dir.input(2,0);dir.input(1,0);dir.step();
            dir.data[0][0]=std::numeric_limits<float>::quiet_NaN();dir.input(2,1);dir.input(1,1);
            require(dir.step()==std::vector<float>(3,1.0f/3),"invalid alpha uniform fallback");
            Instance urn(specs[1],audio);urn.input(2,1);auto chosen=urn.step();require(urn.step()==chosen,"held urn trigger");
            urn.data[1]={0,0,5};urn.refresh(1);urn.input(5,1);
            auto restarted=urn.step();require(restarted[0]==0&&restarted[2]==0&&restarted[3]==0,"urn reload restores base without draw");
            urn.input(5,0);urn.input(2,0);urn.step();urn.input(2,1);
            require(urn.step()[0]==2,"zero urn weight selected");

            auto buffers=standardBuffers();buffers[4]={0,1e30f,0};buffers[5]={1e30f,0,0};
            auto mm=specs[2];mm.inputs[5]=mm.inputs[6]=1e30f;
            Instance mod(mm,audio,buffers);auto switched=mod.step();
            require(switched==std::vector<float>({0,1,0,1}),"new state retroactively changed event decision");
            const auto next=mod.step();require(next[0]==1&&next[1]==1&&next[2]==1e30f&&next[3]==0,"new state next-calculation intensity");
            mod.input(9,1);require(mod.step()==std::vector<float>({0,0,0,0}),"modulated reset auxiliary outputs");

            auto stationary=specs[7];stationary.inputs[1]=0;
            Instance spatial(stationary,audio);const auto old=spatial.step();
            std::fill(spatial.data[6].begin(),spatial.data[6].end(),3);
            spatial.input(4,1);require(spatial.step()==old,"Lorenz reload teleported state");
            spatial.input(4,0);spatial.step();spatial.input(5,1);
            require(spatial.step()==std::vector<float>(8,3),"Lorenz reset buffer template");
            spatial.input(5,0);spatial.input(3,-1);spatial.input(4,1);spatial.step();
            spatial.input(0,9);spatial.input(5,1);auto defaults=spatial.step();
            require(std::abs(defaults[0]-9.01f)<1e-6&&defaults[1]==9,"Lorenz current-forcing default reset");

            auto paused=specs[5];paused.inputs[5]=0;Instance jump(paused,audio);
            require(jump.step()==std::vector<float>(4,0),"paused jump has zero auxiliaries");
            jump.input(5,1);jump.input(2,1e30f);require(jump.step()==std::vector<float>(4,0),"unsupported count recovery");
        }
        // Coefficient inputs are block-sampled, while mean and reset may be audio-rate.
        Instance a(specs[3],true),b(specs[3],true);
        for(int i=1;i<64;++i){a.ins[1][i]=100;a.ins[2][i]=.0001f;a.ins[4][i]=100;}
        require(a.step(64)==b.step(64),"Matérn coefficient modulation is not block-rate");
        a.input(1,100);require(a.step(64)!=b.step(64),"Matérn block coefficients not refreshed");

        auto maximum=specs;
        maximum[0].outputs=32;maximum[1].inputs[0]=64;maximum[2].inputs[0]=64;maximum[7].outputs=64;maximum[7].inputs[2]=16;
        auto data=standardBuffers();data[0]=std::vector<float>(32,.2f);data[1]=std::vector<float>(64,1);
        data[2]=std::vector<float>(64,100);data[3]=std::vector<float>(4096,1);data[4]=std::vector<float>(64,100);data[5]=std::vector<float>(64,1e6f);data[6]=std::vector<float>(64,8);data[6][0]+=0.01f;
        for(const auto& spec:maximum){Instance max(spec,true,data);max.trace(400);max.step(64);}
        for(int repeat=0;repeat<200;++repeat)for(const auto& spec:specs){Instance x(spec,repeat%2);x.step(64);}
        require(alive==0,"RT memory leaked after lifecycle tests");
        failAllocation=true;for(const auto& spec:specs){Instance x(spec,true);x.step(64);}require(alive==0,"allocation failure cleanup");
        std::cout<<"Round 3 server ABI passed: 1600 lifecycle cycles, no calculation RT allocations or leaks\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
