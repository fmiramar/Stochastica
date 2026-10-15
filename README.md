![Stochastica Banner](docs/assets/stochastica_banner.png)

# Stochastica

Stochastica provides mathematical SuperCollider UGens for generative music: nonlinear maps and flows, shaped probability distributions, stochastic trajectories, event timing, discrete state and interacting populations. Raw state is separate from sonic mapping, so a patch can expose what a model remembers and how it organizes change.

Deterministic maps and flows can be periodic, excitable or chaotic depending on parameters. Stochastic models consume random innovations. Independent triggered distributions, mean-reverting motion, event feedback and accumulated preference introduce different kinds of structure; the installed Stochastica guide explains these relationships across all 61 UGens and the language-side `SDPhaseView` utility.

This remains in BiroSCa as an experimental SuperCollider-native collection independently implementing mathematical techniques, rather than wrapping an upstream plugin. Equations, adaptations and citations are documented in each help page and the [source notes](docs/sources.md).

## GENDY waveform expansion (unreleased)

Eight additions bring stochastic processes directly into evolving waveform geometry. The dedicated **Stochastica-GENDY** help guide explains their historical sources, mathematical differences and practical use alongside the existing Gendy1–5 family.

- `Gendy1977` evolves remembered amplitudes and segment durations with first-order reflected walks. It reconstructs the method associated with Xenakis's 1977 DSS, using explicit modern seeding and duration scaling.
- `Gendy1991` gives each breakpoint two levels of stochastic memory. Primary reflected walks drive secondary amplitude/duration walks, following the method associated with GENDY3 and crediting Xenakis and Marie-Hélène Serra.
- `GendyS709` periodically changes the rules of first-order waveform motion. Sine/saw modulation is historically informed; its exponential control routes are explicit Stochastica choices.
- `GendyField` supplies independently timed sound/silence gates. Exponential durations and Bernoulli activity implement the PARAG timing subset, without recreating its complete sections.
- `GendyPerc` contracts stochastic mobility after each trigger. Brown and Jenkins's IDSS work informs its two-level walk and transition from rough motion to a stable polygon, independently of volume envelopes.
- `GendyInterp` renders identical breakpoint histories with hold, linear, cosine or zero-tangent cubic Hermite interpolation. This extends the rendering vocabulary without claiming exact equivalence to sc3-plugins Gendy4/5.
- `GendyDiffusion` maps radial diffusion into waveform amplitudes at fixed pitch. It implements Rojas and Cádiz’s published Bessel-process DDSS concept with explicit timestep, origin regularization and angular-mapping choices.
- `GendyCoupled` makes neighboring breakpoint proposals interact around a ring. Its synchronous discrete Laplacian is a new Stochastica design, with exact uncoupled-baseline behavior at zero coupling.

Waveform classes are audio-only with 2–64 points; GendyField is control-only with 1–32 gates. These oscillators are not bandlimited and can contain DC. Bounds, initialization, scheduling and interpolation differ intentionally from the original programs. GendyDiffusion uses the full-paper equations with documented numerical choices; it is not a bit-exact recreation of the authors’ Max instrument.

See the [implementation decisions](docs/gendy-implementation.md), [research audit](docs/research/GENDY_HISTORICAL_SOURCES.md), and [verification record](docs/gendy-verification.md).

## Round 3 expansion (unreleased)

Eight additions develop memory, hidden organization, constrained geometry, transport and collective dynamics. They continue the existing Stochastica project; existing DSP and seeded behavior are preserved.

