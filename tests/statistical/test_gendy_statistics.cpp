#include "SDGendy.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace sd::gendy;
void check(bool x,const char* s){if(!x)throw std::runtime_error(s);}
int main(){try {
    // Distribution fit against analytic CDFs, independently of the inverse transforms.
    sd::PCG32 rng;rng.setSeed(602);double largestKS=0;
    for(int law=0;law<6;++law)for(double a:{.2,1.}) {
        std::vector<double> draws(20000);
        for(auto& x:draws)x=innovation(law,a,sd::openUniformDouble(rng));
        std::sort(draws.begin(),draws.end());double ks=0;
        for(size_t i=0;i<draws.size();++i) {
            const double x=draws[i],q=.499*a;double cdf=(x+1)/2;
            if(law==1)cdf=.5+std::atan(10*a*x)/(2*std::atan(10*a));
            if(law==2){const double scale=std::log((.5+q)/(.5-q));cdf=.5+(1/(1+std::exp(-scale*x))-.5)/(2*q);}
            if(law==3){const double scale=std::log(std::tan(sd::kPi*(.5+q)/2));cdf=(2*std::atan(std::exp(scale*x))/sd::kPi-(.5-q))/(2*q);}
            if(law==4)cdf=.5+std::asin(x*std::sin(sd::kPi*a/2))/(sd::kPi*a);
            if(law==5)cdf=(1-std::pow(1-.999*a,(x+1)/2))/(.999*a);
            ks=std::max({ks,std::abs(cdf-double(i)/draws.size()),std::abs(cdf-double(i+1)/draws.size())});
        }
        check(ks<.018,"innovation distribution fit");largestKS=std::max(largestKS,ks);
    }
    std::cout<<"maximum quantile CDF discrepancy="<<largestKS<<'\n';
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
