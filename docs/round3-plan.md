# Stochastica — Round 3 Agentic Implementation Plan

## Memory, hidden organization, stochastic geometry, anomalous transport, competing populations, and spatially extended chaos

**Target repository:** `https://github.com/fmiramar/Stochastica`

**Scope:** implement the third expansion round of Stochastica as eight new UGens, extend the general Stochastica Guide with a rigorous conceptual taxonomy of the whole library, and properly categorize the procedures/classes already present in the original Stochastica implementation. This round is an expansion and documentation-architecture pass, not a bug-fix round.

**New UGens:**

1. `MaternNoise`
2. `TDirichletRand`
3. `PolyaUrn`
4. `MarkovModulatedTrig`
5. `LevyFlight`
6. `JumpDiffusion`
7. `WilsonCowan`
8. `Lorenz96`

**Language:** C++17 server-plugin code, SuperCollider language classes, `.schelp` helpfiles, deterministic documentation/analysis scripts, and the existing Stochastica testing infrastructure.

---

# Part I — Non-negotiable project rules

## 1. Inspect the repository before editing

Before writing code, inspect the current repository rather than assuming that the layout described in an older implementation plan is still exact.

At minimum inspect:

- `Stochastica.cpp`;
- `CMakeLists.txt`;
- the existing `maps`, `flows`, `processes`, `distributions`, `state`, and `sonifiers` directories;
- `Classes`;
- `HelpSource/Classes`;
- `HelpSource/Guides/Stochastica.schelp`;
- shared RNG, trigger, range, distribution, buffer, matrix, and flow helpers;
- current tests and documentation-image scripts;
- the implementation of `OUProcess`, `TGammaRand`, `TStableRand`, `TPoissonRand`, `MarkovChain`, `SemiMarkov`, `MultiGaussNoise`, `CoupledMapLattice`, `Chua`, and `Izhikevich`;
- Round 2 classes if they have already been merged.

Reuse existing infrastructure when it already provides the required behavior correctly.

Do not create a second framework inside Stochastica.

Do not duplicate a sampler, RNG, buffer validator, rising-edge detector, small matrix routine, or integrator merely because writing a local copy is convenient.

If an existing implementation needs to expose an internal helper for reuse, a behavior-preserving extraction is allowed only when golden tests prove that existing UGen output sequences and semantics are unchanged.

---

## 2. This is not a bug-fix round

Do not include unrelated fixes to existing classes.

Do not:

- repair old SuperCollider/sc3-plugins stochastic UGens;
- alter equations of existing Stochastica classes;
- change seeded sequences of existing Stochastica stochastic UGens;
- rename existing arguments;
- change existing buffer layouts;
- change existing reset behavior;
- replace existing examples merely because a different example looks nicer;
- perform broad source reformatting;
- modernize unrelated CMake code;
- change old output scaling;
- silently alter existing documentation claims unrelated to categorization.

The documentation work explicitly requested in this round is allowed:

- improve the conceptual taxonomy in `HelpSource/Guides/Stochastica.schelp`;
- properly categorize original/v1 classes;
- add the new Round 3 classes to the Guide;
- update `categories::` metadata when an existing class is objectively in the wrong Stochastica documentation category;
- update top-level `related::` metadata when needed for the new conceptual cross-links.

Do not use the documentation task as an excuse for unrelated rewriting.

---

## 3. Preserve the mathematical/raw-first design

Stochastica must continue to separate a mathematical process from its sonic use.

Therefore:

- mathematical state is exposed directly;
- no hidden conversion of a state variable into pitch;
- no hidden conversion into amplitude;
- no arbitrary normalization simply to fit `[-1, 1]`;
- no hidden fold/wrap/clip unless that boundary behavior is the mathematical definition of the class;
- discontinuities that belong to the process remain visible;
- multivariate processes expose their mathematically meaningful state;
- sound-design examples perform mapping explicitly in SuperCollider code.

A user should be able to use the same process for pitch, timbre, spatialization, event control, GUI motion, or analysis without fighting a sonification decision embedded in C++.

---

## 4. Real-time safety

Every server calculation function must remain bounded and real-time safe.

Mandatory rules:

- no heap allocation in the calculation function;
- no file access;
- no exception propagation;
- no console output from the sample loop;
- no unbounded rejection sampling;
- no unbounded event catch-up loop;
- no matrix allocation in the sample loop;
- no buffer resize in the sample loop;
- no dynamic output-count change after construction;
- no general-purpose matrix exponential in the per-sample loop;
- no standard-library random engine with platform-dependent sequence behavior;
- no algorithm whose running time can grow without a documented fixed bound.

Any rejection sampler must have an explicit maximum attempt count and a finite deterministic fallback.

Any class whose complexity depends on `N` must have a documented `N` limit.

---

## 5. Trigger semantics

All `trig`, `reset`, and `reload` inputs follow the existing Stochastica rising-edge convention:

```text
current > 0 && previous <= 0
```

A held-positive trigger creates one edge only.

Unless a specific class section states otherwise:

1. `reset` has highest priority;
2. `reload` has second priority;
3. ordinary process/event updates happen after those controls.

Reset must be atomic.

---

## 6. Seed semantics

Use the existing Stochastica local PCG32 generator and current seed conversion convention.

For every new stochastic class:

- `seed = 0` derives the local seed from the parent Synth;
- nonzero seed values produce deterministic local sequences;
- reset restores the original local RNG state;
- after construction, a local seeded UGen must not consume the parent Synth RNG;
- seeded reference sequences must be tested;
- a behavior-preserving shared-helper extraction must not change the deterministic sequences of existing classes.

Deterministic flows such as `WilsonCowan` and `Lorenz96` do not need a seed.

---

## 7. Floating-point policy

- Store long-lived stochastic or dynamical state in `double` unless the repository has an established reason not to.
- Emit server outputs as `float`.
- Check complete state updates for finiteness.
- Validate logarithm, square-root, division, matrix-dimension, and normalization operations.
- Raw processes should not be silently bounded merely because they can become inconvenient for audio.
- If state becomes non-finite, use the class-specific recovery behavior in this document.
- Never allow a NaN to become a long-lived state that poisons the graph.

---

# Part II — Documentation rules

## 8. Documentation style is part of the implementation

The new UGens are not complete until their helpfiles explain both:

1. what the algorithm computes;
2. what kind of stochastic/dynamical organization it introduces conceptually.

The conceptual explanation is essential. Do not reduce a helpfile to equations plus argument descriptions.

Explain what the user gains that a simpler random generator does not provide.

---

## 9. Minimal examples — mandatory

The first examples on every class page must be extremely small.

Prefer one expression when one expression is enough.

Do not introduce multiple temporary variables in a demonstration whose only purpose is to hear one output.

Use larger examples only when the musical idea itself requires multiple interacting objects, buffers, states, voices, or mappings.

Large conceptual demonstrations may use many variables.

---

## 10. Runnable code must never be commented out

Any code intended to be run must be active.

Never place a runnable expression in a disabled/commented state.

Never require the reader to uncomment code.

Never place alternate executable examples inside comments.

---

## 11. Comments are allowed only inside a main `(...)` block

Do not put `// Example name` before a runnable block.

Use `.schelp` prose or subsection headings for titles and explanation.

A runnable block may contain brief explanatory comments only inside its outer `(...)` block.

Allowed pattern:

```supercollider
(
{
    var x;
    // Rare large increments create abrupt relocations.
    x = LevyFlight.kr(alpha: 1.3, scale: 0.4);
    SinOsc.ar(x.linexp(-8, 8, 80, 4000).clip(40, 12000), 0, 0.08)
}.play;
)
```

Do not put commented executable alternatives inside the block.

---

## 12. Buffer examples

Every runnable buffer example must:

1. allocate/fill the buffer inside a `Routine` or another synchronization-safe structure;
2. call `s.sync` before Synth creation;
3. free buffers when a large example creates persistent resources, when appropriate;
4. use only core SuperCollider plus Stochastica.

Do not rely on server timing accidents.

---

## 13. `See Also` placement — mandatory new rule

**Never place a `See Also` section at the lowest/bottom part of a helpfile.**

Stochastica should normally use SuperCollider's top metadata:

```text
related:: Guides/Stochastica, Classes/OUProcess, Classes/FBrownianMotion
```

This belongs at the top of the document with `class::`, `summary::`, and `categories::`.

If there is a strong reason to use a literal prose section named `See Also`, it must appear near the top of the page, after the initial conceptual description and before class methods/examples.

Forbidden structure:

```text
examples::
...
section:: See Also
...
```

Do not append `See Also` after examples.

When touching an existing helpfile for category metadata in this round, do not move its related links to the bottom.

This rule applies to:

- all eight new class helpfiles;
- any new Guide page;
- any existing page edited as part of the categorization pass.

---

# Part III — Why Round 3 exists

## 14. The three-round conceptual progression

The Guide must explain that Stochastica is evolving through increasingly structured forms of randomness and nonlinear organization.

A useful conceptual summary is:

| Round | Main question |
|---|---|
| **Round 1 / original Stochastica** | What kinds of randomness and nonlinear dynamics can we generate? |
| **Round 2** | How can stochastic agents interact, inhibit, synchronize, switch, and form metastable behavior? |
| **Round 3** | **How can randomness acquire memory, hidden organization, geometry, transport, and spatial structure?** |

Do not present these merely as release-history trivia. Use the progression to explain why different mathematical processes matter musically.

---

## 15. Round 3 conceptual goal

Round 3 should deliberately avoid being “eight more equations.”