- `MaternNoise` generates Gaussian trajectories with separate variance, correlation-time and smoothness controls. Exact finite-step linear-SDE transitions implement the half-integer Matérn family through derivative-state memory.
- `TDirichletRand` distributes a unit budget among nonnegative shares on each trigger. Bounded Gamma sampling in log space preserves sparse allocations at small concentrations.
- `PolyaUrn` reinforces the probability of previously selected states. Private accumulated weights expose path dependence, pre-selection probability and post-selection entropy.
- `MarkovModulatedTrig` gives one event stream persistent quiet or dense regimes. A latent Markov state controls bounded discrete-hazard event timing independently of observed events.
- `LevyFlight` accumulates symmetric stable increments into unbounded transport. Exact elapsed-time scaling distinguishes heavy-tailed relocations from correlated fractional motion.
- `JumpDiffusion` combines Brownian wandering with finite-rate Gaussian-marked Poisson shocks. An aggregate jump draw exposes state, jump flag, displacement and count without a per-shock loop.
- `WilsonCowan` models feedback between excitatory and inhibitory population activities. Fixed-substep RK4 integrates a normalized logistic variant of the population equations with optional external stochastic drive.
- `Lorenz96` evolves locally coupled continuous states around a ring. Linear-cost cyclic RK4 stages expose spatially extended deterministic dynamics, distinct from trigger-driven coupled maps.

Each addition has audio/control rates, a sourced help page, small runnable examples and a reproducible figure. The guide audits the original collection and both expansions, with comparisons of memory, regimes, movement and interaction. See the [Round 3 implementation](docs/round3-implementation.md) and [verification record](docs/round3-verification.md).

Dirichlet dimension is at most 32; urn states, latent states and Lorenz cells at most 64. Event/state rates in MarkovModulatedTrig should stay well below the calculation rate. Matérn coefficient parameters are sampled at block entry. JumpDiffusion rejects Poisson means above 1e8 per calculation. Flow substeps are limited to 16; 64-cell Lorenz96 at 16 audio-rate substeps can exceed a core's real-time budget. Heavy-tailed/raw outputs require explicit sonic mapping, and the flows do not provide antialiasing.

## Round 2 expansion (unreleased)

Eight new models extend Stochastica's stochastic organization. This work remains an experimental, unreleased expansion; existing UGen behavior is preserved.

- `MultiHawkesTrig` spreads activity between event streams through directional cross-excitation. A bounded exponential-kernel Hawkes network draws simultaneous events before updating its interaction matrix.
- `SelfCorrectingTrig` produces random events that resist clustering. Its log-intensity builds during silence and falls after an event, providing inhibitory feedback beside `HawkesTrig`.
- `RandomTelegraph` switches between two persistent regimes with asymmetric random dwell times. Integrated exponential hazards preserve timing under ordinary rate modulation.
- `ShotNoise` accumulates Gaussian-marked Poisson impacts into a decaying signal. Analytical inter-event decay connects isolated particles to dense continuous textures.
- `MultiOUProcess` generates correlated trajectories with temporal memory. Exact finite-step OU covariance combines the scalar process's mean reversion with `MultiGaussNoise`-style matrix handling.
- `DoubleWellProcess` produces continuous metastable motion and noise-driven barrier crossings. Fixed Euler–Maruyama substeps expose the raw quartic-potential dynamics without audio-range clipping.
- `TVonMisesRand` samples angles around a preferred direction. Bounded Best–Fisher rejection treats the wrap boundary as part of a circle.
- `KuramotoNetwork` models an oscillator population that can synchronize through mutual coupling. A linear-cost mean-field update exposes every phase, coherence, and mean phase.

Telegraph and shot scheduling process at most eight internal events per calculation; overload carries time forward and can delay the model. Hawkes/self-correcting events use a discrete hazard approximation, so individual rates should remain well below the calculation rate. Vector reloads are explicit snapshots; OU factorization costs more than an ordinary update. Kuramoto and double-well integration need suitable time steps, and raw outputs are not normalized or antialiased.

See the installed Stochastica guide, [implementation record](docs/round2-implementation.md), and [verification record](docs/round2-verification.md). This expansion stays in BiroSCa because it develops experimental SuperCollider-native generative systems from mathematical techniques rather than wrapping an upstream plugin.

## Design

