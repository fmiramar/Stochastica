// Offline only: no tracing or file I/O exists in the audio callback.
#include "SDGendy.hpp"
#include <array>
#include <iomanip>
#include <iostream>
using namespace sd::gendy;
int main(){
    std::cout<<std::setprecision(12)<<"kind,variant,cycle,time,index,amp,dur,ampPrimary,durPrimary,segmentLength\n";
    for(int variant=0;variant<11;++variant){
        std::array<Point,12> points{};Engine e;e.points=points.data();e.seed=1729;Parameters p;
        p.minFreq=150;p.maxFreq=350;p.ampStep=.035;p.durStep=.015;
        if(variant==1){p.secondOrder=true;p.ampRange=.035;p.durRange=.015;p.ampStep=p.durStep=.05;}
        if(variant==3){p.secondOrder=true;p.ampStep=p.durStep=.1f;}
        if(variant>=4&&variant<=7)p.interp=variant-4;
        if(variant>=8)p.ampCoupling=(variant-8)*.125;
        e.reset(p);uint64_t last=0;
        for(int i=0;i<96000;++i){double t=i/e.fs;
            if(variant==2){const double m=std::sin(sd::kTwoPi*2*t);p.ampStep=.035*std::exp2(2*m);p.durStep=.015*std::exp2(2*m);}
            if(variant==3){p.ampRange=mobility(.5,0,t,.6);p.durRange=mobility(.5,0,t,.6);}
            const double y=e.sample(p);
            std::cout<<"audio,"<<variant<<','<<e.cycle<<','<<t<<",0,"<<y<<",0,0,0,0\n";
            if(e.cycle!=last){last=e.cycle;
                for(int j=0;j<12;++j){const auto& x=points[j];std::cout<<"point,"<<variant<<','<<e.cycle<<','<<t<<','<<j<<','<<x.amp<<','<<x.dur<<','<<x.ampPrimary<<','<<x.durPrimary<<','<<x.dur<<'\n';}}
        }
    }
    for(int j=0;j<8;++j){Field f;f.reset(voiceSeed(1729,j));for(int i=0;i<5000;++i){f.step(.3,.55,.002);std::cout<<"field,0,0,"<<i*.002<<','<<j<<','<<f.gate<<",0,0,0,0\n";}}
}