It adds **memory, latent organization, stochastic geometry, anomalous movement, hybrid continuous/discontinuous behavior, interacting populations, and high-dimensional spatial structure**.

The Round 3 batch is:

| UGen | New conceptual territory | Why it belongs in Round 3 |
|---|---|---|
| `MaternNoise` | smoothness + correlation scale | Controls how rough a stochastic trajectory is independently from how quickly it changes |
| `TDirichletRand` | simplex / allocation | Generates random positive weight vectors whose components sum to one |
| `PolyaUrn` | reinforcement / path dependence | Past choices alter future probabilities |
| `MarkovModulatedTrig` | latent regimes | A hidden Markov state changes event density through time |
| `LevyFlight` | anomalous transport | Mostly ordinary motion with rare very large relocations |
| `JumpDiffusion` | hybrid continuous/discontinuous motion | Continuous stochastic wandering is punctuated by explicit random shocks |
| `WilsonCowan` | excitation/inhibition at population scale | Competing populations generate balance, oscillation, suppression, and transitions |
| `Lorenz96` | high-dimensional spatiotemporal chaos | Local interactions around a ring create waves, turbulence, and collective chaotic structure |

The Guide should communicate this idea in substance:

> Round 3 expands Stochastica from stochastic trajectories and interacting event systems into **structured randomness with memory and hidden organization**. It introduces smooth correlated fields, probabilistic allocation, self-reinforcement, latent regimes, anomalous transport, hybrid jump-diffusion dynamics, excitatory/inhibitory population behavior, and spatially extended chaos.

---

## 16. Why these eight belong together

The final Guide should make the conceptual map explicit:

| Concept | UGen |
|---|---|
| smooth stochastic geometry | `MaternNoise` |
| allocation of a whole | `TDirichletRand` |
| reinforcement / historical memory | `PolyaUrn` |
| hidden behavioral regimes | `MarkovModulatedTrig` |
| anomalous long-range movement | `LevyFlight` |
| smooth motion + discontinuous shocks | `JumpDiffusion` |
| competing populations | `WilsonCowan` |
| distributed spatial chaos | `Lorenz96` |

This is the principal narrative of Round 3.

---

# Part IV — Rebuild the conceptual taxonomy in the Stochastica Guide

## 17. Do not confuse source folders with concepts

The current technical categories such as `maps`, `flows`, `processes`, `distributions`, and `state` are useful implementation categories.

They are not sufficient as a conceptual explanation.

The Guide must contain both:

1. **mathematical/implementation categories** — what kind of mathematical object the class is;
2. **behavioral/conceptual categories** — what kind of organization the class introduces.

A reader should be able to answer both:

> Is this a distribution, map, flow, point process, or state model?

and:

> Does this produce mean reversion, clustering, reinforcement, long memory, hidden regimes, anomalous jumps, synchronization, or spatial coupling?

---

## 18. Properly categorize the original/v1 Stochastica classes

Audit the complete original Stochastica class inventory and explain it in `HelpSource/Guides/Stochastica.schelp`.

Do not merely list names.

The Guide should categorize the original classes approximately as follows, adjusted only if the current repository has changed.

### 18.1 Discrete dynamical systems

#### Independent/raw two-dimensional maps

- `HenonMap`
- `GbmanMap`
- `LatoocarfianMap`
- `IkedaMap`
- `LoziMap`
- `TinkerbellMap`
- `CliffordMap`
- `DeJongMap`
- `CircleMap`

Conceptual explanation:

These are deterministic recurrence systems. They do not generate randomness internally; unpredictability comes from nonlinear sensitivity and complicated iteration. They are useful as irregular sequencers, coordinate trajectories, modulation sources, and raw nonlinear structures. The Guide must explicitly distinguish **deterministic chaos** from stochastic randomness.

#### Coupled and spatial discrete systems

- `CoupledLogisticMap`
- `CoupledMapLattice`

Conceptual explanation:

Coupling introduces interaction between recurrence states. `CoupledMapLattice` additionally introduces a spatial dimension: local states evolve while exchanging information with neighbors. These classes are early examples in Stochastica of **emergent structure created by interaction rather than by one isolated generator**.

#### Excitable discrete dynamics

- `RulkovMap`

Conceptual explanation:

`RulkovMap` is not merely another strange attractor. It models excitable neuron-like dynamics and can produce quiescence, spikes, and bursting. Categorize it conceptually with threshold/excitability behavior while retaining its technical location among discrete maps.

### 18.2 Continuous deterministic systems

#### Delay dynamics

- `MackeyGlass`

Conceptual explanation:

The future state depends on delayed history, not only the instantaneous state. This introduces **explicit temporal memory** into a deterministic continuous system and can generate complex or chaotic trajectories from a compact equation.

#### Circuit chaos

- `Chua`

Conceptual explanation:

`Chua` represents a nonlinear circuit flow. It demonstrates how continuous feedback and nonlinearity can create structured chaos in a low-dimensional physical model.

#### Excitable/spiking dynamics

- `Izhikevich`

Conceptual explanation:

`Izhikevich` represents neuron-like voltage/recovery dynamics. It should be explained as an **excitable dynamical system** whose threshold/reset behavior produces spikes rather than as a generic chaotic oscillator.

### 18.3 Continuous stochastic trajectories

#### Mean-reverting diffusion

- `OUProcess`

Conceptual explanation:

`OUProcess` adds a restoring tendency to random diffusion. It wanders, but the farther it moves from its mean, the stronger the drift back toward the center. This introduces **stochastic motion with memory and equilibrium**.

Do not describe it as mathematically bounded. It is unbounded but mean reverting.

#### Long-memory and scale-dependent stochastic processes

- `FractionalNoise`
- `FBrownianMotion`

Conceptual explanation:

These classes introduce temporal correlation extending beyond one-step memory. Their output can exhibit persistent or anti-persistent structure and scale-dependent roughness. They should be connected conceptually to Round 3 `MaternNoise`, which instead controls a finite correlation scale and smoothness explicitly.

#### Bounded stochastic motion

- `BoundedWalk`

Conceptual explanation:

`BoundedWalk` demonstrates that boundaries are part of a stochastic model, not merely output limiting. Reflection, wrapping, clipping, resampling, and absorption create different processes and therefore different long-term behavior.

### 18.4 Event and point processes

#### Renewal timing

- `RenewalTrig`

Conceptual explanation:

A renewal process is organized by independently sampled inter-event intervals. Different interval distributions alter regularity, clustering, refractory behavior, and tail behavior without requiring a metrical grid.

#### Self-excitation

- `HawkesTrig`

Conceptual explanation:

Past events temporarily increase future event intensity. This introduces **positive feedback in event time**, producing cascades, bursts, and clustering.

This class becomes the conceptual starting point for Round 2 interaction/inhibition classes and Round 3 latent-regime event processes.

### 18.5 Triggered probability distributions

The Guide must explain that these are **memoryless samplers**: the distribution shapes independent draws, but the class itself does not carry evolving stochastic state between triggers.

Subcategorize them conceptually.

#### Positive continuous distributions

- `TGammaRand`
- `TWeibullRand`
- `TLogNormalRand`

Explain uses such as positive durations, amplitudes, scales, multiplicative variation, and skewed physical quantities.

#### Count and waiting distributions

- `TPoissonRand`
- `TGeometricRand`
- `TNegBinomialRand`

Explain that these generate integer-valued counts or waiting/failure quantities and should not be presented as merely unusual continuous random curves.

#### Heavy-tailed distributions

- `TCauchyRand`
- `TStableRand`
- `TLevyRand`

Explain that heavy tails make extreme events materially more likely than Gaussian sampling. Connect this to Round 3 `LevyFlight`, where heavy-tailed draws become a process through accumulation over time.

#### Conditioned/bounded distribution

- `TTruncNormalRand`

Explain the difference between sampling a distribution conditioned to an interval and clipping a Gaussian after sampling.

### 18.6 Discrete state, duration, and multivariate structure

#### Markov state transition

- `MarkovChain`

Conceptual explanation:

The next state depends on the current state through a transition matrix. This introduces state-dependent local memory but not accumulated historical preference.

#### State transition with duration memory

- `SemiMarkov`

Conceptual explanation:

`SemiMarkov` separates **where the process goes next** from **how long it remains in the current state**. This introduces explicit dwell-time structure absent from a basic Markov chain.

#### Correlated random vectors

- `MultiGaussNoise`

Conceptual explanation:

Randomness can be multivariate. Multiple outputs can vary together according to covariance without being identical. Explain this as **instantaneous correlation across dimensions**, and contrast it with Round 2 `MultiOUProcess`, which adds temporal memory to correlated motion.

### 18.7 Explicit sonification tools

- `HenonSonify`
- `GbmanSonify`
- `LatoocarfianSonify`

Conceptual explanation:

These are deliberately not mathematical raw-map classes. They are rendering/sonification strategies that map dynamical state into frequency and level. Preserve this distinction prominently.

### 18.8 Visualization utility

- `SDPhaseView`

Explain that this is a language-side visualization/sonification utility, not a stochastic model or server-side mathematical process.

---

## 19. Add Round 2 concepts to the same conceptual taxonomy when present

If the Round 2 implementation exists in the target branch, include it in the same Guide taxonomy rather than creating a disconnected “Round 2” appendix.

Explain:

