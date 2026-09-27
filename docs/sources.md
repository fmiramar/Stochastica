# Sources and technique notes

StochasticDynamicsUGens implements `../SC_Stochastic_Dynamics_New_UGens_Implementation.md`. Integration was developed against the matching SuperCollider 3.14.1 unit-generator guide, plugin-interface headers, server-plugin CMake helper, multi-output class pattern, RT allocation API, and shared `SndBuf` locks.

The module does not copy or modify legacy SuperCollider chaos or random UGen implementations.

## Mathematical sources

- Hénon: Michel Hénon, “A two-dimensional mapping with a strange attractor,” 1976.
- Ikeda: Kensuke Ikeda's optical ring-cavity recurrence.
- Lozi: René Lozi's 1978 piecewise-linear Hénon modification.
- Circle map: the normalized sine circle map used in mode-locking studies.
- Coupled lattice: periodic diffusive coupling of logistic-map images.
- Rulkov: Nikolai Rulkov, “Regularization of synchronized chaotic bursts,” 2001.
- Mackey–Glass: Michael Mackey and Leon Glass, “Oscillation and chaos in physiological control systems,” 1977.
- Chua: Leon Chua and collaborators' canonical piecewise-linear circuit equations.
- Izhikevich: Eugene Izhikevich, “Simple model of spiking neurons,” 2003.
- Ornstein–Uhlenbeck and Hawkes processes: their standard mean-reverting diffusion and self-exciting point-process definitions.
- Stable sampling: Chambers, Mallows, and Stuck's 1976 method, expressed in Nolan S0 form.
- Gamma sampling: Marsaglia and Tsang's 2000 method.
- Poisson sampling: Knuth's product method and Hörmann's PTRS transformed rejection.
- Inverse normal: Peter Acklam's rational approximation.
- PCG32: Melissa O'Neill's permuted congruential generator design.

The Gingerbreadman, Latoocarfian, Tinkerbell, Clifford, and Peter de Jong equations are written directly in their helpfiles because variants circulate. Help and server code use the same recurrence definitions.

## Adaptations

- Raw maps are trigger driven and contain no sonification.
- Continuous equations use double recurrence state and finite-state restoration.
- Rejection loops stop after 64 attempts and use finite fallbacks.
- Fractional noise is a stationary finite-band OU-bank approximation.
- Fractional Brownian motion is a finite-memory control-rate approximation.
- Sonifiers share raw recurrence functions but explicitly add folding, mapping, bounds, and interpolation.

