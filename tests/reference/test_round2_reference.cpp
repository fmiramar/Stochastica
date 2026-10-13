#include "SDEventProcesses.hpp"
#include "SDCollective.hpp"
#include "SDVectorProcesses.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool ok, const char* message)
{
    if (!ok) throw std::runtime_error(message);
}
void near(double a, double b, double tolerance, const char* message)
{
    require(std::isfinite(a) && std::abs(a-b) <= tolerance, message);
}

int main()
{
    try {
        near(sd::eventProbability(1e-15, 1e-5), 1e-20, 1e-34, "small hazard precision");
        near(sd::eventProbability(1e30, 1.0), 1.0, 0.0, "large hazard");
        sd::PCG32 rng; rng.setSeed(123456);
        double z = 0;
        auto e = sd::selfCorrectingStep(rng, z, 0, 1, 100, 0.1);
        require(!e.event && e.intensity == 0, "zero self-correcting rate");
        e = sd::selfCorrectingStep(rng, z, 2, 0, 100, 0.1);
        near(e.intensity, 2, 0, "Poisson limit intensity");
        e = sd::selfCorrectingStep(rng, z, 1e30, 1e30, 20, 1);
        require(e.intensity <= 20 && std::isfinite(z), "explicit log ceiling");

        sd::TelegraphState telegraph;
        telegraph.reset(rng, false);
        telegraph.hazard = 0.25;
        require(telegraph.step(rng, 1, 0, 0.5) == 1 && telegraph.high, "fractional telegraph switch");
        require(telegraph.step(rng, 1, 0, 100) == 0 && telegraph.high, "absorbing high state");
        telegraph.reset(rng, false);
        require(telegraph.step(rng, 0, 1, 100) == 0 && !telegraph.high, "absorbing low state");
        require(telegraph.step(rng, 1e30, 1e30, 1) == 8 && telegraph.debt > 0,
                "bounded transitions carry time");

        sd::ShotState shot;
        shot.reset(rng);
        shot.hazard = 0.25;
        require(shot.step(rng, 1, 1, 2, 0, 0.25) == 1, "shot at interval boundary");
        near(shot.value, 2, 0, "fixed mark");
        shot.step(rng, 0, 1, 2, 0, 0.5);
        near(shot.value, 2*std::exp(-0.5), 1e-14, "analytical shot decay");
        shot.hazard = 0.01;
        require(shot.step(rng, 1, 0, 3, 0, 0.01) == 1, "instantaneous shot");
        near(shot.value, 3, 0, "instantaneous mark value");
        shot.step(rng, 0, 0, 3, 0, 0.1);
        near(shot.value, 0, 0, "instantaneous shot clears next calculation");
        require(shot.step(rng, 1e30, 1, 1, 0, 1) == 8 && shot.debt > 0,
                "bounded shots carry time");

        sd::RisingEdge edge; edge.initialize(0);
        require(edge.update(1) && !edge.update(1) && !edge.update(0) && edge.update(2), "edge rule");
        near(sd::wrapAngle(sd::kPi), -sd::kPi, 0, "positive pi wraps");
        near(sd::wrapAngle(-sd::kPi), -sd::kPi, 0, "negative pi retained");
        near(sd::wrapAngle(0.2+100*sd::kTwoPi), 0.2, 1e-12, "multiple wraps");
        require(sd::angleFloat(sd::kPi) >= -sd::kPi && sd::angleFloat(sd::kPi) < sd::kPi, "float angle range");
        for (double kappa : {0.0, 1e-9, 1.0, 10.0, 1e6, 1e30}) {
            rng.setSeed(55);
            double first = sd::vonMisesSample(rng, 3.14, kappa);
            rng.setSeed(55);
            near(sd::vonMisesSample(rng, 3.14, kappa), first, 0, "circular reset replay");
            require(first >= -sd::kPi && first < sd::kPi, "circular support");
        }
        {
            double covariance[] = {4, 1, 1, 9}, theta[] = {1, 2}, decay[2], factor[4];
            sd::ouInnovationCovariance(covariance, theta, decay, 2, 0.1);
            near(covariance[0], 2*-std::expm1(-0.2), 1e-14, "OU diagonal covariance");
            near(covariance[1], -std::expm1(-0.3)/3, 1e-14, "OU cross covariance");
            require(sd::factorCovariance(covariance, factor, 2), "OU Cholesky");
            near(factor[0]*factor[2], covariance[1], 1e-14, "LLt cross term");
            near(factor[2]*factor[2]+factor[3]*factor[3], covariance[3], 1e-14, "LLt variance");
            double brownian[] = {1, -0.5, -0.5, 1}, zero[] = {0,0};
            sd::ouInnovationCovariance(brownian, zero, decay, 2, 0.2);
            near(brownian[1], -0.1, 0, "Brownian covariance limit");
            double silent[4] = {};
            require(sd::factorCovariance(silent, factor, 2), "zero diffusion valid");
            near(factor[0]+factor[3], 0, 0, "zero diffusion exact");
            double indefinite[] = {1, 10, 10, 1};
            require(!sd::factorCovariance(indefinite, factor, 2), "indefinite diagonal fallback");
            near(factor[0], 1, 0, "fallback diagonal"); near(factor[2], 0, 0, "fallback cross term");
            double singular[] = {1,1,1,1};
            require(sd::factorCovariance(singular, factor, 2), "singular jitter");
        }
        {
            double h[4] = {}, base[] = {1,0}, alpha[] = {0,0,12,0}, decay[] = {1,1,1,1}, intensities[2];
            double events[] = {1,0};
            sd::hawkesIntensities(h,base,decay,intensities,2,100);
            near(intensities[1], 0, 0, "no same-step cascade");
            sd::hawkesExcite(h,alpha,events,2);
            sd::hawkesIntensities(h,base,decay,intensities,2,100);
            near(intensities[1], 12, 0, "target row / source column");
            near(intensities[0], 1, 0, "one-way excitation");
            double simultaneous[] = {1,1}, allAlpha[] = {2,3,5,7};
            std::fill(h,h+4,0);
            sd::hawkesExcite(h,allAlpha,simultaneous,2);
            sd::hawkesIntensities(h,base,decay,intensities,2,10);
            near(intensities[0],6,0,"simultaneous contributions"); near(intensities[1],10,0,"Hawkes ceiling");
            // A reverse-order reference accumulation with the same event vector.
            double reverse[4] = {};
            for (int j=1;j>=0;--j) for (int i=1;i>=0;--i) reverse[i*2+j]+=allAlpha[i*2+j]*simultaneous[j];
            for (int k=0;k<4;++k) near(h[k],reverse[k],0,"order-independent excitation");
        }
        for (double fixed : {-2.0, 2.0})
            near(sd::doubleWellStep(rng,fixed,0,4,1,0,1,-1,16,0.1),fixed,0,"double-well fixed points");
        require(sd::doubleWellStep(rng,0.01,0,1,1,0,1,-1,1,0.01)>0.01,"unstable origin");
        near(sd::doubleWellStep(rng,2,0,1,1,1,0,-1,1,1),2,0,"zero timeScale holds");
        near(sd::doubleWellStep(rng,1e30,0,1,-1,0,1,-1,16,1),-1,0,"divergence recovers initial");
        {
            double reference=0.2, coarse=0.2, fine=0.2;
            for(int i=0;i<10000;++i) reference=sd::doubleWellStep(rng,reference,0.1,1,1,0,1,-1,1,0.0001);
            for(int i=0;i<10;++i) {
                coarse=sd::doubleWellStep(rng,coarse,0.1,1,1,0,1,-1,1,0.1);
                fine=sd::doubleWellStep(rng,fine,0.1,1,1,0,1,-1,16,0.1);
            }
            require(std::abs(fine-reference)<std::abs(coarse-reference)*0.15,"substeps improve convergence");
        }
        {
            double p[] = {0,sd::kPi/2,-sd::kPi,-sd::kPi/2}, w[4]={}, next[4];
            near(sd::kuramotoOrder(p,4).r(),0,1e-15,"uniform phases low order");
            std::fill(p,p+4,0.2); std::fill(w,w+4,sd::kTwoPi);
            sd::kuramotoStep(rng,p,w,next,4,10,0,1,4,0.01);
            near(sd::kuramotoOrder(p,4).r(),1,1e-15,"identical phases stay coherent");
            near(p[0],0.2+0.01*sd::kTwoPi,1e-14,"cycles to radians frequency");
            double phases[] = {0.1,0.4,1.2,-0.9};
            double expect[4];
            for (int i=0;i<4;++i) {
                double drift=w[i];
                for(int j=0;j<4;++j) drift+=3.0/4*std::sin(phases[j]-phases[i]);
                expect[i]=sd::wrapAngle(phases[i]+drift*0.01);
            }
            sd::kuramotoStep(rng,phases,w,next,4,3,0,1,1,0.01);
            for(int i=0;i<4;++i) near(phases[i],expect[i],1e-14,"mean field equals pairwise equation");
        }
        std::cout << "Round 2 reference tests passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
