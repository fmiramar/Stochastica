// Offline figure data uses the same numerical kernels as the server.
#include "SDAllocation.hpp"
#include "SDLatent.hpp"
#include "SDMatern.hpp"
#include "SDPopulation.hpp"
#include "SDTransport.hpp"
#include <iomanip>
#include <iostream>

int main()
{
    std::cout<<std::setprecision(12);
    sd::PCG32 rng;
    for(int mode=0;mode<3;++mode) {
        rng.setSeed(1729);sd::MaternCoefficients c;
        sd::maternCoefficients(c,mode,1,.25,.002);
        double x[3]={};
        for(int j=-10000;j<120000;++j) {
            sd::maternStep(rng,x,0,c);
            if(j>=0)std::cout<<"matern,"<<mode<<','<<j*.002<<','<<x[0]<<'\n';
        }
    }
    const double alphas[4][3]={{1,1,1},{.2,.2,.2},{8,8,8},{6,1,1}};
    for(int mode=0;mode<4;++mode) {
        rng.setSeed(401);double x[3];
        for(int j=0;j<4000;++j) {
            sd::dirichletSample(rng,alphas[mode],x,3);
            std::cout<<"dirichlet,"<<mode<<','<<x[0]<<','<<x[1]<<','<<x[2]<<'\n';
        }
    }
    for(int run=0;run<3;++run) {
        rng.setSeed(17+run*31);double w[3]={1,1,1},scale=1;
        for(int j=0;j<=1000;++j) {
            const double total=sd::weightTotal(w,3);
            std::cout<<"urn,"<<run<<','<<j;
            for(double v:w)std::cout<<','<<v/total;
            std::cout<<','<<sd::normalizedEntropy(w,3)<<'\n';
            sd::urnDraw(rng,w,3,1,scale);
        }
    }
    rng.setSeed(81);
    const double transition[]={0,1,0,.35,0,.65,1,0,0},events[]={1,8,28},rates[]={.7,.8,1.2};
    int state=0;
    for(int j=0;j<20000;++j) {
        auto r=sd::modulatedEventStep(rng,state,transition,events,rates,3,20000,20000,.001);
        std::cout<<"mmpp,"<<j*.001<<','<<state<<','<<r.intensity<<','<<r.event<<'\n';
        state=r.state;
    }
    for(int mode=0;mode<2;++mode) {
        rng.setSeed(57);double x=0,dx=0;
        for(int j=0;j<50000;++j) {
            sd::levyFlightStep(rng,x,dx,mode?1.2:2,1,0,.001);
            std::cout<<"levy,"<<mode<<','<<j*.001<<','<<x<<','<<dx<<'\n';
        }
    }
    rng.setSeed(72);double x=0;sd::JumpResult jump{};
    for(int j=0;j<12000;++j) {
        sd::jumpDiffusionStep(rng,x,jump,.03,.25,.8,0,1.5,.002);
        std::cout<<"jump,"<<j*.002<<','<<x<<','<<jump.size<<','<<jump.count<<'\n';
    }
    // Coupling is the only parameter that differs between these two regimes.
    for(int mode=0;mode<2;++mode) {
        sd::PopulationParameters p={3,0,mode?16.0:0.0,12,15,3,.02,.04,1,4,1,3};
        std::array<double,2> population={.1,.1};
        for(int j=0;j<40000;++j) {
            sd::wilsonCowanStep(population,p,4,.0001);
            if(j%10==0)std::cout<<"wilson,"<<mode<<','<<j*.0001<<','<<population[0]<<','<<population[1]<<'\n';
        }
    }
    double cells[32],scratch[160];std::fill(cells,cells+32,8);cells[0]+=.01;
    for(int j=-2000;j<4000;++j) {
        sd::lorenz96Step(cells,scratch,32,8,4,.005);
        if(j>=0) { std::cout<<"lorenz,"<<j*.005;for(double v:cells)std::cout<<','<<v;std::cout<<'\n'; }
    }
}