- A raw map updates recurrence state only on a rising trigger.
- A flow integrates differential equations at the UGen calculation rate.
- A distribution sampler produces an independent value on a rising trigger.
- A stochastic process carries random state through time.
- An event process emits one-sample or one-control-block events.
- A sonifier explicitly folds and maps raw-map state into audio.

Seeded classes use a local PCG32 generator. An explicit nonzero seed repeats the local algorithmic sequence for the same inputs and calculation rate; zero derives a seed from the parent Synth generator at construction. Reset restores the original local seed. Floating-point library differences may affect last-bit values across platforms.

## Classes

### Raw maps

- `HenonMap` outputs both coordinates of the Hénon recurrence. It samples parameters on trigger edges, holds between events, and resets rather than conditioning divergent state.
- `GbmanMap` outputs the Gingerbreadman map's two raw coordinates. Its recurrence is shared with the explicit `GbmanSonify` wrapper.
- `LatoocarfianMap` exposes one documented two-sine Latoocarfian recurrence. No interpolation, normalization, or frequency mapping occurs inside the raw map.
- `IkedaMap` implements the two-dimensional Ikeda map. Parameters remain unclipped and non-finite results restore the requested initial state.
- `LoziMap` implements the piecewise-linear Lozi recurrence. It holds raw state between rising trigger edges.
- `TinkerbellMap` emits both coordinates of the Tinkerbell polynomial map. Old-state values are used consistently for both next coordinates.
- `CliffordMap` implements Clifford's trigonometric attractor equations. Output is mathematical state rather than normalized audio.
- `DeJongMap` implements Peter de Jong's two-dimensional trigonometric recurrence. It is trigger driven and exposes both coordinates.
- `CircleMap` implements the normalized standard circle map. Wrapping to `[0, 1)` is part of the map definition.
- `CoupledLogisticMap` implements two symmetrically coupled logistic maps. It deliberately leaves states outside `[0, 1]` visible.
- `CoupledMapLattice` runs 2–64 periodically coupled logistic cells. Two RT-allocated arrays are swapped after each trigger-driven update.
- `RulkovMap` implements the two-state discrete Rulkov neuron model. It can expose quiescent, spiking, or bursting trajectories.

### Continuous systems

- `MackeyGlass` integrates the Mackey–Glass delay equation. It uses an RT-allocated circular history, cubic delay reads, and a fixed Heun step.
- `Chua` outputs all three states of Chua's circuit equations. It uses fixed-substep RK4 integration without output normalization.
- `Izhikevich` outputs membrane potential, recovery state, and a one-calculation spike trigger. It uses the canonical two-half-step voltage update.

### Stochastic and event processes

- `OUProcess` implements the exact finite-step Ornstein–Uhlenbeck transition and its Brownian limit. Control rate uses the full control duration.
- `RenewalTrig` schedules exponential, gamma, Weibull, lognormal, or Erlang intervals. It emits at most one event per calculation and carries timing remainder.
- `HawkesTrig` implements a bounded self-exciting point process. Its second output is the pre-event intensity used for the probability.
- `FractionalNoise` approximates finite-band `1/f^alpha` noise with normalized logarithmically spaced OU components. Quality selects 4–32 components.
- `FBrownianMotion` is a control-rate finite-memory ARFIMA-style fractional Brownian approximation. It filters normalized Gaussian innovations.
- `BoundedWalk` provides reflect, wrap, clip, bounded-resample, and absorb rules with uniform, normal, or Cauchy increments.

### Triggered distributions

