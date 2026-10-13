// Offline data for documentation: same numerical helpers as the server.
#include "SDCollective.hpp"
#include "SDVectorProcesses.hpp"
#include <iostream>
#include <iomanip>
int main(){
    std::cout<<std::setprecision(10);
    sd::PCG32 rng;
    for(int mode=0;mode<2;++mode){rng.setSeed(37);double z=0;for(int i=0;i<6000;++i){auto e=sd::selfCorrectingStep(rng,z,4,mode?2:0,1000,.002);std::cout<<"correct,"<<mode<<','<<i*.002<<','<<e.event<<','<<e.intensity<<'\n';}}
    rng.setSeed(4321);sd::TelegraphState tele;tele.reset(rng,false);
    for(int i=0;i<6000;++i){tele.step(rng,1,.5,.005);std::cout<<"tele,"<<i*.005<<','<<tele.high<<'\n';}
    rng.setSeed(22);sd::ShotState shot;shot.reset(rng);
    for(int i=0;i<2500;++i){int events=shot.step(rng,3,.25,1,0,.002);std::cout<<"shot,"<<i*.002<<','<<shot.value<<','<<events<<'\n';}
    rng.setSeed(41);double q[]={1,.8,.8,1},theta[]={1,1},mean[]={0,0},decay[2],factor[4],v[2]={},normal[2];
    sd::ouInnovationCovariance(q,theta,decay,2,.005);sd::factorCovariance(q,factor,2);
    for(int i=0;i<6000;++i){sd::multiOUStep(rng,v,mean,decay,factor,normal,2);std::cout<<"ou,"<<i*.005<<','<<v[0]<<','<<v[1]<<'\n';}
    rng.setSeed(456);double x=-1;
    for(int i=0;i<30000;++i){x=sd::doubleWellStep(rng,x,0,1,1,.8,1,-1,2,.005);if(i%5==0)std::cout<<"well,"<<i*.005<<','<<x<<'\n';}
    for(double k:{0,2,8}){rng.setSeed(321);for(int i=0;i<12000;++i)std::cout<<"von,"<<k<<','<<sd::vonMisesSample(rng,0,k)<<'\n';}
    for(int mode=0;mode<2;++mode){rng.setSeed(53);double p[24],w[24],next[24];for(int i=0;i<24;++i){p[i]=-2.8+i*.24;w[i]=sd::kTwoPi*(.9+i*.008);}
        for(int i=0;i<10000;++i)sd::kuramotoStep(rng,p,w,next,24,mode?4:0,0,1,4,.002);
        for(int i=0;i<24;++i)std::cout<<"kur,"<<mode<<','<<p[i]<<'\n';}
    rng.setSeed(43);double base[]={1.2,.03,.03,.03},alpha[]={0,0,0,2,10,0,0,0,0,10,0,0,0,0,10,0},d[16],h[16]={},intensity[4],events[4];
    std::fill(d,d+16,std::exp(-12*.002));
    for(int i=0;i<10000;++i){sd::multiHawkesStep(rng,h,base,alpha,d,intensity,events,4,20000,.002);std::cout<<"hawkes,"<<i*.002;for(double e:events)std::cout<<','<<e;for(double l:intensity)std::cout<<','<<l;std::cout<<'\n';}
}
