#include "SDGendyDiffusion.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace sd::gendy;
void check(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
void near(double a,double b,double tol=1e-11){check(std::abs(a-b)<tol,"DDSS reference mismatch");}
int main(int argc,char** argv){try{
    // Hand-computed bulk EM equation, zero limits, and explicit origin policy.
    near(diffusionRadius(2,1,3,.01,0),2.01);
    near(diffusionRadius(2,1,1,.005,2),2.2);
    near(diffusionRadius(1,0,2,.01,5),1);
    near(diffusionRadius(1,1,2,0,5),1);
    near(diffusionRadius(0,1,2,.001,0),1000,1e-9);
    near(diffusionRadius(.1,1,1,.005,-2),.1);
    near(diffusionAmplitude(1,1,0,0),1/std::sqrt(1.01));
    near(diffusionAmplitude(1.25,1,0,0),.75/std::sqrt(1.01));
    near(diffusionAmplitude(5.25,1,0,0),.75/std::sqrt(1.01));
    near(diffusionAmplitude(.5,0,1,1),.05/std::sqrt(1.01));
    // Constant radius, no modulation: sample the exact zero-ended polygon.
    std::array<DiffusionPoint,4> memory{};DiffusionEngine e;e.points=memory.data();e.n=4;
    e.fs=100;e.reset();DiffusionParameters p;p.freq=10;p.diffusivity=0;p.modFreq=0;
    std::array<double,10> expected={0,0,0,.5,1,.5,0,-.5,-1,-.5};
    for(int k=0;k<100;++k)near(e.sample(p),expected[k%10]/std::sqrt(1.01));
    // Cycle-latched controls and finite two/64-point high-pitch long runs.
    p.diffusivity=1;e.reset();e.sample(p);p.diffusivity=0;e.sample(p);
    near(e.parameters.diffusivity,1);
    while(e.cycle==0)e.sample(p);near(e.parameters.diffusivity,0);
    std::array<DiffusionPoint,64> storage{};
    for(int n:{2,64})for(double d:{1.,2.,16.}) {
        e.points=storage.data();e.n=n;e.fs=48000;e.reset();p={1e20,10,d,.01,1e20};
        for(int k=0;k<1000000;++k){const double out=e.sample(p);check(std::isfinite(out)&&std::abs(out)<=1.000000001,"DDSS long-run output");}
        for(int j=0;j<n;++j)check(std::isfinite(storage[j].radius)&&storage[j].radius>=0,"DDSS physical radius");
    }
    // Independently generated Python fixture: three dimensions, 204 internal records.
    check(argc>2,"missing DDSS trace path");std::ifstream in(argv[2]);check(in.good(),"missing DDSS trace");
    std::string line;std::getline(in,line);int oldDimension=0,oldCycle=-1,records=0;
    e.points=memory.data();e.n=4;e.seed=123;
    while(std::getline(in,line)) {
        std::replace(line.begin(),line.end(),',',' ');std::istringstream row(line);
        int d,cycle,j;double radius,q,amp;check(bool(row>>d>>cycle>>j>>radius>>q>>amp),"malformed DDSS trace");
        if(d!=oldDimension){e.reset();e.parameters=DiffusionParameters{};e.parameters.dimensions=d;oldDimension=d;oldCycle=-1;}
        if(cycle!=oldCycle){e.modPhase=.2*cycle/220;e.updateShape(cycle!=0);oldCycle=cycle;}
        near(memory[j].radius,radius);near(reflect(memory[j].radius,-1,1),q);near(memory[j].amp,amp);++records;
    }
    check(records==204,"incomplete DDSS trace");
    // Common Brownian increments: refine the EM grid away from its singularity.
    // The fine solution supplies only a convergence reference, not ground truth.
    sd::PCG32 rng;rng.setSeed(29);double coarseError=0,fineError=0;
    constexpr int steps=256, paths=2000;constexpr double dt=.1/steps;
    for(int path=0;path<paths;++path) {
        std::array<double,steps> dw{};for(auto& x:dw)x=std::sqrt(dt)*diffusionNormal(rng);
        auto integrate=[&](int stride){double r=1;
            for(int j=0;j<steps;j+=stride){double increment=0;for(int k=0;k<stride;++k)increment+=dw[j+k];
                r=diffusionRadius(r,.1,3,stride*dt,increment/std::sqrt(stride*dt));}
            return r;};
        const double reference=integrate(1);coarseError+=std::abs(integrate(16)-reference);fineError+=std::abs(integrate(4)-reference);
    }
    check(fineError<.45*coarseError,"DDSS timestep refinement");
    // Initial noncentral Bessel second moment: E[r(t)^2]=r0^2+2*D*d*t.
    // Small horizon avoids origin regularization; also compare d=1 with d=3.
    double moment=0,mean1=0,mean3=0;constexpr int ensemble=20000;
    for(int path=0;path<ensemble;++path){double r1=1,r3=1;
        for(int j=0;j<100;++j){const double z=diffusionNormal(rng);r1=diffusionRadius(r1,.1,1,.001,z);r3=diffusionRadius(r3,.1,3,.001,z);}
        moment+=r3*r3;mean1+=r1;mean3+=r3;}
    near(moment/ensemble,1.06,.01);check((mean3-mean1)/ensemble>.018,"DDSS outward drift");
    std::cout<<"DDSS: 204 oracle records; refinement error ratio="<<fineError/coarseError
             <<"; radial second moment="<<moment/ensemble<<" (expected 1.06); mean drift separation="<<(mean3-mean1)/ensemble<<'\n';
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