- `TGammaRand` uses bounded Marsaglia–Tsang gamma sampling. Shape below one uses the standard shape-boost transform.
- `TWeibullRand` uses an open-uniform inverse transform. Shape and scale retain their conventional meanings.
- `TLogNormalRand` exponentiates a normal draw and converts overflow to the largest finite float. Sigma is interpreted by magnitude.
- `TCauchyRand` uses the tangent inverse transform. Extreme finite values are expected because no finite mean or variance exists.
- `TPoissonRand` uses bounded Knuth sampling for small means and PTRS rejection for large means. Output is an integer represented as float.
- `TGeometricRand` returns failures before the first success. Endpoint probabilities have explicit finite fallbacks.
- `TNegBinomialRand` uses a gamma–Poisson mixture and supports positive real success counts. It returns failures before the requested successes.
- `TTruncNormalRand` uses normal-CDF and inverse-CDF sampling. It samples the conditioned distribution instead of clipping a normal draw.
- `TStableRand` uses Chambers–Mallows–Stuck sampling in Nolan's S0 parameterization. Gaussian and Cauchy special cases are documented.
- `TLevyRand` uses the inverse-square normal construction with a bounded retry loop. It has one-sided support and an expected heavy tail.

### State and multivariate processes

- `MarkovChain` samples a live row-major transition-weight buffer and optionally maps states through a value buffer. User buffers are never rewritten.
- `SemiMarkov` combines the same matrix with per-state fixed, exponential, gamma, Weibull, or lognormal dwell times. It reports change and remaining time.
- `MultiGaussNoise` factors a covariance buffer at construction, reset, or reload. Failed Cholesky attempts use bounded diagonal jitter and a diagonal fallback.

### Explicit sonifiers

- `HenonSonify`, `GbmanSonify`, and `LatoocarfianSonify` map selected coordinates to iteration frequency and level. Folding ranges, selection, frequency bounds, and hold/linear/cubic-Hermite interpolation are explicit.

### Visualization utility

- `SDPhaseView` is a language-side bounded phase-portrait window for two-output state UGens. It visualizes and sonifies one shared server state, transports coordinates only at display rate, and performs all storage and drawing outside the audio thread.

## Building

```bash
cmake -S . -B build \
  -DSC_PATH=/path/to/supercollider-source \
  -DCMAKE_BUILD_TYPE=Release \
  -DSTRICT=ON
cmake --build build --config Release
cmake --install build --config Release
```

Install the staged `Stochastica` folder in the SuperCollider user extension directory and recompile the class library. For supernova, configure with `-DSUPERNOVA=ON`.

Configure with `-DBUILD_TESTING=ON` and run `ctest --test-dir build --output-on-failure` for the numerical, statistical and server ABI checks. Local server/doc commands and performance limits are in the [verification record](docs/round3-verification.md). Platform verification workflows stage architecture-labeled extension artifacts; this source expansion has not been published.

## Buffer formats

- `CoupledMapLattice`: at least `numCells` samples.
- `MarkovChain`: one-channel, row-major `numStates * numStates` transition weights; optional one-channel values.
- `SemiMarkov`: the same matrix plus a three-channel duration buffer containing mean seconds, shape, and distribution code.
- `MultiGaussNoise`: one-channel, row-major covariance matrix; optional one-channel mean vector.
- `TDirichletRand`: mono positive concentration vector.
- `PolyaUrn`: mono nonnegative base weights with positive total; optional finite value vector.
- `MarkovModulatedTrig`: mono row-major transition weights plus event-rate and transition-opportunity-rate vectors.
- `Lorenz96`: optional mono finite initial vector; reload stores a template and a later reset applies it.

Buffer examples use a `Routine` and `s.sync` before Synth creation.

## Current limits

- Fractional generators are finite-band or finite-memory approximations.
- `FBrownianMotion` is control rate only.
- Lattice size is limited to 64, Markov states to 256, Gaussian dimension to 32, and fractional quality to 32.
- `MackeyGlass.maxDelaySeconds` is allocated at construction and bounded to 60 seconds.
- Changing buffer dimensions requires recreating the Synth.
- Heavy-tailed outputs may need explicit user-side limiting before audio use.

## License

This project is distributed under GPL-2.0-or-later for compatibility with SuperCollider's server-plugin interface.
