#pragma once
#include "SDDistributions.hpp"
#include "SDEventProcesses.hpp"

namespace sd {
constexpr double kMaxJumpPoissonMean = 1e8;
inline bool levyFlightStep(PCG32& rng, double& state, double& increment, double alpha,
                           double scale, double drift, double dt)
{
    increment=0.0;
    if (!(dt>0.0) || !finite(dt)) return true;
    alpha=clamp(alpha,0.01,2.0); scale=std::abs(finiteOr(scale,0.0));
    increment=finiteOr(drift,0.0)*dt;
    if (scale>0.0) increment+=scale*std::pow(dt,1.0/alpha)*stableS0Sample(rng,alpha,0,1,0);
    const double next=state+increment;
    if (!finite(increment) || !finite(next)) { increment=0; return false; }
    state=next; return true;
}

struct JumpResult { double size; uint32_t count; };
inline bool jumpDiffusionStep(PCG32& rng, double& state, JumpResult& result,
                              double drift, double diffusion, double jumpRate,
                              double jumpMean, double jumpStd, double dt)
{
    result={};
    if (!(dt>0.0) || !finite(dt)) return true;
    const double lambda=nonnegative(jumpRate)*dt;
    // The shared sampler returns uint32. Reject unsupported numeric settings
    // before its candidate conversion; do not silently change an input rate.
    if (!finite(lambda) || lambda>kMaxJumpPoissonMean) return false;
    const uint32_t count=poissonSample(rng,lambda);
    diffusion=std::abs(finiteOr(diffusion,0.0)); jumpStd=std::abs(finiteOr(jumpStd,0.0));
    double increment=finiteOr(drift,0.0)*dt;
    if (diffusion>0) increment+=diffusion*std::sqrt(dt)*normalSample(rng);
    double jump=0;
    if (count>0) {
        jump=double(count)*finiteOr(jumpMean,0.0);
        if (jumpStd>0) jump+=std::sqrt(double(count))*jumpStd*normalSample(rng);
    }
    const double next=state+increment+jump;
    if (!finite(next) || !finite(jump)) return false;
    state=next; result={jump,count}; return true;
}
} // namespace sd
