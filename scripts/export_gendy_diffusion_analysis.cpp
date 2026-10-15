// Offline DDSS/DSS comparison. Same seed/increments and equal fixed spacing.
#include "SDGendyDiffusion.hpp"
#include <array>
#include <iomanip>
#include <iostream>
using namespace sd::gendy;
int main(){
    std::cout<<std::setprecision(12)<<"kind,variant,time,index,radius,amplitude\n";
    for(int variant=0;variant<3;++variant){
        std::array<DiffusionPoint,36> points{};std::array<double,36> brownian{};brownian.fill(1);
        DiffusionEngine e;e.points=points.data();e.seed=1729;e.reset();
        DiffusionParameters p;p.modFreq=0;p.dimensions=variant==0?1:2;
        sd::PCG32 baseline;baseline.setSeed(1729);
        uint64_t last=0;
        for(int i=0;i<96000;++i){const double time=i/e.fs;
            // A separate simplified DSS comparison retains the same Gaussian path
            // but has neither radial drift/origin reflection nor angular mapping.
            if(variant==2 && e.phase>=1)for(auto& r:brownian)
                r+=std::sqrt(2*p.diffusivity*p.timeStep)*diffusionNormal(baseline);
            const double position=(e.phase>=1?e.phase-1:e.phase)*37;
            double y=e.sample(p);
            if(variant==2){const int index=std::min(36,static_cast<int>(position));
                const double left=index==0?0:reflect(brownian[index-1],-1,1)/std::sqrt(1.01);
                const double right=index==36?0:reflect(brownian[index],-1,1)/std::sqrt(1.01);
                y=interpolate(1,left,right,position-index);}
            std::cout<<"audio,"<<variant<<','<<time<<",0,0,"<<y<<'\n';
            if(i==0 || e.cycle!=last){last=e.cycle;
                for(int j=0;j<36;++j)std::cout<<"point,"<<variant<<','<<time<<','<<j<<','
                    <<(variant==2?brownian[j]:points[j].radius)<<','
                    <<(variant==2?reflect(brownian[j],-1,1)/std::sqrt(1.01):points[j].amp)<<'\n';}
        }
    }
}
