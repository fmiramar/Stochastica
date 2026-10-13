#pragma once
#include "SDCategorical.hpp"
#include "SDEventProcesses.hpp"

namespace sd {
constexpr int kMaxModulatedStates = 64;
struct ModulatedEvent { bool event; int state; double intensity; bool changed; };
inline ModulatedEvent modulatedEventStep(PCG32& rng, int state, const double* transitions,
                                         const double* eventRates, const double* stateRates,
                                         int n, double eventCeiling, double stateCeiling, double dt)
{
    const double intensity=std::min(eventRates[state],nonnegative(eventCeiling));
    const double exit=std::min(stateRates[state],nonnegative(stateCeiling));
    const bool event=openUniformDouble(rng)<eventProbability(intensity,dt);
    const bool opportunity=openUniformDouble(rng)<eventProbability(exit,dt);
    const int next=opportunity ? categoricalSample(rng,transitions+state*n,n,state,true) : state;
    return {event,next,intensity,next!=state};
}
} // namespace sd
