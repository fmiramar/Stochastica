# Round 2 implementation record

## Baseline and scope

Work starts at published `main` commit `701df8c`, on the local-only branch
`round2-stochastic-organization`. The supplied plan is preserved in
[round2-plan.md](round2-plan.md). No push, release, or pull request is part of this work.
Existing DSP, classes, seeded sequences, and class help pages remain unchanged.

Before source edits, both existing CTest suites passed, and scsynth and supernova
compiled with strict warnings enabled and no project warnings. Verification uses
SuperCollider 3.14.1 headers and a build-only copy of nova-tt. The supplied 3.14.1
source checkout was missing that submodule; no upstream checkout was modified.

## Architecture and checklist

Reuse `SDRandom.hpp` (PCG32, normals), `SDCommon.hpp` (edges, finite checks),
`SDRange.hpp` (wrapping), `SDBuffer.hpp` (Cholesky), and `SDServer.hpp` (RT allocation,
input rates, seed derivation). New testable numerical helpers extend `include/`;
they do not change helpers used by existing classes. Registration extends
`Stochastica.cpp`; CMake only adds sources, classes, tests and guide assets.

| Class | Server source | Language file | Help and tests |
|---|---|---|---|
| SelfCorrectingTrig | processes/AdvancedEventProcesses.cpp | SDAdvancedProcesses.sc | Class page; Round 2 reference/statistical/server suites |
| RandomTelegraph | processes/AdvancedEventProcesses.cpp | SDAdvancedProcesses.sc | Same |
| ShotNoise | processes/ShotNoise.cpp | SDAdvancedProcesses.sc | Same |
| TVonMisesRand | distributions/CircularDistributions.cpp | SDCircularDistributions.sc | Same |
| MultiOUProcess | state/MultiOUProcess.cpp | SDMultiOUProcess.sc | Same |
| MultiHawkesTrig | processes/MultiHawkesTrig.cpp | SDMultiHawkesTrig.sc | Same |
| DoubleWellProcess | processes/DoubleWellProcess.cpp | SDDoubleWellProcess.sc | Same |
| KuramotoNetwork | flows/KuramotoNetwork.cpp | SDKuramotoNetwork.sc | Same |

All eight pages link to the existing Stochastica guide. Figures use the existing
Python standard-library raster workflow and numerical data exported by C++ helpers.
No plotting dependency enters the plugin runtime.

## Mathematical and implementation decisions (before coding)

- Self-correction: `lambda = min(maxRate, rate * exp(z))`,
  `dz/dt = correction * rate`, `z -= correction` on an event. Cap log state at
  `log(maxRate/rate)` when the ceiling is positive; zero rate/ceiling is silent.
- Telegraph and shots: carry an exponential integrated-hazard threshold across
  calculations, locate each arrival within the interval, process at most eight
  events, and carry unprocessed time. Sustained overload delays model time;
  accumulated time uses parameters of the next calculation. This is a documented
  overload approximation, not exact behavior under overloaded modulation.
- Shot signal: decay analytically between events, add Gaussian marks; nonpositive
  decay sums only this calculation's marks. Reset clears signal and timing debt.
- Circular sampler: Best–Fisher wrapped-Cauchy rejection, at most 64 attempts,
  uniform below `1e-8`, wrapped-normal asymptote above `1e6`; failure returns mean.
- OU: diagonal mean reversion, exact `Q_ij = Sigma_ij * -expm1(-s*dt)/s`,
  `s = theta_i+theta_j`, Brownian limit `Sigma_ij*dt`. Factor only at construction
  and explicit reload, using existing Cholesky and the existing bounded jitter /
  diagonal-fallback policy. An exactly zero covariance stays exactly zero.
- Hawkes: rows are targets; decay and draw all events before adding any excitation.
  Maximum 16 streams, one flag per stream/calculation, no spectral-radius work.
- Double well: fixed 1–16 Euler–Maruyama substeps for
  `dx = (a*x-b*x^3+drive)dt + abs(noise)dW`; raw state, no audio-range clipping.
- Kuramoto: fixed 1–16 Euler–Maruyama substeps, maximum 64 oscillators,
  `omega + K*(meanSin*cos(theta)-meanCos*sin(theta))`, simultaneous updates.
  Frequencies are cycles/model-second, phases wrap to `[-pi,pi)`.

Reset edges take priority over reload and trigger edges; all edge histories still
advance. Constructors initialize output without advancing continuous processes.
Triggered circular sampling draws once at construction and once on reset, matching
existing distributions. Random Kuramoto initial phases are stored together with the
post-initialization RNG state, making reset replay the first trajectory exactly.

New code must reject out-of-range buffer IDs before the existing resolver's integer
conversion and validate buffer data/dimensions while holding the shared lock. Buffer
snapshots are read sequentially, never holding duplicate/nested locks. Coherent
multi-buffer edits require `s.sync` before reload.

## Verification status

Implementation and expanded verification are in progress. Hosted Linux/macOS/
Windows CI cannot be claimed as run while publication is prohibited; local checks
and CI configuration will be reported separately.
