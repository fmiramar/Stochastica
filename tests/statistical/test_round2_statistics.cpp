#include "SDEventProcesses.hpp"
#include "SDCollective.hpp"
#include "SDVectorProcesses.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct Moments {
    double sum = 0, square = 0; int count = 0;
    void add(double x) { sum += x; square += x*x; ++count; }
    double mean() const { return sum/count; }
    double variance() const { return square/count - mean()*mean(); }
};

int main()
{
    try {
        double fano[2], shortFraction[2];
        for (int mode = 0; mode < 2; ++mode) {
            sd::PCG32 rng; rng.setSeed(117);
            double z = 0; int total = 0, window = 0, previous = -100000, shortCount = 0;
            Moments windows;
            for (int i = 0; i < 1000000; ++i) {
                auto result = sd::selfCorrectingStep(rng, z, 8, mode ? 1.5 : 0, 10000, 0.001);
                if (result.event) { ++total; ++window; if (i-previous < 20) ++shortCount; previous = i; }
                if ((i+1)%1000 == 0) { windows.add(window); window = 0; }
            }
            check(std::abs(total/1000.0-8) < 0.3, "self-correcting / Poisson rate");
            fano[mode] = windows.variance()/windows.mean();
            shortFraction[mode] = double(shortCount)/total;
        }
        check(fano[0] > 0.8 && fano[0] < 1.2, "Poisson count dispersion");
        std::cout << "Fano " << fano[0] << "," << fano[1] << " short " << shortFraction[0] << "," << shortFraction[1] << std::endl;
        check(fano[1] < 0.3 && shortFraction[1] < shortFraction[0]*0.7, "inhibition suppresses clustering");
        std::cout << "Fano Poisson=" << fano[0] << " correcting=" << fano[1] << '\n';

        sd::PCG32 rng; rng.setSeed(4321);
        sd::TelegraphState telegraph; telegraph.reset(rng, false);
        int high = 0, last = 0; Moments lowDwell, highDwell;
        for (int i = 0; i < 1000000; ++i) {
            bool before = telegraph.high;
            telegraph.step(rng, 2, 1, 0.001);
            high += telegraph.high;
            if (before != telegraph.high) {
                (before ? highDwell : lowDwell).add((i-last)*0.001); last = i;
            }
        }
        check(std::abs(high/1000000.0 - 2.0/3.0) < 0.04, "telegraph asymmetric occupancy");
        check(std::abs(lowDwell.mean()-0.5)<0.06 && std::abs(highDwell.mean()-1)<0.1, "telegraph dwell means");
        std::cout << "Telegraph occupancy=" << high/1000000.0 << " dwells=" << lowDwell.mean() << ',' << highDwell.mean() << '\n';

        sd::ShotState shot; rng.setSeed(9876); shot.reset(rng); Moments shots;
        for (int i = 0; i < 1500000; ++i) {
            shot.step(rng, 20, 0.15, 0.7, 0.6, 0.002);
            if (i > 5000) shots.add(shot.value);
        }
        check(std::abs(shots.mean()-2.1)<0.09, "shot stationary mean");
        check(std::abs(shots.variance()-1.275)<0.12, "shot stationary variance");
        std::cout << "Shot mean=" << shots.mean() << " variance=" << shots.variance() << '\n';
        const double expectedR[] = {0,0.4463899659,0.6977746580,0.8933831370};
        int index=0;
        for(double kappa : {0.0,1.0,2.0,5.0}) {
            rng.setSeed(321); double c=0,s=0;
            for(int i=0;i<200000;++i) {
                double angle=sd::vonMisesSample(rng,3.13,kappa);
                check(angle>=-sd::kPi && angle<sd::kPi,"von Mises support");
                c+=std::cos(angle-3.13); s+=std::sin(angle-3.13);
            }
            c/=200000; s/=200000;
            check(std::abs(c-expectedR[index++])<0.008 && std::abs(s)<0.008,"von Mises circular moments I1/I0");
            std::cout << "von Mises kappa=" << kappa << " resultant=" << std::hypot(c,s) << '\n';
        }
        for(double kappa : {999999.0,1000001.0}) {
            Moments angles; rng.setSeed(600);
            for(int i=0;i<100000;++i) angles.add(sd::wrapAngle(sd::vonMisesSample(rng,-3.13,kappa)+3.13));
            check(std::abs(angles.variance()*kappa-1)<0.03,"large concentration continuity");
        }
        {
            double q[]={4,-1,-1,2},theta[]={1,2},means[]={2,-1},decay[2],factor[4],normals[2],v[]={2,-1};
            sd::ouInnovationCovariance(q,theta,decay,2,0.01); sd::factorCovariance(q,factor,2);
            rng.setSeed(888); Moments x,y,cross;
            for(int i=0;i<1500000;++i) {
                sd::multiOUStep(rng,v,means,decay,factor,normals,2);
                if(i>10000){x.add(v[0]);y.add(v[1]);cross.add((v[0]-2)*(v[1]+1));}
            }
            check(std::abs(x.mean()-2)<0.08 && std::abs(y.mean()+1)<0.05,"OU means");
            check(std::abs(x.variance()-2)<0.12 && std::abs(y.variance()-0.5)<0.04,"OU stationary variances");
            check(std::abs(cross.mean()+1.0/3)<0.045,"OU negative cross covariance");
            std::cout << "OU covariance=" << cross.mean() << " variances=" << x.variance() << ',' << y.variance() << '\n';
            // N=1 uses the exact scalar OU transition, with the same normal stream.
            double scalarQ[]={0.49},scalarTheta[]={1.3},scalarDecay[1],scalarFactor[1],mu[]={0.2},a[]={0.2},z1[1],b=0.2;
            sd::ouInnovationCovariance(scalarQ,scalarTheta,scalarDecay,1,0.002);
            sd::factorCovariance(scalarQ,scalarFactor,1);
            sd::PCG32 other; rng.setSeed(777); other.setSeed(777);
            for(int i=0;i<10000;++i) {
                sd::multiOUStep(rng,a,mu,scalarDecay,scalarFactor,z1,1);
                b=0.2+std::exp(-1.3*0.002)*(b-0.2)+std::sqrt(0.49*(-std::expm1(-2.6*0.002))/2.6)*sd::normalSample(other);
                check(std::abs(a[0]-b)<1e-12,"scalar OU equivalence");
            }
        }
        for(int mode=0;mode<3;++mode) {
            double base[]={2,2},alpha[]={0,0,0,0},decay[4],h[4]={},intensities[2],events[2];
            if(mode==1) alpha[0]=alpha[3]=4;
            if(mode==2) alpha[2]=8;
            std::fill(decay,decay+4,std::exp(-10*0.001)); rng.setSeed(990);
            double total[2]={},following=0,background=0; int lastSource=-1000,afterSamples=0;
            for(int i=0;i<2000000;++i) {
                sd::multiHawkesStep(rng,h,base,alpha,decay,intensities,events,2,20000,0.001);
                total[0]+=events[0];total[1]+=events[1];
                if(i-lastSource<100){ following+=events[1];++afterSamples;} else background+=events[1];
                if(events[0])lastSource=i;
            }
            double expected0=mode==1?2.0/(1-0.4):2;
            double expected1=mode==1?expected0:(mode==2?3.6:2);
            check(std::abs(total[0]/2000-expected0)<0.2 && std::abs(total[1]/2000-expected1)<0.25,"Hawkes mean rates");
            if(mode==2) check(following/afterSamples>background/(2000000-afterSamples)*1.7,"directional post-event excitation");
            std::cout << "Hawkes mode=" << mode << " rates=" << total[0]/2000 << ',' << total[1]/2000 << '\n';
        }
        int crossings[2]={};
        for(int mode=0;mode<2;++mode) {
            rng.setSeed(456); double x=-1; bool well=false;
            for(int i=0;i<500000;++i) {
                x=sd::doubleWellStep(rng,x,0,1,1,mode?0.8:0.08,1,-1,2,0.005);
                if((well && x<-.5)||(!well && x>.5)){++crossings[mode];well=!well;}
            }
        }
        check(crossings[0]==0 && crossings[1]>50,"metastable transitions");
        std::cout << "Well transitions low=" << crossings[0] << " moderate=" << crossings[1] << '\n';
        double finalOrder[3];
        for(int mode=0;mode<3;++mode) {
            double phases[16],omega[16],next[16]; rng.setSeed(987);
            for(int i=0;i<16;++i){phases[i]=-2.5+i*0.31;omega[i]=sd::kTwoPi*(1+(i-7.5)*0.004);}
            Moments order;
            for(int i=0;i<15000;++i){
                sd::kuramotoStep(rng,phases,omega,next,16,mode?3:0,mode==2?1.5:0,1,4,0.002);
                const double r=sd::kuramotoOrder(phases,16).r();check(r>=0 && r<=1,"order bounds");
                if(i>10000)order.add(r);
            }
            finalOrder[mode]=order.mean();
        }
        check(finalOrder[1]>0.97 && finalOrder[1]>finalOrder[0]+0.4 && finalOrder[2]<finalOrder[1]-0.1,"coupling synchronizes, noise disperses");
        std::cout << "Kuramoto order uncoupled=" << finalOrder[0] << " coupled=" << finalOrder[1] << " noisy=" << finalOrder[2] << '\n';
        std::cout << "Round 2 statistical tests passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
