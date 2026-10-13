#include "SDCollective.hpp"
#include "SDVectorProcesses.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <array>
volatile double sink=0;
template<typename F> void measure(const char* name,F function){
    constexpr int blockSize=64, blocks=256;
    std::array<double,blocks> elapsed{};
    double total=0;
    for(int block=0;block<blocks;++block){
        const auto start=std::chrono::steady_clock::now();
        for(int i=0;i<blockSize;++i)sink=function();
        elapsed[block]=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();
        total+=elapsed[block];
    }
    std::sort(elapsed.begin(),elapsed.end());
    const double us=total/(blocks*blockSize);
    std::cout<<name<<" us/calculation="<<us<<" single-core % at 48k="<<us*4.8<<" at 750Hz="<<us*.075
             <<"; 64-calculation batch p99/max us="<<elapsed[253]<<'/'<<elapsed.back()<<'\n';
}
int main(){
    std::cout<<std::setprecision(5);sd::PCG32 rng;rng.setSeed(77);
    double h[256]={},base[16],alpha[256],decay[256],intensity[16],events[16];
    std::fill(base,base+16,100);std::fill(alpha,alpha+256,10);std::fill(decay,decay+256,std::exp(-10.0/48000));
    measure("Hawkes N=16",[&]{sd::multiHawkesStep(rng,h,base,alpha,decay,intensity,events,16,20000,1.0/48000);return intensity[0];});
    double q[1024]={},factor[1024],theta[32],e[32],means[32]={},values[32]={},z[32];
    std::fill(theta,theta+32,1);for(int i=0;i<32;++i)for(int j=0;j<32;++j)q[i*32+j]=i==j?1:.4;
    sd::ouInnovationCovariance(q,theta,e,32,1.0/48000);sd::factorCovariance(q,factor,32);
    measure("OU N=32",[&]{sd::multiOUStep(rng,values,means,e,factor,z,32);return values[0];});
    // Reload work is excluded from ordinary update timing. Deliberately
    // indefinite covariance exercises all eight bounded jitter attempts.
    measure("OU N=32 factor SPD",[&]{sd::factorCovariance(q,factor,32);return factor[0];});
    for(int i=0;i<32;++i)for(int j=0;j<32;++j)q[i*32+j]=i==j?1:10;
    measure("OU N=32 factor indefinite",[&]{sd::factorCovariance(q,factor,32);return factor[0];});
    double p[64],w[64],next[64];for(int i=0;i<64;++i){p[i]=i*.1;w[i]=sd::kTwoPi*(.9+i*.003);}
    for(int substeps:{1,16})measure(substeps==1?"Kuramoto N=64 steps=1":"Kuramoto N=64 steps=16",[&]{sd::kuramotoStep(rng,p,w,next,64,3,.2,1,substeps,1.0/48000);return p[0];});
    for(double rate:{20.0,1e30}){
        sd::TelegraphState tele;tele.reset(rng,false);sd::ShotState shot;shot.reset(rng);
        measure(rate==20?"Telegraph normal":"Telegraph overload",[&]{tele.step(rng,rate,rate,1.0/48000);return double(tele.high);});
        measure(rate==20?"Shot normal":"Shot overload",[&]{shot.step(rng,rate,.05,0,1,1.0/48000);return shot.value;});
    }
}
