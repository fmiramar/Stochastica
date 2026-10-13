#include "SDAllocation.hpp"
#include "SDLatent.hpp"
#include "SDMatern.hpp"
#include "SDPopulation.hpp"
#include "SDTransport.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

void check(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
struct Moments {
    double sum=0,squares=0;int n=0;
    void add(double x){sum+=x;squares+=x*x;++n;}
    double mean()const{return sum/n;}
    double variance()const{return squares/n-mean()*mean();}
};
int main()
{
    try {
        sd::PCG32 rng;
        for(double concentration:{.2,1.0,8.0}) {
            double alpha[]={concentration,concentration,concentration},values[3];
            rng.setSeed(771); Moments components[3],maximum,product;
            for(int i=0;i<100000;++i) {
                sd::dirichletSample(rng,alpha,values,3);
                for(int j=0;j<3;++j)components[j].add(values[j]);
                maximum.add(*std::max_element(values,values+3));
                product.add(values[0]*values[1]);
            }
            const double covariance=product.mean()-components[0].mean()*components[1].mean();
            check(std::abs(covariance+(1.0/9)/(3*concentration+1))<.003,"Dirichlet cross covariance");
            for(auto m:components) {
                check(std::abs(m.mean()-1.0/3)<.006,"Dirichlet symmetric mean");
                check(std::abs(m.variance()-(2.0/9)/(3*concentration+1))<.003,"Dirichlet variance");
            }
            if(concentration==.2)check(maximum.mean()>.8,"small alpha favors corners");
            if(concentration==8)check(maximum.mean()<.46,"large alpha balanced");
            std::cout<<"Dirichlet alpha="<<concentration<<" mean="<<components[0].mean()<<" max="<<maximum.mean()<<'\n';
        }
        double alpha[]={6,1,1},values[3];Moments asymmetric;
        for(int i=0;i<100000;++i){sd::dirichletSample(rng,alpha,values,3);asymmetric.add(values[0]);}
        check(std::abs(asymmetric.mean()-.75)<.005,"Dirichlet asymmetric mean");

        Moments proportions[3],entropy;
        for(int run=0;run<1200;++run) {
            rng.setSeed(2000+run);double weights[]={1,1,1},scale=1;
            for(int j=0;j<500;++j)sd::urnDraw(rng,weights,3,1,scale);
            for(int j=0;j<3;++j)proportions[j].add(weights[j]/sd::weightTotal(weights,3));
            entropy.add(sd::normalizedEntropy(weights,3));
        }
        for(auto m:proportions)check(std::abs(m.mean()-1.0/3)<.025 && m.variance()>.035,"urn ensemble symmetry and path dependence");
        check(entropy.mean()<.82,"reinforcement reduces mean diversity");
        std::cout<<"Urn mean proportion="<<proportions[0].mean()<<" variance="<<proportions[0].variance()<<" entropy="<<entropy.mean()<<'\n';
        double fixed[]={1,2,3},scale=1;int counts[3]={};
        for(int i=0;i<100000;++i)++counts[sd::urnDraw(rng,fixed,3,0,scale).state];
        for(int i=0;i<3;++i)check(std::abs(counts[i]/100000.0-(i+1)/6.0)<.006,"zero reinforcement categorical law");

        // Audio-rate hidden regimes: measure against the represented Bernoulli
        // hazard (the O(dt) difference from continuous time is explicit).
        double transitions[]={0,1,0,.3,0,.7,1,0,0},eventRates[]={400,1200,2400},stateRates[]={200,100,300};
        int state=0,dwell=0,occupancy[3]={},observed[3]={},destinations[3]={};Moments dwells[3];
        constexpr double dt=1.0/48000;
        rng.setSeed(143);
        for(int i=0;i<2000000;++i) {
            const int old=state;++occupancy[old];++dwell;
            auto e=sd::modulatedEventStep(rng,state,transitions,eventRates,stateRates,3,20000,20000,dt);
            observed[old]+=e.event;state=e.state;
            if(e.changed){dwells[old].add(dwell*dt);dwell=0;if(old==1)++destinations[state];}
        }
        for(int i=0;i<3;++i) {
            const double rate=observed[i]/(occupancy[i]*dt);
            const double expected=sd::eventProbability(eventRates[i],dt)/dt;
            check(std::abs(rate/expected-1)<.1,"conditional event intensity");
            check(std::abs(dwells[i].mean()*stateRates[i]-1)<.12,"latent exponential dwell");
            std::cout<<"Modulated state="<<i<<" rate="<<rate<<" dwell="<<dwells[i].mean()<<'\n';
        }
        check(std::abs(double(destinations[0])/(destinations[0]+destinations[2])-.3)<.04,"transition row proportions");

        for(double elapsed:{1.0/48000,1.0/750}) for(int mode=0;mode<3;++mode) {
            const double correlation=elapsed*96;
            sd::MaternCoefficients c;
            check(sd::maternCoefficients(c,mode,2,correlation,elapsed),"Matérn statistical coefficients");
            rng.setSeed(809+mode);double x[3]={};
            for(int i=0;i<3000;++i)sd::maternStep(rng,x,.6,c);
            std::vector<double> data(500000);Moments m,roughness;
            for(size_t i=0;i<data.size();++i){sd::maternStep(rng,x,.6,c);data[i]=x[0];m.add(x[0]);if(i)roughness.add(x[0]-data[i-1]);}
            check(std::abs(m.mean()-.6)<.12,"Matérn mean");
            check(std::abs(m.variance()-2)<.18,"Matérn stationary variance");
            for(int lag:{24,96,192}) {
                double sum=0;for(size_t i=lag;i<data.size();++i)sum+=(data[i]-.6)*(data[i-lag]-.6);
                const double u=c.lambda*elapsed*lag;
                const double expected=2*std::exp(-u)*(1+(mode>=1?u:0)+(mode==2?u*u/3:0));
                check(std::abs(sum/(data.size()-lag)-expected)<.18,"Matérn autocovariance");
            }
            if(mode==0)check(roughness.variance()>.025,"OU roughness");
            else check(roughness.variance()<.003,"higher Matérn smoothness");
            std::cout<<"Matern dt="<<elapsed<<" mode="<<mode<<" mean="<<m.mean()<<" variance="<<m.variance()<<" increment variance="<<roughness.variance()<<'\n';
        }

        double quantileRatio[2];
        for(int mode=0;mode<2;++mode) {
            rng.setSeed(129);std::vector<double> magnitudes(100000);Moments serial,signs;double previous=0;
            for(size_t i=0;i<magnitudes.size();++i) {
                double x=0,delta=0;sd::levyFlightStep(rng,x,delta,mode?1.2:2,1,0,.01);
                magnitudes[i]=std::abs(delta);if(i)serial.add((delta>0?1.0:-1.0)*previous);previous=delta>0?1:-1;
                signs.add(previous);
            }
            check(std::abs(serial.mean())<.015,"stable innovation sign independence");
            check(std::abs(signs.mean())<.015,"symmetric stable sign balance");
            std::sort(magnitudes.begin(),magnitudes.end());quantileRatio[mode]=magnitudes[99000]/magnitudes[50000];
        }
        check(quantileRatio[1]>quantileRatio[0]*4,"stable heavy tails");
        std::cout<<"Levy q99/q50 Gaussian="<<quantileRatio[0]<<" alpha1.2="<<quantileRatio[1]<<'\n';
        for(double alphaValue:{1.2,1.7,2.0}) {
            Moments cf[2];
            for(int mode=0;mode<2;++mode) {
                rng.setSeed(17+mode);
                for(int trial=0;trial<30000;++trial) {
                    double x=0,delta;
                    const int steps=mode?1:64;
                    for(int k=0;k<steps;++k)sd::levyFlightStep(rng,x,delta,alphaValue,1,0,1.0/steps);
                    cf[mode].add(std::cos(x));
                }
                check(std::abs(cf[mode].mean()-std::exp(-1))<.014,"stable finite-time characteristic function");
            }
        }
        for(int mode=0;mode<2;++mode) {
            rng.setSeed(99+mode);Moments totals,counts;
            for(int trial=0;trial<60000;++trial) {
                double x=0;sd::JumpResult jump;int count=0;
                const int steps=mode?1:64;
                for(int k=0;k<steps;++k){check(sd::jumpDiffusionStep(rng,x,jump,.3,.7,3,.4,.8,.1/steps),"jump transition");count+=jump.count;}
                totals.add(x);counts.add(count);
            }
            check(std::abs(totals.mean()-.15)<.012,"jump mean finite interval");
            check(std::abs(totals.variance()-.289)<.015,"jump variance finite interval");
            check(std::abs(counts.mean()-.3)<.012,"Poisson count mean");
            std::cout<<"Jump steps="<<(mode?1:64)<<" mean="<<totals.mean()<<" variance="<<totals.variance()<<" count="<<counts.mean()<<'\n';
        }
        std::cout<<"Round 3 statistical tests passed\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
