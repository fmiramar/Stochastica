#pragma once

// Independent DSS math. Provenance and intentional departures:
// docs/research/GENDY_HISTORICAL_SOURCES.md and docs/gendy-implementation.md.
#include "SDEventProcesses.hpp"
#include "SDRange.hpp"

namespace sd::gendy {
constexpr int maxPoints = 64;
constexpr int maxVoices = 32;

inline double reflect(double x, double lo, double hi)
{
    if (!(hi > lo) || !finite(x)) return lo;
    if (x>=lo && x<=hi) return x;
    // Production bounds are modest; reducing before subtraction also handles
    // arbitrarily large finite overshoots without x-lo overflow.
    const double w = hi-lo;
    const double period = 2*w;
    if (!finite(period)) { // General helper fallback for extreme finite bounds.
        return 2*reflect(x/2, lo/2, hi/2);
    }
    double rx=std::fmod(x,period), rl=std::fmod(lo,period);
    if (rx<0) rx+=period;
    if (rl<0) rl+=period;
    const double reduced=rx-rl;
    return foldValue(reduced, 0.0, w)+lo;
}

inline double bounded(double x, double lo, double hi, double fallback)
{ return std::clamp(finiteOr(x, fallback), lo, hi); }
inline int selector(double x, int lo, int hi)
{ return static_cast<int>(bounded(x, lo, hi, lo)); }

// Quantile transforms on a strictly open u. All return [-1,1]. These
// mathematical parameterizations are explicit modern choices, not a copied
// Xenakis/Collins transform table. Exponential is deliberately asymmetric.
inline double innovation(int which, double shape, double u)
{
    const double a = bounded(shape, .0001, 1, 1);
    u = bounded(u, 0x1p-33, 1-0x1p-33, .5);
    const double z = 2*u-1;
    switch (which) {
    case 1: return std::tan(z*std::atan(10*a))/(10*a);
    case 2: {
        const double q = .499*a;
        return std::atanh(2*q*z)/std::atanh(2*q);
    }
    case 3: {
        const double q = .499*a;
        return std::atanh(std::sin(kPi*q*z))/std::atanh(std::sin(kPi*q));
    }
    case 4: return std::sin(kPi*z*a/2)/std::sin(kPi*a/2);
    case 5: return 2*std::log1p(-.999*a*u)/std::log1p(-.999*a)-1;
    default: return z;
    }
}

inline double interpolate(int mode, double a, double b, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    if (mode == 0) return a;
    if (mode == 2) t = .5-.5*std::cos(kPi*t);
    if (mode == 3) t = t*t*(3-2*t); // Hermite with both endpoint tangents zero.
    return a+(b-a)*t;
}

inline double mobility(double start, double end, double elapsed, double decay)
{
    start = bounded(start, 0, 1, 0); end = bounded(end, 0, 1, 0);
    const double t = decay > 0 ? std::clamp(elapsed/decay, 0.0, 1.0) : 1;
    if (t == 0) return start;
    if (t == 1) return end;
    return start > 0 && end > 0 ? std::exp((1-t)*std::log(start)+t*std::log(end))
                               : start+(end-start)*t;
}

struct Bounds { double lo, hi; };
inline Bounds durationBounds(double fs, int n, double minFreq, double maxFreq)
{
    double f1 = bounded(minFreq, .001, fs/n, 100);
    double f2 = bounded(maxFreq, .001, fs/n, 1000);
    if (f1 > f2) std::swap(f1, f2);
    return {fs/(n*f2), fs/(n*f1)};
}

struct Point { double amp=0, dur=0, ampPrimary=0, durPrimary=0, ampProposal=0, durProposal=0; };
struct Parameters {
    int ampDist=0, durDist=0, interp=1;
    double ampShape=1, durShape=1, minFreq=100, maxFreq=1000;
    double ampStep=.1, durStep=.1, ampRange=.1, durRange=.5;
    double ampCoupling=0, durCoupling=0;
    bool secondOrder=false;
};

inline double firstOrder(double value, double delta, double step, Bounds bounds)
{ return reflect(value+delta*step*(bounds.hi-bounds.lo), bounds.lo, bounds.hi); }
inline double secondOrder(double value, double& primary, double delta,
                          double primaryStep, double rangeRatio, Bounds bounds)
{
    const double r = rangeRatio*(bounds.hi-bounds.lo);
    primary = reflect(primary+delta*primaryStep*2*r, -r, r);
    return reflect(value+primary, bounds.lo, bounds.hi);
}

struct Engine {
    Point* points=nullptr;
    PCG32 rng;
    uint32_t seed=1;
    int n=12, index=0, mode=1;
    uint64_t cycle=0;
    double fs=48000, elapsed=0, length=0, left=0, right=0;
    bool ready=false;