- `MultiHawkesTrig` — interaction / contagion among event streams;
- `SelfCorrectingTrig` — inhibition / anti-clustering;
- `RandomTelegraph` — persistent stochastic regime switching;
- `ShotNoise` — stochastic impulses accumulated into a continuous signal;
- `MultiOUProcess` — temporally correlated multivariate motion;
- `DoubleWellProcess` — metastability and stochastic resonance;
- `TVonMisesRand` — circular probability;
- `KuramotoNetwork` — emergent synchronization.

If these classes are absent, do not create broken links. Record the missing dependency in the implementation report and restrict the Guide to classes actually present.

---

## 20. Add a “Forms of organization” section to the Guide

The Guide should contain a concise conceptual table that allows users to choose a process by behavior rather than by class name.

A good target structure is:

| Desired behavior | Representative classes |
|---|---|
| independent shaped randomness | triggered distribution classes |
| deterministic nonlinear iteration | raw map classes |
| continuous deterministic feedback | `Chua`, `MackeyGlass`, `WilsonCowan` |
| mean-reverting stochastic drift | `OUProcess`, `MultiOUProcess` |
| controlled smooth random trajectories | `MaternNoise` |
| long-memory / scale-free correlation | `FractionalNoise`, `FBrownianMotion` |
| stochastic event spacing | `RenewalTrig` |
| self-exciting bursts | `HawkesTrig` |
| interacting contagion | `MultiHawkesTrig` |
| inhibitory / anti-clustered events | `SelfCorrectingTrig` |
| persistent discrete switching | `RandomTelegraph` |
| hidden event-density regimes | `MarkovModulatedTrig` |
| path-dependent reinforcement | `PolyaUrn` |
| random allocation summing to one | `TDirichletRand` |
| anomalous long-range motion | `LevyFlight` |
| diffusion with explicit shocks | `JumpDiffusion` |
| metastable switching | `DoubleWellProcess` |
| circular randomness | `TVonMisesRand` |
| collective phase synchronization | `KuramotoNetwork` |
| excitatory/inhibitory population dynamics | `WilsonCowan` |
| spatially extended deterministic chaos | `CoupledMapLattice`, `Lorenz96` |

Do not let this table replace prose. It is a navigation aid.

---

# Part V — Shared implementation work

## 21. Reuse existing distribution primitives

Round 3 intentionally combines several algorithms already present at the scalar level.

Before adding new helpers, inspect whether Stochastica already has reusable internal implementations for:

- open-uniform random values;
- Gaussian sampling;
- bounded Gamma sampling;
- Poisson sampling;
- stable-distribution sampling;
- categorical sampling from nonnegative weights;
- buffer copying/validation;
- Cholesky or tiny matrix operations;
- RK4 integration;
- finite-state reset logic.

Preferred architecture:

- `TDirichletRand` reuses the same bounded Gamma primitive as `TGammaRand`;
- `LevyFlight` reuses the stable sampler used by `TStableRand`;
- `JumpDiffusion` reuses Gaussian and Poisson primitives;
- `PolyaUrn` and `MarkovModulatedTrig` reuse categorical-buffer logic from the Markov classes;
- `WilsonCowan` and `Lorenz96` reuse existing deterministic-flow conventions.

If helpers are currently private to one `.cpp` file, extract them only with regression tests proving that existing sequences remain unchanged.

---

## 22. Do not create generic abstractions prematurely

Do not introduce a large template framework for “all SDEs” or “all multivariate models” in this round.

Shared utilities should be small and driven by at least two concrete users.

A good helper removes duplicate correctness-sensitive code.

A bad helper obscures a short equation behind generic machinery.

---

# Part VI — `MaternNoise`

## 23. Concept

`MaternNoise` generates a stationary Gaussian stochastic trajectory whose **correlation time** and **smoothness** can be controlled separately.

This is the key conceptual distinction.

Many noise generators mainly answer:

> How much low-frequency versus high-frequency energy is present?

Matérn processes additionally answer:

> Over what time scale are values related, and how differentiable/smooth should a realization look?

This makes `MaternNoise` particularly useful for natural parameter trajectories, gestural modulation, slow control drift, animation coordinates, spatial motion, and correlated timbral change.

The Guide should compare:

```text
OUProcess          finite correlation scale, rough sample paths
FractionalNoise    scale-dependent / approximately power-law correlation
FBrownianMotion    long-memory nonstationary motion approximation
MaternNoise        explicit finite correlation scale + selectable smoothness
```

---

## 24. Mathematical family

Use the one-dimensional Matérn covariance convention:

```text
k(r) = variance * 2^(1-nu) / Gamma(nu)
       * (sqrt(2*nu) * r / ell)^nu
       * K_nu(sqrt(2*nu) * r / ell)
```

where:

- `variance` is the stationary output variance;
- `ell` is `correlationTime` in model-time units;
- `nu` controls smoothness.

Implement only half-integer cases with compact Markov state representations:

```text
smoothness = 0    nu = 1/2
smoothness = 1    nu = 3/2
smoothness = 2    nu = 5/2
```

Use:

```text
lambda = sqrt(2 * nu) / ell
```

For `nu = 1/2`, the system is an OU process.

For `nu = 3/2`, use the second-order linear SDE corresponding to transfer denominator `(s + lambda)^2`.

For `nu = 5/2`, use the third-order linear SDE corresponding to `(s + lambda)^3`.

Choose continuous-time noise amplitude so that stationary output variance equals the user-provided `variance`.

Reference amplitudes for centered output `y = x - mean`:

```text
nu = 1/2: q^2 = 2 * lambda * variance
nu = 3/2: q^2 = 4 * lambda^3 * variance
nu = 5/2: q^2 = (16/3) * lambda^5 * variance
```

Verify these analytically and statistically rather than copying them blindly.

---

## 25. Proposed API

```supercollider
MaternNoise.ar(
    mean = 0,
    variance = 1,
    correlationTime = 1,
    smoothness = 1,
    timeScale = 1,
    initial = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr` with identical arguments.

### `smoothness`

Init-rate integer selector:

```text
0 -> nu = 1/2
1 -> nu = 3/2
2 -> nu = 5/2
```

Clamp invalid values to the nearest supported mode at construction.

Do not change state dimension while the Synth is running.

### `correlationTime`

Must be finite and strictly positive.

Use a documented small positive floor for invalid/nonpositive values.

### `variance`

Negative values become zero.

`variance = 0` produces deterministic relaxation toward the mean.

### Parameter sampling

Because exact linear-SDE discretization coefficients depend on `variance`, `correlationTime`, and `timeScale`, sample these parameters once per server calculation block for `.ar`, then use one coefficient set across that block.

Document that coefficient modulation is block-rate even when output is audio-rate.

Do not perform a matrix exponential per sample.

---

## 26. Exact finite-step implementation

Use an exact discrete transition for each supported linear SDE.

For each mode, compute:

```text
state[n+1] = A(dt) * state[n] + L(dt) * z
```

where `z` is a vector of independent unit Gaussians and `L * L^T` equals the finite-step process-noise covariance.

Requirements:

- no Euler-Maruyama approximation for the final implementation;
- no per-sample dynamic allocation;
- dimensions are only 1, 2, or 3;
- derive closed-form transition/covariance expressions or use a fixed-size analytic routine recomputed once per block;
- factor the tiny covariance with fixed-size code;
- protect against nearly zero `dt` and nearly zero variance;
- if coefficient construction becomes non-finite, hold/reset safely rather than emitting NaN.

A generic heap-allocating matrix library is not acceptable in the server calculation path.

---

## 27. Reset semantics

Reset must:

- restore the local RNG;
- set output position to `initial`;
- set hidden derivative/filter states to zero relative to the requested initial output;
- output `initial` on the reset sample.

The process then relaxes toward its stationary regime naturally.

Documentation/statistical tests that measure stationary covariance must include a burn-in period.

---

## 28. Required tests

Test:

- deterministic reset/seed behavior;
- empirical mean approximately equals `mean` after burn-in;
- empirical variance approximately equals requested `variance`;
- `nu = 1/2` autocovariance matches exponential decay;
- `nu = 3/2` covariance approximately follows `(1 + lambda*r) exp(-lambda*r)`;
- `nu = 5/2` covariance approximately follows `(1 + lambda*r + lambda^2*r^2/3) exp(-lambda*r)`;
- larger `correlationTime` increases correlation duration without changing target stationary variance;
- higher smoothness reduces high-frequency roughness;
- `variance = 0` produces deterministic behavior;
- `.ar` and `.kr` use elapsed time consistently.

---

## 29. Helpfile examples

The first example should be tiny.

```supercollider
(
{ SinOsc.ar(MaternNoise.kr(600, 90000, 0.5, 1).clip(80, 4000), 0, 0.08) }.play
)
```

The helpfile must include a conceptual comparison in which three processes use the same `mean`, `variance`, and `correlationTime` but different smoothness modes.

A larger example may route the three modes to three related timbral voices so that the listener hears rough, moderately smooth, and very smooth random motion.

Do not bury the conceptual difference under an elaborate synth.

---

## 30. Documentation visualization

Generate a deterministic documentation image containing:

1. three equal-duration time series for `nu = 1/2`, `3/2`, and `5/2`;
2. corresponding normalized autocorrelation curves;
3. identical variance and correlation-time settings.

The image should demonstrate smoothness, not decorative plotting.

---

# Part VII — `TDirichletRand`

## 31. Concept

`TDirichletRand` introduces randomness on a **simplex**.

Ordinary random generators choose a number.

A Dirichlet draw divides a whole into multiple nonnegative parts:

```text
x_i >= 0
sum_i x_i = 1
```

Conceptually:

> Randomness can mean allocating a finite resource, not merely choosing a scalar value.

