# Changelog

## 0.3.0 - 2026-10-15

Consolidated major release incorporating Round 2, Round 3, and GENDY waveform synthesis expansions, bringing Stochastica to 61 UGens and the `SDPhaseView` visualization utility.

### Dynamic Stochastic Synthesis (GENDY)
- Added `Gendy77`, `Gendy91`, `GendyS709`, `GendyField`, `GendyPerc`, `GendyInterp`, `GendyDiffusion`, and `GendyCoupled`.
- Reconstructed Xenakis 1977 single-order and 1991 two-level reflected breakpoint walks.
- Added S.709 time-dependent parameter modulation and PARAG exponential/Bernoulli sound-silence gating.
- Added IDSS percussive mobility contraction and multiple interpolation modes (hold, linear, cosine, zero-tangent Hermite).
- Implemented full-paper Rojas–Cádiz Drift Dynamic Stochastic Synthesis (DDSS) with 2D Bessel diffusion and drift.
- Introduced synchronous discrete ring coupling (`GendyCoupled`).
- Added dedicated *Stochastica Gendy (Xtochastica)* guide and reproducible waveform figures.

### Memory, Transport, Allocation & Continuous Flows (Round 3)
- Added `MaternNoise`, `TDirichletRand`, `PolyaUrn`, `MarkovModulatedTrig`, `LevyFlight`, `JumpDiffusion`, `WilsonCowan`, and `Lorenz96` at audio and control rates.
- Implemented exact finite-step linear-SDE half-integer Matérn transitions with derivative memory.
- Added bounded log-space Dirichlet unit-budget allocations and Pólya urn reinforcement preference.
- Added latent Markov-modulated Poisson process (MMPP) event regimes.
- Added symmetric stable Lévy flights and compound-Poisson jump diffusion with aggregate draws.
- Added RK4 integration for Wilson–Cowan population interactions and cyclic Lorenz 96 ring dynamics.

### Stochastic Organization, Regimes & Interaction (Round 2)
- Added `MultiHawkesTrig`, `SelfCorrectingTrig`, `RandomTelegraph`, `ShotNoise`, `MultiOUProcess`, `DoubleWellProcess`, `TVonMisesRand`, and `KuramotoNetwork` at audio and control rates.
- Implemented directional cross-exciting Hawkes networks and inhibitory self-correcting point processes.
- Added two-state continuous-time random telegraph switching and analytical decaying shot noise.
- Implemented multivariate Ornstein–Uhlenbeck diffusion with Cholesky innovation covariance.
- Added double-well potential Langevin diffusion with stochastic resonance.
- Added wrapped-Cauchy Best–Fisher von Mises circular distribution sampling and all-to-all Kuramoto oscillator phase synchronization.

### Documentation, Verification & Infrastructure
- Complete documentation audit across all 62 classes with 0-warning SCDoc compliance.
- Quoted all primary academic literature in a unified bibliography.
- Added reproducible mathematical figures, rigorous test suites (unit, reference, statistical, ABI), and updated workflows.

## 0.1.0 - 2026-07-29

- Added shared PCG32, rising-edge, range, distribution, map, flow, buffer, and server helpers.
- Added twelve raw maps; Mackey–Glass, Chua, and Izhikevich systems; six stochastic/event processes; ten triggered distributions; three state/multivariate processes; and three explicit map sonifiers.
- Added SuperCollider classes, individual help, a project guide, source notes, and reference/server/statistical/stress/SCDoc checks.
- Added a 35-image light-theme visualization atlas covering maps, flows, processes, events, state models, covariance, and all triggered distributions.
- Added `SDPhaseView` and exact-state live examples for the raw two-state maps and Izhikevich while retaining Chua's established live animation.
