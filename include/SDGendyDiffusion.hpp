#pragma once
#include "SDGendy.hpp"

// Independent reconstruction of Rojas/Cadiz DDSS, equation 19 (CMJ 45:2,
// pp. 56-60). Numerical regularization and mapping choices are documented in
// docs/research/GendyDiffusion-source-notes.md; no upstream code is incorporated.
namespace sd::gendy {
constexpr double radiusFloor = 1e-6;
inline double diffusionRadius(double radius, double diffusivity, double dimensions,
                              double dt, double normal)
{
    if (diffusivity == 0 || dt == 0) return radius;
    const double drift = diffusivity*(dimensions-1)*dt/std::max(radius,radiusFloor);
    return std::abs(radius+drift+std::sqrt(2*diffusivity*dt)*normal);
}
inline double diffusionNormal(PCG32& rng)
{
    const double u=openUniformDouble(rng), v=openUniformDouble(rng);
    return std::sqrt(-2*std::log(u))*std::cos(kTwoPi*v);
}
inline double diffusionAmplitude(double radius, double sine, double cosine, double modulation)
{
    return reflect(radius,-1,1)*(sine+.1*cosine*modulation)/std::sqrt(1.01);
}
struct DiffusionPoint { double radius=1, sine=0, cosine=1, amp=0; };
struct DiffusionParameters {
    double freq=220, diffusivity=1, dimensions=2, timeStep=.001, modFreq=.2;
};
inline DiffusionParameters diffusionParameters(DiffusionParameters p,double fs,int n)
{
    p.freq=bounded(p.freq,.001,fs/(n+1),220);
    p.diffusivity=bounded(p.diffusivity,0,10,1);
    p.dimensions=bounded(p.dimensions,1,16,2);
    p.timeStep=bounded(p.timeStep,0,.01,.001);
    p.modFreq=bounded(p.modFreq,0,fs/2,.2);
    return p;
}
struct DiffusionEngine {
    DiffusionPoint* points=nullptr;
    PCG32 rng;
    uint32_t seed=1;
    int n=36;
    double fs=48000, phase=0, modPhase=0;
    uint64_t cycle=0;
    bool ready=false;
    DiffusionParameters parameters;

    void reset()
    {
        rng.setSeed(seed);phase=modPhase=0;cycle=0;ready=false;
        for(int j=0;j<n;++j) {
            points[j]=DiffusionPoint{};
            const double angle=kTwoPi*j/n;
            points[j].sine=std::sin(angle);points[j].cosine=std::cos(angle);
        }
    }
    void updateShape(bool advance)
    {
        const double modulation=std::sin(kTwoPi*modPhase);
        for(int j=0;j<n;++j) {
            auto& p=points[j];
            if(advance) p.radius=diffusionRadius(p.radius,parameters.diffusivity,
                parameters.dimensions,parameters.timeStep,diffusionNormal(rng));
            p.amp=diffusionAmplitude(p.radius,p.sine,p.cosine,modulation);
        }
    }
    double sample(const DiffusionParameters& p)
    {
        if(!ready || phase>=1) {
            const bool advance=ready;
            if(advance){phase-=1;++cycle;}
            parameters=diffusionParameters(p,fs,n);updateShape(advance);ready=true;
        }
        const double position=phase*(n+1);
        const int index=std::min(n,static_cast<int>(position));
        const double left=index==0 ? 0 : points[index-1].amp;
        const double right=index==n ? 0 : points[index].amp;
        const double out=interpolate(1,left,right,position-index);
        phase+=parameters.freq/fs;
        modPhase+=parameters.modFreq/fs;modPhase-=std::floor(modPhase);
        return out;
    }
};
} // namespace sd::gendy