This is directly useful for:

- mixing weights;
- probabilities;
- spectral energy allocation;
- density allocation among rhythmic layers;
- interpolation between materials;
- speaker-energy distribution;
- weighted algorithm selection.

---

## 32. Distribution

For positive concentration parameters:

```text
alpha = [alpha_0, ..., alpha_N-1]
```

sample independently:

```text
g_i ~ Gamma(alpha_i, 1)
```

then return:

```text
x_i = g_i / sum_j g_j
```

This construction must reuse the project's bounded Gamma sampler.

Do not create an unbounded Gamma rejection loop.

---

## 33. Proposed API

```supercollider
TDirichletRand.kr(
    numChannels = 4,
    alphaBuf = 0,
    trig = 1,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Provide `.ar` if the repository's triggered-distribution conventions support audio-rate triggers cleanly.

Return exactly `numChannels` outputs.

Set a first-version limit:

```text
2 <= numChannels <= 32
```

---

## 34. Buffer format

`alphaBuf`:

- one channel;
- at least `numChannels` samples;
- every concentration parameter must be finite and strictly positive.

Copy parameters into RT-owned memory at construction/reload.

Do not read the user buffer repeatedly during every trigger.

If any alpha is invalid:

- mark the parameter set invalid;
- output a uniform vector on triggers;
- do not enter the Gamma sampler with invalid shape;
- issue at most one non-RT warning if the repository supports safe warning behavior.

Do not silently reinterpret negative alpha values as positive.

---

## 35. Reload/reset

### Reload

On a reload edge:

- validate and copy the alpha vector;
- preserve RNG position;
- preserve the current output until the next trigger.

### Reset

Reset:

- restores the original RNG state;
- outputs the normalized concentration mean `alpha_i / sum(alpha)` when parameters are valid;
- outputs uniform weights when invalid;
- does not consume a random draw on the reset sample.

---

## 36. Required tests

- every output is finite and nonnegative;
- output sum is one within floating tolerance;
- empirical mean approaches `alpha_i / sum(alpha)`;
- symmetric large alpha concentrates near equal allocation;
- symmetric alpha below one favors corners/sparse allocations;
- seeded reset reproduces output sequence;
- buffer reload changes future distribution without buffer mutation;
- invalid alpha cannot crash or loop indefinitely;
- audio-rate multiple trigger edges produce independent vectors if `.ar` is provided.

---

## 37. Creative documentation examples

### Fixed total energy

Use a four-dimensional Dirichlet vector to distribute one fixed amplitude budget among four oscillators.

The example should make the invariant perceptually obvious: total gain remains constrained while internal allocation changes.

### Sparse versus balanced mixtures

Use the same four voices with two alpha buffers:

```text
[0.2, 0.2, 0.2, 0.2]   -> sparse/extreme allocation
[8, 8, 8, 8]           -> balanced allocation
```

Keep synthesis simple so the geometry of the distribution remains audible.

---

## 38. Documentation visualization

For `N = 3`, generate a triangular simplex plot for several concentration settings.

Show at least:

- `[1, 1, 1]`;
- `[0.2, 0.2, 0.2]`;
- `[8, 8, 8]`;
- one asymmetric vector such as `[6, 1, 1]`.

This is one of the rare cases where a simplex/ternary plot is mathematically more informative than a histogram.

---

# Part VIII — `PolyaUrn`

## 39. Concept

`PolyaUrn` introduces **reinforcement and path dependence**.

Three distinctions must be explicit in the Guide:

```text
independent sampling
    past choices do not affect future probabilities

MarkovChain
    future depends on the current state

PolyaUrn
    future probabilities depend on accumulated choice history