    void reset(const Parameters& p)
    {
        rng.setSeed(seed); cycle=0; index=0; elapsed=0; length=0;
        left=right=0; ready=false; mode=1;
        const auto d=durationBounds(fs,n,p.minFreq,p.maxFreq);
        for (int i=0;i<n;++i) { points[i]=Point{}; points[i].dur=d.lo+(d.hi-d.lo)/2; }
    }

    void updateCycle(const Parameters& p)
    {
        const auto d=durationBounds(fs,n,p.minFreq,p.maxFreq);
        const double as=bounded(p.ampStep,0,1,0), ds=bounded(p.durStep,0,1,0);
        for (int i=0;i<n;++i) {
            auto& x=points[i];
            const double da=innovation(p.ampDist,p.ampShape,openUniformDouble(rng));
            const double dd=innovation(p.durDist,p.durShape,openUniformDouble(rng));
            if (p.secondOrder) {
                x.ampProposal=secondOrder(x.amp,x.ampPrimary,da,as,bounded(p.ampRange,0,1,0),{-1,1});
                x.durProposal=secondOrder(x.dur,x.durPrimary,dd,ds,bounded(p.durRange,0,1,0),d);
            } else {
                // Keep proposals unreflected until the synchronous coupling pass.
                x.ampProposal=x.amp+2*as*da;
                x.durProposal=x.dur+ds*(d.hi-d.lo)*dd;
            }
        }
        const double ka=bounded(p.ampCoupling,0,.5,0), kd=bounded(p.durCoupling,0,.5,0);
        for (int i=0;i<n;++i) {
            auto& x=points[i]; const auto& l=points[(i+n-1)%n]; const auto& r=points[(i+1)%n];
            x.amp=reflect(x.ampProposal+ka*(l.ampProposal-2*x.ampProposal+r.ampProposal),-1,1);
            x.dur=reflect(x.durProposal+kd*(l.durProposal-2*x.durProposal+r.durProposal),d.lo,d.hi);
        }
        ++cycle;
    }

    void beginSegment(const Parameters& p)
    {
        if (ready) { left=right; index=(index+1)%n; }
        if (index==0) updateCycle(p);
        right=points[index].amp; length=points[index].dur;
        mode=selector(p.interp,0,3); ready=true;
    }

    double sample(const Parameters& p)
    {
        if (!ready) beginSegment(p);
        else if (elapsed>=length) { elapsed-=length; beginSegment(p); }
        const double out=interpolate(mode,left,right,elapsed/length);
        elapsed+=1;
        return out;
    }
};

// SplitMix-style avalanche derives a stable distinct local seed per voice.
inline uint32_t voiceSeed(uint32_t seed, uint32_t index)
{
    uint32_t x=seed+0x9e3779b9U*(index+1);
    x=(x^(x>>16))*0x85ebca6bU; x=(x^(x>>13))*0xc2b2ae35U;
    x^=x>>16; return x ? x : 1;
}
struct Field {
    PCG32 rng;
    double remaining=0;
    bool gate=false;
    uint64_t fields=0;
    void reset(uint32_t seed) { rng.setSeed(seed); remaining=0; gate=false; fields=0; }
    int step(double mean, double probability, double dt)
    {
        mean=bounded(mean,dt,1e9,1); probability=bounded(probability,0,1,.5);
        int transitions=0;
        while (remaining<=0 && transitions<8) {
            remaining+=-mean*std::log(openUniformDouble(rng));
            gate=openUniformDouble(rng)<probability; ++fields; ++transitions;
        }
        remaining-=dt; // Negative remainder is retained as bounded-work debt.
        return transitions;
    }
};
} // namespace sd::gendy
