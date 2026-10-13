#include "SDGendy.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
using namespace sd::gendy;
void check(bool x,const char* s){if(!x)throw std::runtime_error(s);}
int main(){try {
    std::array<Point,12> ma{},mb{},mc{};Engine a,b,c;Parameters pa,pb,pc;
    a.points=ma.data();b.points=mb.data();c.points=mc.data();a.seed=b.seed=c.seed=2718;
    pa.ampStep=.02;pb=pa;pb.secondOrder=true;pb.ampRange=.02;pb.ampStep=.01;
    pc=pa;pc.ampCoupling=.25;a.reset(pa);b.reset(pb);c.reset(pc);
    double edgeA=0,edgeB=0,roughA=0,roughC=0;
    for(int k=0;k<100000;++k){a.updateCycle(pa);b.updateCycle(pb);c.updateCycle(pc);
        if(k>10000)for(int j=0;j<12;++j){edgeA+=std::abs(ma[j].amp)>.8;edgeB+=std::abs(mb[j].amp)>.8;
            roughA+=std::pow(ma[j].amp-ma[(j+1)%12].amp,2);roughC+=std::pow(mc[j].amp-mc[(j+1)%12].amp,2);}}
    check(edgeB>1.4*edgeA,"second-order boundary residence");check(roughC<roughA*.5,"coupling roughness");
    Field f,g;f.reset(voiceSeed(29,0));g.reset(voiceSeed(29,1));
    double on=0,cross=0,sumG=0;constexpr int n=500000;
    for(int i=0;i<n;++i){check(f.step(.1,.3,.01)<=8,"field event bound");g.step(.1,.3,.01);
        on+=f.gate;sumG+=g.gate;cross+=static_cast<int>(f.gate&&g.gate);}
    const double mean=n*.01/f.fields;
    check(std::abs(mean-.1)<.003,"field exponential mean");check(std::abs(on/n-.3)<.02,"field Bernoulli occupancy");
    check(std::abs(cross/n-(on/n)*(sumG/n))<.01,"voice independence");
    f.reset(1);g.reset(2);
    for(int i=0;i<10000;++i){f.step(0,0,.01);check(!f.gate,"probability zero");g.step(0,1,.01);check(g.gate,"probability one");}
    // Mobility reaching zero freezes both levels exactly after the last evolving cycle.
    pb.ampRange=0;pb.durRange=0;b.updateCycle(pb);const auto frozen=mb;
    for(int i=0;i<1000;++i)b.updateCycle(pb);
    for(int j=0;j<12;++j)check(mb[j].amp==frozen[j].amp&&mb[j].dur==frozen[j].dur,"zero mobility freeze");
    std::cout<<"boundary residence ratio="<<edgeB/edgeA<<" roughness ratio="<<roughC/roughA
             <<" field mean="<<mean<<" occupancy="<<on/n<<'\n';
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