```

A selected category receives additional weight and therefore becomes more likely to be selected again.

Small early accidents can become long-term preferences.

Different seeded runs can develop different dominant categories even when they begin from identical initial weights.

This is useful for generative systems that should develop habits, preferences, stylistic biases, or self-reinforcing material during a performance.

---

## 40. Mathematical model

Let initial positive weights be:

```text
w_i(0) > 0
```

At every trigger:

1. choose state `i` with probability `w_i / sum(w)`;
2. output the choice;
3. apply nonnegative reinforcement:

```text
w_i <- w_i + reinforcement
```

The first version should implement classic additive reinforcement only.

Do not add decay, inhibition, replacement matrices, or generalized urn dynamics yet.

Those can form a future extension.

---

## 41. Proposed API

```supercollider
PolyaUrn.kr(
    numStates = 4,
    weightBuf = 0,
    trig = 1,
    reinforcement = 1,
    valueBuf = -1,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Set:

```text
2 <= numStates <= 64
```

### Outputs

Return four outputs:

```text
stateIndex
stateValue
selectedProbability
normalizedEntropy
```

`stateValue` equals the mapped value from `valueBuf` when supplied and valid, otherwise it equals `stateIndex`.

`selectedProbability` is the probability of the chosen state **before reinforcement**.

`normalizedEntropy` is:

```text
H = -sum(p_i * log(p_i)) / log(numStates)
```

with zero-probability terms contributing zero.

This output makes the gradual transition from diversity toward dominance musically accessible without emitting one output per internal weight.

---

## 42. Buffers

### `weightBuf`

- one channel;
- at least `numStates` samples;
- finite nonnegative values;
- total initial weight must be positive.

Zero individual weights are allowed and mean the state cannot initially be selected.

### `valueBuf`

Optional one-channel buffer with at least `numStates` values.

Never rewrite either user buffer.

Store initial/base weights and current weights separately in RT-owned arrays.

---

## 43. Numerical stability

The internal sum of weights can grow indefinitely in a long-lived Synth.

Do not wait for floating overflow.

When total weight exceeds a documented large threshold, rescale all current weights by the same positive factor.

Because categorical probabilities depend only on ratios, proportional rescaling must leave the process mathematically unchanged apart from floating roundoff.

Test that rescaling does not create a discontinuous probability change.

---

## 44. Reload/reset

### Reload

A reload edge:

- validates and copies new base weights/value mapping;
- sets current weights to the newly loaded base weights;
- preserves RNG position;
- recalculates entropy;
- does not make a selection on the reload sample.

### Reset

Reset:

- restores current weights from the currently loaded base weights;
- restores the original RNG seed;
- restores output state to state 0 or the repository's documented initial-state convention;
- recalculates probability/entropy from base weights;
- performs no random selection on the reset sample.

---

## 45. Required tests

- equal initial weights and positive reinforcement remain finite;
- chosen states increase their subsequent probability;
- `reinforcement = 0` reduces to repeated independent categorical sampling from fixed weights;
- symmetric urns preserve the expected ensemble mean proportion even though single runs may diverge;
- normalized entropy remains within `[0, 1]` within tolerance;
- rescaling preserves normalized probabilities;
- reset reproduces a seeded run;
- reload replaces the learned state with new base weights without rewriting the user buffer;
- invalid all-zero weights fail safely.

---

## 46. Creative documentation examples

### A system that learns a preference

Use four sound materials or four oscillator registers.

Each trigger selects one through `PolyaUrn`.

As one category happens to be selected more often, reinforcement makes it increasingly dominant.

Map `normalizedEntropy` to another synthesis parameter:

- high entropy -> wide spectrum or wide stereo field;
- low entropy -> narrow spectrum or centralized image.

This makes **the loss of diversity itself** audible.

### Compare with `MarkovChain`

Keep the example concise and explain conceptually:

- Markov transition depends on current state;
- urn reinforcement depends on accumulated history.

Do not build an overcomplicated A/B framework.

---

## 47. Documentation visualization

Plot over draw number:

- normalized proportions of each category;
- normalized entropy.

Generate at least three seeded runs from the same symmetric initial weights to demonstrate path dependence.

---

# Part IX — `MarkovModulatedTrig`

## 48. Concept

`MarkovModulatedTrig` introduces **latent stochastic regimes**.

The event stream itself does not have one fixed rate.

A hidden state evolves through a Markov model, and each hidden state has its own event intensity.

Conceptually:

```text
state 0 -> sparse
state 1 -> active
state 2 -> dense
state 3 -> burst-like
```

The listener may hear the event density change without directly hearing the state variable.

This differs from `RandomTelegraph`:

```text
RandomTelegraph
    the switching state is itself the main stochastic signal

MarkovModulatedTrig
    a hidden switching state changes the statistics of another observable event process
```

It also differs from `SemiMarkov`: the principal audible output here is an event process modulated by latent state rather than the state sequence itself.

---

## 49. Model

Implement a finite-state continuous-time Markov modulation with state-specific exponential exit rates.

For current state `i`:

- observed event process rate is `eventRate[i]`;
- latent state transition opportunity rate is `stateRate[i]`;
- when a latent transition occurs, choose the next state from row `i` of the transition matrix.

At each audio sample/control calculation, compute event and state-transition probabilities from the state that existed at the **start** of the calculation.

Then draw both decisions before applying a new state.

This prevents implementation order from making a newly selected state retroactively change the event probability of the same calculation.

The new state affects event intensity beginning on the next calculation.

---

## 50. Proposed API

```supercollider
MarkovModulatedTrig.ar(
    numStates = 3,
    transitionBuf = 0,
    eventRateBuf = 1,
    stateRateBuf = 2,
    initialState = 0,
    maxEventRate = 20000,
    maxStateRate = 20000,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr` with the same arguments.

Set:

```text
2 <= numStates <= 64
```

### Outputs

Return:

```text
eventTrig
stateIndex
intensity
stateChangeTrig
```

`intensity` is the pre-event `eventRate` of the current latent state.

---

## 51. Buffer formats

### `transitionBuf`

One channel, row-major `numStates * numStates` nonnegative weights.

Rows need not be normalized.

If a row sums to zero, a latent transition opportunity leaves the process in the current state.

### `eventRateBuf`

One channel, at least `numStates` samples.

Negative/nonfinite rates become zero during loading.

### `stateRateBuf`

One channel, at least `numStates` samples.

These are exponential exit/opportunity rates per second.

Negative/nonfinite rates become zero.

Copy all buffer data into RT-owned storage on construction/reload.

---

## 52. Runtime behavior

For elapsed real time `dt`:

```text
pEvent = 1 - exp(-min(eventRate[state], maxEventRate) * dt)
pState = 1 - exp(-min(stateRate[state], maxStateRate) * dt)
```

Use the repository's numerically stable probability helper if available.

Draw event and state-change decisions independently from the old state.

When a state change is scheduled:

- sample the transition row;
- update state after the event decision;
- `stateChangeTrig = 1` only if the resulting state differs from the previous state.

At `.kr`, document that at most one observed event and one state-transition opportunity can be represented per control block.

For accurate high rates, recommend `.ar`.

---

## 53. Reset/reload

Reset:

- restores `initialState`;
- restores the original RNG state;
- outputs no event or state-change trigger on the reset sample;
- outputs the initial state's intensity.

Reload:

- copies/validates all buffers;
- preserves current state when it remains valid;
- clamps current state to valid range only if dimensions/configuration require it;
- preserves RNG position;
- emits no event caused solely by reload.

---

## 54. Required tests

- with zero state rates, latent state remains fixed and events match a fixed-rate Poisson/Bernoulli process;
- conditional event rate depends on latent state;
- empirical latent-state transition proportions match row weights;
- empirical latent dwell times match state-specific exponential rates at audio rate;
- old-state semantics are independent of code loop ordering;
- state changes do not retroactively change same-sample event intensity;
- reset reproduces deterministic seeded behavior;
- malformed buffers fail safely;
- event and state rates respect ceilings.

---

## 55. Creative documentation example

Create a minimal three-regime trigger process:

```text
quiet -> active -> dense
```

Use the event trigger to excite the same short sound in every regime.

Do not directly map `stateIndex` to pitch in the first conceptual example. The point is that the hidden state changes the **statistics** of the observed event stream.

A larger example may display/poll the hidden state while the listener hears only event density.

---

## 56. Documentation visualization

Create one aligned figure containing:

1. latent state timeline;
2. event-rate/intensity step function;
3. event raster.

This should make the idea of a hidden regime visually immediate.

---

# Part X — `LevyFlight`

## 57. Concept

`LevyFlight` turns heavy-tailed random increments into a trajectory.

Most movement can be modest, but rare increments can be extremely large.

The defining compositional idea is **anomalous transport**:

> A process may explore by many local moves and occasional long-range relocations rather than by gradual Gaussian diffusion.

The Guide must connect it to existing classes:

```text
TStableRand
    independent heavy-tailed draws

LevyFlight
    accumulated heavy-tailed increments forming a trajectory

OUProcess
    diffusive motion with mean reversion

FBrownianMotion
    correlated/self-similar motion without the same stable-jump mechanism
```

---

## 58. Model

Implement a symmetric alpha-stable Lévy motion approximation using exact independent stable increments over each calculation interval.

Restrict the first version to **symmetric** stable increments (`beta = 0`).

This avoids parameterization ambiguities in skewed stable scaling and keeps the mathematical meaning clean.

For elapsed model time `dt`:

```text
DeltaX = drift * dt + scale * dt^(1/alpha) * S_alpha
x <- x + DeltaX
```

where `S_alpha` is a standardized symmetric alpha-stable draw in the same convention used by Stochastica's stable sampler.

Supported:

```text
0 < alpha <= 2
```

Use the same practical alpha floor as the existing stable sampler if it already defines one.

`alpha = 2` should reduce to the corresponding Gaussian stable limit under the chosen parameterization.

---

## 59. Proposed API

```supercollider
LevyFlight.ar(
    alpha = 1.5,
    scale = 1,
    drift = 0,
    timeScale = 1,
    initial = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

Return two outputs:

```text
state
increment
```

The second output exposes rare large relocations directly for compositional use and analysis.

---

## 60. Runtime rules

- `scale < 0` uses its magnitude or follows the existing stable-scale convention consistently;
- `timeScale <= 0` holds the state and outputs zero increment;
- use `double` for accumulated state;
- if one increment is finite but enormous, preserve it: heavy tails are the point;
- if state becomes non-finite, restore `initial`, emit zero increment on recovery, and restore/retain RNG according to the project's established non-finite recovery convention;
- do not clip large finite values internally.

---

## 61. Required tests

- reset reproduces deterministic seeded increments;
- increments are independent under fixed parameters;
- empirical increment scale follows approximately `dt^(1/alpha)`;
- `alpha = 2` matches the expected Gaussian stable special case;
- decreasing alpha increases tail extremity;
- drift contributes linearly with elapsed model time;
- `.ar` and `.kr` have consistent finite-time scaling;
- no hidden clipping occurs.

For statistical tests, use robust tail/quantile metrics rather than sample variance when variance is mathematically infinite.

---

## 62. Creative documentation examples

### Rare register relocation

Use `state` to control oscillator register after an explicit safe user-side mapping/clip.

The listener should hear extended local activity interrupted by rare radical relocations.

### Compare transport types

A larger example may compare:

- `OUProcess`;
- `FBrownianMotion`;
- `LevyFlight`.

Use the same simple synthesis voice and avoid additional random modulation that obscures the difference.

---

## 63. Documentation visualization

Plot:

- a long time series;
- increments below it;
- a log-scaled increment-magnitude distribution or complementary CDF.

Large jumps should remain visible rather than being cropped out of the figure.

---

# Part XI — `JumpDiffusion`

## 64. Concept

`JumpDiffusion` combines two qualitatively different mechanisms:

1. continuous Gaussian diffusion;
2. discrete stochastic shocks.

The trajectory wanders continuously most of the time, but at random event times its state makes an explicit jump.

This is not the same as `ShotNoise`:

```text
ShotNoise
    random events add transient impulse energy to a signal

JumpDiffusion
    random events permanently displace the evolving process state
```

It is also not the same as `LevyFlight`:

```text
LevyFlight
    one heavy-tailed stable increment process

JumpDiffusion
    explicitly separates continuous diffusion from a finite-rate jump mechanism
```

This separation is musically useful because the continuous and discontinuous parts can be controlled independently.

---

## 65. Model

Implement a compound-Poisson Gaussian jump diffusion:

```text
dX = drift * dt + diffusion * dW + J * dN
```

with:

- Poisson jump count over each finite interval;
- independent normally distributed individual jump sizes with mean `jumpMean` and standard deviation `jumpStd`.

For an interval with:

```text
K ~ Poisson(jumpRate * dt)
```

the sum of `K` independent Gaussian jumps is exactly:

```text
jumpSum ~ Normal(K * jumpMean, sqrt(K) * jumpStd)
```

This avoids looping once per jump and supports multiple jumps per control block efficiently.

Then:

```text
continuousIncrement = drift * dt + diffusion * sqrt(dt) * Z1
jumpIncrement       = K * jumpMean + sqrt(K) * jumpStd * Z2
state              += continuousIncrement + jumpIncrement
```

with `jumpIncrement = 0` for `K = 0`.

---

## 66. Proposed API

```supercollider
JumpDiffusion.ar(
    drift = 0,
    diffusion = 1,
    jumpRate = 0.5,
    jumpMean = 0,
    jumpStd = 3,
    timeScale = 1,
    initial = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

Return:

```text
state
jumpTrig
jumpSize
jumpCount
```

`jumpTrig` is one when `jumpCount > 0` for that calculation.

`jumpSize` is the total compound jump added during the calculation, not the continuous diffusion increment.

---

## 67. Sampling

Reuse the existing bounded Poisson sampler and Gaussian sampler.

Do not loop `jumpCount` times.

If the Poisson sampler hits its bounded fallback, use its documented fallback consistently.

Rates and diffusion magnitudes must remain finite.

Negative `jumpRate` becomes zero.

Use magnitude for negative `jumpStd` or the existing Gaussian-scale policy consistently.

---

## 68. Required tests

For a finite interval `dt`, verify empirically:

```text
E[DeltaX] = (drift + jumpRate * jumpMean) * dt
```

and approximately:

```text
Var[DeltaX]
    = diffusion^2 * dt
    + jumpRate * dt * (jumpStd^2 + jumpMean^2)
```

Also test:

- `jumpCount` mean approximately equals `jumpRate * dt`;
- `jumpRate = 0` reduces to ordinary drift-diffusion;
- `diffusion = 0` isolates compound jumps;
- `jumpStd = 0` produces fixed-size jumps times Poisson count;
- reset reproduces seeded sequence;
- `.ar` and `.kr` agree statistically under equal elapsed model time;
- no per-jump loop exists in the calculation function.

---

## 69. Creative documentation example

Use the state to control a slowly wandering oscillator/filter parameter.

Use `jumpTrig` to add a brief accent whenever the state undergoes a shock.

The example should make both layers audible:

- continuous local wandering;
- occasional structural relocation.

Keep other modulation deterministic.

---

## 70. Documentation visualization

Plot one trajectory with jump times marked.

Show the continuous-looking path between jumps and explicit discontinuities at jump events.

Optionally show `jumpSize` beneath the trace.

---

# Part XII — `WilsonCowan`

## 71. Concept

`WilsonCowan` introduces **competing excitatory and inhibitory populations**.

This is a population-level model rather than an individual spiking neuron model.

The important behaviors are created by feedback between two activities:

- excitation can amplify activity;
- inhibition suppresses activity;
- their balance can create stable equilibria;
- coupling can create oscillation;
- parameter changes can cause transitions between dynamical regimes.

The Guide should compare:

```text
Izhikevich / RulkovMap
    neuron-like individual excitable/spiking dynamics

WilsonCowan
    population activity emerging from excitation/inhibition feedback

SelfCorrectingTrig
    probabilistic inhibition in event timing

WilsonCowan
    deterministic dynamical inhibition between continuous populations
```

Do not call `WilsonCowan` stochastic by itself. It is a deterministic nonlinear flow that becomes especially interesting when driven by Stochastica stochastic processes.

---

## 72. Equations

Use the normalized two-population form:

```text
dE/dt = (-E + S_E(wEE*E - wEI*I + inputExc)) / tauExc

dI/dt = (-I + S_I(wIE*E - wII*I + inputInh)) / tauInh
```

with stable logistic activation:

```text
S(x; gain, threshold) = 1 / (1 + exp(-gain * (x - threshold)))
```

Implement logistic evaluation in a numerically stable branch form so extreme arguments do not overflow `exp`.

---

## 73. Proposed API

```supercollider
WilsonCowan.ar(
    inputExc = 0,
    inputInh = 0,
    wEE = 10,
    wEI = 12,
    wIE = 10,
    wII = 2,
    tauExc = 0.02,
    tauInh = 0.01,
    gainExc = 1,
    thresholdExc = 0,
    gainInh = 1,
    thresholdInh = 0,
    timeScale = 1,
    substeps = 1,
    initialExc = 0.1,
    initialInh = 0.1,
    reset = 0
)
```

Provide `.kr`.

`substeps` is init-rate and bounded:

```text
1 <= substeps <= 16
```

### Outputs

Return:

```text
excitation
inhibition
```

No internal noise input is required beyond `inputExc`/`inputInh`; users can explicitly patch `OUProcess`, `MaternNoise`, `ShotNoise`, or other sources into those inputs.

---

## 74. Integration

Use the existing Stochastica continuous-flow integration convention.

Preferred final method: fixed-substep RK4.

For each server calculation interval:

```text
dt = calculationDuration * timeScale / substeps
```

Perform exactly `substeps` RK4 steps.

Do not adaptively subdivide in the audio thread.

`tauExc` and `tauInh` must have a documented positive floor.

When `timeScale <= 0`, hold state.

If a full RK4 update becomes non-finite, restore the requested initial state and continue safely.

Do not clip finite `E` or `I` merely to `[0, 1]`; correct integration should keep typical trajectories near the activation range, but hidden clamping would modify the ODE.

---

## 75. Required tests

- derivative implementation matches a reference implementation at selected states;
- fixed-point cases converge appropriately;
- zero/low coupling behaves sensibly;
- RK4 results converge as substeps increase;
- `.ar` and `.kr` agree over equal model time within integration tolerance;
- stable logistic never emits NaN/Inf for extreme input;
- reset restores initial state;
- deterministic output contains no RNG dependency;
- changing excitation/inhibition weights produces expected qualitative regime changes in reference cases.

---

## 76. Creative documentation examples

### Excitation and inhibition as two timbral forces

Map `E` to spectral brightness or excitation density.

Map `I` to attenuation, filtering, or opposing spectral energy.

Keep the source simple so the feedback relationship is audible.

### Noise reveals population dynamics

Use a small external `OUProcess` or `MaternNoise` input to perturb one population.

The documentation should emphasize that the stochastic source is external and modular.

### Crossing regimes

A larger example may slowly change one coupling coefficient so the system passes from a stable region into oscillatory activity.

Do not claim universal bifurcation values without verifying them for the chosen parameterization.

---

## 77. Documentation visualization

Create a deterministic figure containing:

- E and I time series;
- E/I phase portrait;
- one comparison across two coupling regimes.

If nullclines are added, compute them accurately and explain them; otherwise omit them.

---

# Part XIII — `Lorenz96`

## 78. Concept

`Lorenz96` introduces **high-dimensional, locally coupled, spatially extended chaos**.

Instead of a trajectory containing two or three coordinates, there are `N` states arranged around a periodic ring.

Each state interacts with nearby neighbors.

The result can display:

- traveling structures;
- local propagation;
- clustering;
- turbulent-looking motion;
- emergent high-dimensional chaos.

Conceptually:

> Chaos is no longer located in one low-dimensional attractor. It is distributed across a spatially organized population of interacting states.

This makes `Lorenz96` particularly appropriate for banks of oscillators, grains, filters, spatial channels, partials, or agents.

The Guide should connect it to `CoupledMapLattice`:

```text
CoupledMapLattice
    spatially coupled discrete-time nonlinear maps

Lorenz96
    spatially coupled continuous-time nonlinear flow
```

---

## 79. Equation

For periodic indices:

```text
dx_i/dt = (x_(i+1) - x_(i-2)) * x_(i-1) - x_i + forcing
```

Indices wrap around the ring.

Use old-state values consistently inside each derivative evaluation.

---

## 80. Proposed API

```supercollider
Lorenz96.ar(
    numCells = 8,
    forcing = 8,
    timeScale = 1,
    substeps = 1,
    initialBuf = -1,
    reload = 0,
    reset = 0
)
```

Provide `.kr`.

Set:

```text
4 <= numCells <= 64
1 <= substeps <= 16
```

Return exactly `numCells` outputs.

`numCells` and `substeps` are init-rate.

---

## 81. Initial state

If `initialBuf` is valid:

- require one channel with at least `numCells` samples;
- copy the initial vector into RT-owned memory.

If no valid buffer is supplied, use a deterministic conventional perturbation:

```text
x_i = forcing
x_0 = forcing + 0.01
```

For this default mode, reset rebuilds the default initial vector using the current block-sampled `forcing` value.

For buffer mode, reset restores the currently loaded initial vector.

---

## 82. Reload

A reload edge:

- validates/copies `initialBuf` when supplied;
- updates the reset template;
- does not alter current dynamical state immediately;
- requires a separate reset to jump to the new initial state.

This separates “change the stored initial condition” from “restart the flow.”

---

## 83. Integration

Use fixed-substep RK4 consistent with other Stochastica flows.

Allocate two or more fixed-size RT arrays as required for current state and RK stages.

Avoid `O(N^2)` work; the derivative is `O(N)` because each cell reads only local neighbors.

At every RK stage, enforce cyclic indices exactly.

If the full state becomes non-finite, restore the reset template.

Do not normalize or clamp finite states.

---

## 84. Required tests

- derivative matches a simple reference implementation for deterministic state vectors;
- cyclic indexing is correct for cells 0, 1, `N-2`, and `N-1`;
- a spatial rotation of the initial vector produces the corresponding rotated derivative/output trajectory within numerical tolerance;
- equilibrium/low-forcing reference cases behave as expected;
- forcing around the conventional chaotic regime produces bounded finite chaotic-looking trajectories under documented time scales;
- RK4 convergence improves with substeps;
- reset restores exact initial vector;
- reload updates reset template without changing current state;
- `.ar`/`.kr` agree over equal model time within integration tolerance;
- 64-cell worst-case CPU remains bounded and profiled.

---

## 85. Creative documentation example

Create a 16-cell Lorenz96 network and map the states to a bank of related voices.

A strong example is:

- one cell per oscillator/filter/partial;
- fixed base frequencies distributed across a scale or spectrum;
- cell state controls amplitude or filter deviation;
- `Splay` or another core spatial arrangement exposes movement across the population.

This is a legitimate large example and may use many variables.

The main concept must remain audible: neighboring state interaction produces waves and collective texture.

Do not add independent random modulation to every voice.

---

## 86. Documentation visualization

Generate a space-time heatmap:

- horizontal axis: time;
- vertical axis: cell index;
- value: cell state.

Also include one or two individual cell traces if helpful.

This is substantially more informative than plotting a conventional 2-D phase portrait for a high-dimensional system.

---

# Part XIV — Guide-level conceptual examples

## 87. The Guide should contain only a few large examples

Do not turn `Guides/Stochastica.schelp` into an enormous cookbook.

The Guide should primarily explain categories and relationships.

Use a few examples where combining processes teaches a concept that no individual helpfile can show as clearly.

---

## 88. Recommended Guide example: allocation + reinforcement

Combine:

- `TDirichletRand` as a fresh random allocation of a whole;
- `PolyaUrn` as historical reinforcement.

The prose should explain:

> Dirichlet randomness redistributes a whole independently at each draw, while the urn changes its own future distribution according to history.

Do not make the example musically elaborate.

---

## 89. Recommended Guide example: three kinds of regime behavior

Conceptually contrast:

- `RandomTelegraph` — state itself switches;
- `MarkovModulatedTrig` — hidden state changes event statistics;
- `DoubleWellProcess` — continuous nonlinear state moves stochastically between metastable regions.

This is a valuable conceptual comparison even if only one or two short code examples are shown.

---

## 90. Recommended Guide example: four types of stochastic movement

Explain:

```text
OUProcess       local mean-reverting diffusion
MaternNoise     stationary correlated motion with explicit smoothness
LevyFlight      heavy-tailed anomalous transport
JumpDiffusion   continuous diffusion + finite-rate explicit shocks
```

This comparison should become one of the central pieces of the Guide.

---

## 91. Recommended Guide example: interaction across scales

Explain the progression:

```text
MultiHawkesTrig   interacting point processes
KuramotoNetwork   interacting oscillator phases
WilsonCowan       interacting excitatory/inhibitory populations
Lorenz96          interacting states distributed across a ring
```

The point is that “interaction” is not one algorithm. It can organize event probability, phase, population activity, or spatially distributed dynamical state.

---

# Part XV — Helpfile requirements per new class

## 92. Mandatory structure

Every new class helpfile should contain, in this approximate order:

```text
class::
summary::
categories::
related::

description::
conceptual explanation
mathematical/algorithmic clarification

optional early conceptual subsection / image

classmethods::
arguments
returns

instancemethods::

examples::
minimal examples first
larger conceptual example last
```

Do not mechanically add empty sections that the repository convention does not require.

Again: `related::` is at the top.

Never append `See Also` after examples.

---

## 93. Each class page must answer five questions

1. **What is it mathematically?**
2. **What does its state remember?**
3. **What kind of behavior does that memory/structure create?**
4. **How does it differ from the most similar existing Stochastica class?**
5. **What is one musically meaningful way to expose that distinction?**

If a helpfile cannot answer these questions, it is incomplete.

---

## 94. Class-specific related links

Recommended top-level relationships:

### `MaternNoise`

Link to:

- `OUProcess`;
- `FractionalNoise`;
- `FBrownianMotion`;
- `MultiOUProcess` if present;
- Stochastica Guide.

### `TDirichletRand`

Link to:

- `TGammaRand`;
- `MultiGaussNoise`;
- `PolyaUrn`;
- Stochastica Guide.

### `PolyaUrn`

Link to:

- `MarkovChain`;
- `SemiMarkov`;
- `TDirichletRand`;
- Stochastica Guide.

### `MarkovModulatedTrig`

Link to:

- `MarkovChain`;
- `SemiMarkov`;
- `RenewalTrig`;
- `HawkesTrig`;
- `RandomTelegraph` if present;
- Stochastica Guide.

### `LevyFlight`

Link to:

- `TStableRand`;
- `TLevyRand`;
- `OUProcess`;
- `FBrownianMotion`;
- `JumpDiffusion`;
- Stochastica Guide.

### `JumpDiffusion`

Link to:

- `OUProcess`;
- `ShotNoise` if present;
- `TPoissonRand`;
- `LevyFlight`;
- Stochastica Guide.

### `WilsonCowan`

Link to:

- `Izhikevich`;
- `RulkovMap`;
- `SelfCorrectingTrig` if present;
- Stochastica Guide.

### `Lorenz96`

Link to:

- `CoupledMapLattice`;
- `Chua`;
- `KuramotoNetwork` if present;
- Stochastica Guide.

Do not create broken links to classes absent from the branch.

---

# Part XVI — Documentation images and scripts

## 95. Images must explain mathematics

Documentation images are not decorative artwork.

Use each image to reveal a property that is hard to understand from prose alone.

Required image concepts:

| Class | Required visualization |
|---|---|
| `MaternNoise` | time series + autocorrelation across smoothness modes |
| `TDirichletRand` | 3-component simplex/ternary samples |
| `PolyaUrn` | evolving proportions + entropy across several runs |
| `MarkovModulatedTrig` | hidden state + intensity + event raster |
| `LevyFlight` | state + increment/jump magnitude distribution |
| `JumpDiffusion` | trajectory with explicit jump markers |
| `WilsonCowan` | E/I traces + phase portrait |
| `Lorenz96` | cell-index × time heatmap |

---

## 96. Reproducible image generation

Place image-generation scripts in the repository's existing documentation/script location.

Rules:

- fixed seeds where randomness is involved;
- no manual image editing required to reproduce figures;
- scripts are offline/documentation tools, not server code;
- use labels that match UGen argument names;
- do not use misleading axis scaling;
- do not normalize away the property the image is meant to demonstrate;
- store only final useful assets, not temporary debug plots.

---

# Part XVII — Testing strategy

## 97. Four layers of tests

Every new UGen should be covered at the appropriate layers.

### 97.1 Deterministic/reference tests

Use for:

- reset semantics;
- seeded sequences;
- derivative/recurrence correctness;
- buffer ordering;
- output ordering;
- reload behavior.

### 97.2 Statistical tests

Use for:

- Matérn covariance;
- Dirichlet moments;
- Pólya reinforcement behavior;
- Markov-modulated dwell/event statistics;
- stable increment tail/scale behavior;
- jump-diffusion moments.

Tests must use tolerances appropriate to finite Monte Carlo samples.

Do not write flaky exact-distribution tests.

### 97.3 Safety/fuzz tests

Exercise:

- NaN parameters;
- infinities;
- negative rates;
- zero time scales;
- invalid buffers;
- too-small buffers;
- all-zero transition rows;
- extreme concentrations;
- extreme stable alpha values within supported range;
- maximum dimensions;
- repeated reset/reload edges.

The server must remain alive and outputs must remain finite or follow the documented recovery policy.

### 97.4 Performance tests

Profile worst supported configurations:

- `TDirichletRand` with 32 channels and dense trigger rate;
- `PolyaUrn` with 64 states;
- `MarkovModulatedTrig` with 64 states;
- `MaternNoise.ar` at audio rate;
- `Lorenz96` with 64 cells and maximum supported substeps;
- multiple simultaneous `WilsonCowan` instances.

Record results in development notes; do not make documentation claims such as “cheap” without measurement.

---

# Part XVIII — Implementation order

## 98. Phase 0 — Repository audit and baselines

Before adding Round 3 code:

1. build the current repository in Release mode;
2. run all existing tests;
3. capture deterministic reference outputs for existing stochastic helper users that may be refactored;
4. inspect current Guide/class categories;
5. inventory current classes so the new Guide contains no stale names;
6. verify whether Round 2 is present.

Do not begin by editing the Guide based on an outdated class list.

---

## 99. Phase 1 — Shared-helper preparation

Only if required:

- expose bounded Gamma sampler;
- expose stable sampler;
- expose Poisson sampler;
- expose categorical weighted-choice helper;
- expose small fixed-size linear-SDE helpers;
- expose flow integration helpers.

After every helper extraction, rerun golden tests for existing UGens.

No behavioral difference is acceptable.

---

## 100. Phase 2 — Distribution and allocation

Implement:

1. `TDirichletRand`;
2. tests;
3. language class;
4. helpfile;
5. visualization script.

This validates multichannel triggered distribution infrastructure early.

---

## 101. Phase 3 — History and latent state

Implement:

1. `PolyaUrn`;
2. `MarkovModulatedTrig`;
3. state/buffer tests;
4. helpfiles;
5. documentation images.

These two classes should reuse existing Markov/categorical infrastructure where possible.

---

## 102. Phase 4 — Continuous stochastic movement

Implement:

1. `MaternNoise`;
2. `LevyFlight`;
3. `JumpDiffusion`.

Do not accept approximate or vague time scaling.

Validate `.ar`/`.kr` finite-time behavior statistically.

---

## 103. Phase 5 — Population and spatial flows

Implement:

1. `WilsonCowan`;
2. `Lorenz96`.

Reuse flow conventions and RK4 implementation.

Verify high-dimensional output registration before writing large examples.

---

## 104. Phase 6 — Guide conceptual rewrite

Only after class names/APIs are stable:

1. update the class overview;
2. preserve technical mathematical categories;
3. add the behavioral/conceptual taxonomy;
4. properly categorize all original/v1 classes;
5. integrate Round 2 classes when present;
6. integrate Round 3 classes;
7. add the three-round conceptual progression;
8. add comparison tables;
9. add only a small number of Guide-level examples;
10. verify every link;
11. enforce the top-only `related::`/`See Also` rule.

---

# Part XIX — Suggested source organization

## 105. File placement

Follow the actual repository structure, but a reasonable distribution is:

```text
processes/
    MaternNoise.cpp
    MarkovModulatedTrig.cpp
    LevyFlight.cpp
    JumpDiffusion.cpp

distributions/
    TDirichletRand.cpp

state/
    PolyaUrn.cpp

flows/
    WilsonCowan.cpp
    Lorenz96.cpp
```

If the existing repository groups several small related classes in one source file, follow that convention only when it remains readable.

Do not create eight one-function translation units if the repository clearly prefers coherent grouped source modules, and do not create one giant Round3.cpp file.

---

# Part XX — Registration and language classes

## 106. Server registration

Register every new UGen using the repository's existing plugin registration pattern.

Verify:

- correct input count;
- correct variable output count;
- constructor/destructor pairing;
- RT allocation cleanup;
- `.ar` and `.kr` calculation functions;
- buffer lock usage;
- supernova compatibility where existing Stochastica supports it.

---

## 107. Multi-output classes

Use `MultiOutUGen` or the repository's established equivalent for:

- `TDirichletRand`;
- `PolyaUrn`;
- `MarkovModulatedTrig`;
- `LevyFlight`;
- `JumpDiffusion`;
- `WilsonCowan`;
- `Lorenz96`.

`MaternNoise` is scalar.

Do not hide variable server outputs inside a language-side array transformation that changes their ordering unpredictably.

Document output order explicitly.

---

# Part XXI — Creative examples by concept

## 108. Smoothness rather than “noise color” — `MaternNoise`

Design an example in which one synthesis parameter is modulated by three Matérn processes with equal variance and correlation time but different smoothness.

The musical question is:

> How does the same stochastic scale feel when its path geometry changes from rough to smooth?

Do not add unrelated noise sources.

---

## 109. Allocation rather than scalar choice — `TDirichletRand`

Use four timbral layers sharing a fixed total gain.

At each trigger, Dirichlet weights redistribute the total among them.

The example should demonstrate conservation/allocation, not just random amplitude modulation.

---

## 110. History becomes preference — `PolyaUrn`

Use several initially equal materials.

Repeated choices gradually create a favorite.

Map entropy to a global property so the system audibly becomes less diverse as reinforcement develops.

This example should feel qualitatively different on different seeds.

---

## 111. Hidden state controls visible activity — `MarkovModulatedTrig`

Keep the triggered sound identical.

Let hidden states alter only event density.

This reveals latent regimes much more clearly than assigning a different pitch to every state.

---

## 112. Local exploration with rare relocation — `LevyFlight`

Use the accumulated process to move through register, filter space, or another one-dimensional parameter domain.

Let extreme increments remain rare and structurally important.

Do not compress all values with `tanh` before the listener can perceive the difference between local and long-range movement; instead use a thoughtful user-side mapping/clipping stage.

---

## 113. Wandering plus shocks — `JumpDiffusion`

Use the continuous state for slow parameter motion and the jump trigger for a transient accent.

The listener should be able to identify two causal mechanisms:

- diffusion;
- shock.

---

## 114. Excitation fights inhibition — `WilsonCowan`

Use excitation and inhibition to control opposing dimensions of one synthesis system.

For example:

- excitation raises brightness/density;
- inhibition closes a filter or reduces excitation amplitude.

A parameter sweep can reveal a transition to oscillatory collective behavior.

---

## 115. Chaos becomes spatial population texture — `Lorenz96`

Use 8–16 cells to control a bank of related voices.

Preserve the cell ordering in panning or frequency so propagation across neighboring cells can be heard as movement rather than an unordered cloud.

This example is intentionally larger than the others.

---

# Part XXII — Conceptual comparison text required in the Guide

## 116. Randomness with and without memory

Explain clearly:

```text
Triggered distribution
    independent draw; no evolving stochastic state

OU / Matern / Levy / JumpDiffusion
    evolving stochastic state through time

MarkovChain
    current discrete state affects next state

PolyaUrn
    accumulated historical choices change future probabilities
```

---

## 117. Four meanings of “interaction”

Explain:

```text
MultiHawkesTrig
    events alter other event intensities

KuramotoNetwork
    oscillator phases mutually synchronize

WilsonCowan
    excitatory and inhibitory populations feed back on each other

Lorenz96
    neighboring spatial states exchange nonlinear influence
```

This comparison is central to the mature Stochastica Guide.

---

## 118. Four meanings of “irregular movement”

Explain:

```text
OUProcess
    mean-reverting Gaussian diffusion

MaternNoise
    stationary Gaussian motion with explicit smoothness/correlation scale

LevyFlight
    heavy-tailed anomalous transport

JumpDiffusion
    Gaussian diffusion plus explicit finite-rate jumps
```

Avoid describing all four generically as “organic random LFOs.”

The documentation should teach why they are mathematically and perceptually different.

---

## 119. Three meanings of “regime”

Explain:

```text
RandomTelegraph
    discrete stochastic state persists then switches

MarkovModulatedTrig
    latent state changes the statistics of an observed event stream

DoubleWellProcess
    continuous nonlinear state occupies metastable regions and crosses barriers under noise
```

---

## 120. Discrete chaos versus stochasticity

The Guide must explicitly state:

- chaotic maps are deterministic given state and parameters;
- stochastic processes consume random innovations;
- both can appear unpredictable;
- sensitivity to initial conditions is not the same mechanism as probabilistic randomness.

This distinction is fundamental to Stochastica and should remain visible as the library grows.

---

# Part XXIII — Documentation categories metadata

## 121. Audit `categories::`

Audit current helpfile `categories::` metadata for all Stochastica classes.

Do not invent dozens of incompatible SuperCollider documentation paths.

Use a small coherent taxonomy such as the repository already follows.

Recommended direction:

```text
UGens>Generators>Chaotic>Maps
UGens>Generators>Chaotic>Flows
UGens>Generators>Stochastic>Processes
UGens>Generators>Stochastic>Distributions
UGens>Generators>Stochastic>State
UGens>Triggers>Stochastic
```

Exact paths must be checked against current SuperCollider/Stochastica conventions before editing.

The goal is consistency, not metadata churn.

A class may have a technical category in metadata while receiving richer behavioral categorization in the Guide.

Do not try to encode the entire conceptual taxonomy into the `categories::` line.

---

# Part XXIV — Commit/PR strategy

## 122. Keep commits conceptually reviewable

Suggested commit sequence:

```text
Stochastica: expose shared bounded distribution helpers for Round 3
Stochastica: add TDirichletRand
Stochastica: add PolyaUrn
Stochastica: add MarkovModulatedTrig
Stochastica: add MaternNoise
Stochastica: add LevyFlight
Stochastica: add JumpDiffusion
Stochastica: add WilsonCowan
Stochastica: add Lorenz96
Stochastica: add Round 3 statistical and reference tests
Help: add Round 3 class documentation
Help: reorganize Stochastica conceptual taxonomy
Help: categorize original Stochastica classes consistently
```

If no helper extraction is needed, omit the first commit.

Do not mix all C++, all helpfiles, and all Guide changes into one enormous commit.

---

## 123. Possible PR grouping

A maintainable series could be:

### PR A — Allocation and reinforced state

- `TDirichletRand`
- `PolyaUrn`

### PR B — Hidden regimes and transport

- `MarkovModulatedTrig`
- `LevyFlight`
- `JumpDiffusion`

### PR C — Smooth Gaussian process

- `MaternNoise`

### PR D — Population and spatial dynamics

- `WilsonCowan`
- `Lorenz96`

### PR E — Guide taxonomy and Round 3 documentation integration

The exact grouping can follow repository-maintainer preference.

---

# Part XXV — References and mathematical verification

## 124. Verify formulas from primary/standard references

Before implementation, verify mathematical definitions against standard sources.

At minimum consult reliable references for:

- Matérn covariance and state-space representations;
- Dirichlet sampling through normalized Gamma variables;
- classical Pólya urn reinforcement;
- Markov-modulated Poisson/event processes;
- symmetric alpha-stable Lévy motion scaling;
- compound-Poisson jump diffusion moments;
- Wilson–Cowan population equations;
- Lorenz–96 cyclic dynamics.

Do not copy code from restrictively licensed implementations.

Implement equations independently or reuse code only when its license is compatible with the repository.

Document any non-obvious parameterization choice, particularly stable-distribution conventions.

---

# Part XXVI — Definition of done

## 125. A new UGen is complete only when all requirements hold

For each of the eight classes:

- server code builds cleanly;
- language class loads;
- `.ar`/`.kr` behavior is implemented where specified;
- inputs and outputs match the documented API;
- no RT-unsafe allocation or unbounded loop exists;
- reset behavior is deterministic and tested;
- seed behavior is tested for stochastic classes;
- malformed parameters/buffers fail safely;
- mathematical/statistical reference tests pass;
- performance limits are documented;
- `.schelp` helpfile exists;
- conceptual explanation is present;
- first examples are minimal;
- creative example demonstrates the process-specific idea;
- runnable code is never commented out;
- explanatory comments appear only inside outer `(...)` runnable blocks;
- top-level `related::` metadata is present;
- no bottom `See Also` section exists;
- documentation image/script exists when required;
- Guide links resolve.

---

## 126. Round 3 is complete only when the Guide itself is improved

The round is **not** complete merely because the eight UGens compile.

`HelpSource/Guides/Stochastica.schelp` must also:

1. retain the technical distinction among maps, flows, processes, distributions, state/vector models, and sonifiers;
2. properly categorize and explain the original/v1 Stochastica classes;
3. distinguish deterministic chaos from stochastic randomness;
4. explain the Round 2 forms of organization when those classes are present;
5. explain Round 3 as memory/hidden organization/geometry/transport/spatial structure;
6. include the “Forms of organization” navigation table;
7. compare closely related processes rather than describing everything as generic randomness;
8. keep Guide examples sparse and concept-driven;
9. contain no broken links;
10. enforce top-only related/See Also placement.

---

# Part XXVII — Final conceptual target

Stochastica should no longer read as a collection of miscellaneous chaotic equations and unusual random-number generators.

After Round 3, the library should present a coherent vocabulary for **ways that uncertainty and nonlinear dynamics can be organized in time and across agents**.

The mature conceptual map should include:

- deterministic nonlinear iteration;
- continuous nonlinear feedback;
- shaped independent randomness;
- mean-reverting diffusion;
- long-memory stochasticity;
- controlled stochastic smoothness;
- renewal timing;
- self-excitation;
- cross-excitation;
- inhibition;
- persistent regimes;
- hidden regimes;
- stochastic impulses;
- correlated multivariate motion;
- random allocation;
- reinforcement and path dependence;
- metastability;
- anomalous transport;
- diffusion with explicit shocks;
- circular probability;
- oscillator synchronization;
- excitatory/inhibitory population dynamics;
- spatially extended chaos.

The documentation should make these differences usable compositionally.

A reader should finish the Stochastica Guide understanding not only **which UGen exists**, but **what kind of organization each model introduces, what it remembers, what interacts with what, and why that distinction might matter in sound**.

That conceptual clarity is as important as the C++ implementation.
