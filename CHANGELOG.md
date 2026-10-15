# Changelog

## GENDY expansion (unreleased)

- Add Gendy1977, Gendy1991, GendyS709, GendyField, GendyPerc, GendyInterp, GendyDiffusion and GendyCoupled.
- Add a shared bounded waveform engine, local seeds/reset, independent math/statistical/lifecycle checks, and a dedicated sourced GENDY guide with reproducible figures.
- Implement the full-paper Rojas–Cádiz DDSS equation, radial/angle mapping and explicit numerical reconstruction choices, with independent traces, timestep checks and a Gaussian-DSS comparison figure. Existing Gendy1–5 and earlier Stochastica algorithms are unchanged.

## Unreleased — Round 3

- Add MaternNoise, TDirichletRand, PolyaUrn, MarkovModulatedTrig, LevyFlight, JumpDiffusion, WilsonCowan, and Lorenz96 at audio and control rates.
- Add exact half-integer Matérn transitions, log-space allocation sampling, reinforced state, hidden event regimes, stable transport, compound-Poisson shocks, and fixed-substep population/lattice flows.
- Preserve existing seeded sequences through a regression-tested categorical helper extraction.
- Add numerical, statistical, server ABI, sanitizer, stress, endurance, performance and documentation checks, with eight reproducible mathematical figures.
- Reorganize the guide around technical families and forms of memory/interaction; audit all 54 class help pages, correct unresolved comparison links, and place related links before examples while retaining earlier runnable expressions.
- Keep expansions unreleased; no existing DSP behavior is intentionally changed.

## Unreleased — Round 2

- Add MultiHawkesTrig, SelfCorrectingTrig, RandomTelegraph, ShotNoise, MultiOUProcess, DoubleWellProcess, TVonMisesRand, and KuramotoNetwork at audio and control rates.
- Add bounded event scheduling, correlated diffusion, circular sampling, metastable motion, and collective synchronization without changing existing DSP.
- Add seeded references, statistical checks, server ABI/allocation checks, server renders, and eight mathematical illustrations.
- Extend the Stochastica guide with stochastic organization and cross-process examples.

## 0.1.0 - 2026-07-29

- Added shared PCG32, rising-edge, range, distribution, map, flow, buffer, and server helpers.
- Added twelve raw maps; Mackey–Glass, Chua, and Izhikevich systems; six stochastic/event processes; ten triggered distributions; three state/multivariate processes; and three explicit map sonifiers.
- Added SuperCollider classes, individual help, a project guide, source notes, and reference/server/statistical/stress/SCDoc checks.
- Added a 35-image light-theme visualization atlas covering maps, flows, processes, events, state models, covariance, and all triggered distributions.
- Added `SDPhaseView` and exact-state live examples for the raw two-state maps and Izhikevich while retaining Chua's established live animation.
