#pragma once
#include "SDFlow.hpp"
#include "SDEventProcesses.hpp"

namespace sd {
constexpr int kMaxPopulationSubsteps=16;
constexpr int kMaxLorenzCells=64;
constexpr double kPopulationTauFloor=1e-6;

inline double logistic(double x)
{
    if (std::isnan(x)) return 0.5;
    if (x>=0) return 1.0/(1.0+std::exp(-x));
    const double e=std::exp(x);
    return e/(1.0+e);
}
struct PopulationParameters {
    double inputExc,inputInh,wEE,wEI,wIE,wII,tauExc,tauInh,gainExc,thresholdExc,gainInh,thresholdInh;
};
inline std::array<double,2> wilsonCowanDerivative(const std::array<double,2>& x,const PopulationParameters& p)
{
    const double e=logistic(p.gainExc*(p.wEE*x[0]-p.wEI*x[1]+p.inputExc-p.thresholdExc));
    const double i=logistic(p.gainInh*(p.wIE*x[0]-p.wII*x[1]+p.inputInh-p.thresholdInh));
    return {(-x[0]+e)/p.tauExc,(-x[1]+i)/p.tauInh};
}
inline bool wilsonCowanStep(std::array<double,2>& state,const PopulationParameters& p,int substeps,double dt)
{
    if (!(dt>0) || !finite(dt)) return true;
    for (int j=0; j<substeps; ++j) {
        state=rk4<2>(state,dt/substeps,[&p](const std::array<double,2>& x){return wilsonCowanDerivative(x,p);});
        if (!finite(state[0]) || !finite(state[1])) return false;
    }
    return true;
}
inline void lorenz96Derivative(const double* x,double* out,int n,double forcing)
{
    for (int i=0; i<n; ++i) {
        const int left=(i+n-1)%n, left2=(i+n-2)%n, right=(i+1)%n;
        out[i]=(x[right]-x[left2])*x[left]-x[i]+forcing;
    }
}
inline bool lorenz96Step(double* state,double* scratch,int n,double forcing,int substeps,double dt)
{
    if (!(dt>0) || !finite(dt)) return true;
    for (int j=0; j<substeps; ++j) {
        rk4(state,n,dt/substeps,scratch,[n,forcing](const double* x,double* out){lorenz96Derivative(x,out,n,forcing);});
        for (int i=0; i<n; ++i) if (!finite(state[i])) return false;
    }
    return true;
}
} // namespace sd
