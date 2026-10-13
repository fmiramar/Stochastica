#pragma once
#include "SDBuffer.hpp"
#include "SDEventProcesses.hpp"
#include <array>

namespace sd {
constexpr double kMaternCorrelationFloor = 1e-6;
struct MaternCoefficients {
    int n;
    double lambda, a[9], l[9]; // row stride 3
    bool moving;
};

inline bool maternCoefficients(MaternCoefficients& c, int mode, double variance,
                               double correlation, double elapsed)
{
    c={}; c.n=clampInt(mode,0,2)+1;
    c.lambda=std::sqrt(2.0*c.n-1.0)/std::max(finiteOr(correlation,kMaternCorrelationFloor),kMaternCorrelationFloor);
    const int n=c.n;
    const double u=c.lambda*nonnegative(elapsed);
    c.moving=u>0.0;
    for (int i=0; i<n; ++i) c.a[i*3+i]=1.0;
    if (!c.moving) return true;
    if (!finite(u)) return false;
    // Companion generator B plus identity is nilpotent of degree n.
    double b[9]={};
    for (int i=0; i<n-1; ++i) b[i*3+i+1]=1.0;
    if (n==1) b[0]=-1;
    if (n==2) { b[3]=-1; b[4]=-2; }
    if (n==3) { b[6]=-1; b[7]=-3; b[8]=-3; }
    for (int i=0; i<n; ++i) b[i*3+i]+=1.0;
    const double exponential=std::exp(-u);
    for (int i=0; i<n; ++i) for (int j=0; j<n; ++j) {
        double square=0;
        for (int k=0; k<n; ++k) square+=b[i*3+k]*b[k*3+j];
        c.a[i*3+j]=exponential*((i==j?1.0:0.0)+u*b[i*3+j]+0.5*u*u*square);
    }
    variance=nonnegative(variance);
    if (variance==0.0) return true;

    double moment[5]={};
    const bool small=u<1.0;
    if (small) {
        // J_k = integral_0^1 t^k exp(-2*u*t) dt. Positive series avoids
        // subtracting nearly equal numbers. 32 terms suffice for 0 <= u < 1.
        for (int k=0; k<5; ++k) {
            double term=1.0/(k+1),sum=term;
            for (int j=1; j<32; ++j) { term*=2*u/(k+j+1); sum+=term; }
            moment[k]=std::exp(-2*u)*sum;
        }
    } else {
        // I_k = integral_0^u t^k exp(-2*t) dt, through degree four.
        const double tail=u>40?0.0:std::exp(-2*u);
        moment[0]=-std::expm1(-2*u)/2;
        double power=1;
        for (int k=1; k<5; ++k) {
            if (u<=40) power*=u;
            moment[k]=0.5*(k*moment[k-1]-power*tail);
        }
    }
    // exp(-v) times the polynomial impulse response of each derivative.
    // For small u, factor u^(n-i-1/2) out of row i before Cholesky. This
    // keeps tiny-step covariances well conditioned without diagonal jitter.
    double polynomial[3][3]={};
    const double v=small?u:1.0;
    if (n==1) polynomial[0][0]=1;
    if (n==2) { polynomial[0][1]=1; polynomial[1][0]=1; polynomial[1][1]=-v; }
    if (n==3) {
        polynomial[0][2]=0.5;
        polynomial[1][1]=1; polynomial[1][2]=-0.5*v;
        polynomial[2][0]=1; polynomial[2][1]=-2*v; polynomial[2][2]=0.5*v*v;
    }
    const double q=n==1?2.0:(n==2?4.0:16.0/3.0);
    double covariance[9]={},factor[9]={};
    for (int i=0; i<n; ++i) for (int j=0; j<n; ++j)
        for (int k=0; k<n; ++k) for (int m=0; m<n; ++m)
            covariance[i*n+j]+=q*polynomial[i][k]*polynomial[j][m]*moment[k+m];
    if (!choleskyFactor(covariance,factor,n)) return false;
    for (int i=0; i<n; ++i) {
        const double scale=std::sqrt(variance)*(small?std::pow(u,n-i-0.5):1.0);
        for (int j=0; j<=i; ++j) c.l[i*3+j]=scale*factor[i*n+j];
    }
    for (double x:c.a) if (!finite(x)) return false;
    for (double x:c.l) if (!finite(x)) return false;
    return true;
}

inline bool maternStep(PCG32& rng, double* state, double mean, const MaternCoefficients& c)
{
    if (!c.moving) return true;
    mean=finiteOr(mean,0.0);
    double next[3]={},z[3]={};
    bool noisy=false;
    for (double x:c.l) noisy=noisy || x!=0;
    if (noisy) for (int i=0; i<c.n; ++i) z[i]=normalSample(rng);
    for (int i=0; i<c.n; ++i) {
        for (int j=0; j<c.n; ++j)
            next[i]+=c.a[i*3+j]*(state[j]-(j==0?mean:0.0))+c.l[i*3+j]*z[j];
        if (i==0) next[i]+=mean;
        if (!finite(next[i])) return false;
    }
    std::copy(next,next+c.n,state);
    return true;
}
} // namespace sd
