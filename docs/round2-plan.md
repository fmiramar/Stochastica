# Stochastica — Round 2 Agentic Implementation Plan

## Interaction, inhibition, regime switching, stochastic impulses, correlated motion, metastability, circular statistics, and collective synchronization

**Target repository:** `https://github.com/fmiramar/Stochastica`

**Scope:** add eight new UGens to the existing Stochastica architecture. This round is an expansion only. Do not repair, refactor, rename, deprecate, or behaviorally change any existing UGen unless a compile-only change is strictly required to register the new classes.

**New UGens:**

1. `MultiHawkesTrig`
2. `SelfCorrectingTrig`
3. `RandomTelegraph`
4. `ShotNoise`
5. `MultiOUProcess`
6. `DoubleWellProcess`
7. `TVonMisesRand`
8. `KuramotoNetwork`

**Language:** C++17 server plugin code, SuperCollider language classes, `.schelp` helpfiles, and the existing Stochastica test/documentation infrastructure.

---

# Part I — Non-negotiable project rules

## 1. Preserve the existing Stochastica design

The current repository already separates mathematical categories into `maps`, `flows`, `processes`, `distributions`, `state`, and `sonifiers`. Keep that architecture.

The implementation agent must inspect the repository before editing and reuse the existing helpers, naming conventions, registration pattern, CMake style, test utilities, PCG32 generator, buffer locks, reset semantics, and documentation layout wherever they already solve the problem correctly.

Do not create a second stochastic framework beside the existing one.

Do not duplicate an existing helper merely because a new local helper is easier to write.

Do not move existing source files as part of this round.

---

## 2. No bug-fix work in this implementation round

Do not include:

- fixes to existing Stochastica classes;
- fixes to legacy SuperCollider or sc3-plugins stochastic UGens;
- seeded-sequence changes to existing classes;
- changes to existing mathematical equations;
- cleanup commits unrelated to the eight new classes;
- broad source reformatting;
- renaming of existing arguments;
- replacement of existing helpfile examples;
- unrelated CMake modernization.

If an existing defect blocks compilation or registration of a new class, isolate the smallest possible compatibility change and document why it was unavoidable.

---

## 3. Real-time safety

Every server calculation function must obey the same rules as the first Stochastica implementation document:

- no heap allocation in the calculation function;
- no file access;
- no unbounded loop;
- no exception propagation;
- no console output from the sample loop;
- no matrix factorization in the sample loop;
- no buffer resizing in the sample loop;
- no dynamic output-count changes;
- no standard-library RNG whose sequence differs by platform;
- no algorithm whose running time can grow without a documented hard bound.

All rejection sampling and event-catch-up loops must have explicit compile-time attempt limits.

When a bounded loop cannot complete, use the process-specific fallback defined in this plan. Never keep iterating until success on the real-time thread.

---

## 4. Trigger semantics

Every `trig`, `reset`, and `reload` input uses the existing Stochastica rising-edge rule:

```text
current > 0 && previous <= 0
```

A held-positive trigger creates one edge only.

Reset is atomic.

For a sample on which both reset and another event occur, reset has priority unless the individual UGen section explicitly defines otherwise.

---

## 5. Seed semantics

Use the existing Stochastica local PCG32 implementation.

- `seed = 0` derives a seed from the parent Synth.
- a nonzero seed is deterministic;
- reset restores the original local seed;
- new seeded classes must not consume the parent Synth RNG after construction;
- deterministic reference sequences must be stored in tests.

Do not replace the repository RNG during this round.

---

## 6. Floating-point policy

- Keep long-lived process state in `double` unless the existing repository convention requires otherwise.
- Write server outputs as `float`.
- Check complete state updates for finiteness.
- Do not silently fold, wrap, or clip raw stochastic state merely to make it convenient for audio.
- Explicit rate ceilings and mathematical periodicity are permitted when they are part of the documented API.
- A non-finite internal process state must recover using the class-specific reset/fallback policy rather than poisoning the server graph with NaNs.

---

## 7. Documentation code rules — mandatory

These rules are especially important and must be enforced in every new `.schelp` file and every new runnable example added to `Guides/Stochastica.schelp`.

### 7.1 Minimal examples

Start with the smallest example that exposes the idea.

Prefer:

```supercollider
{ Decay2.ar(SelfCorrectingTrig.ar(6, 1.2)[0], 0.001, 0.04) * 0.1 }.play
```

Do not expand a one-line idea into six temporary variables.

### 7.2 Variables

Use variables only when they:

- prevent duplicated UGens;
- make a genuinely multistage example readable;
- represent buffer lifecycle;
- make a large conceptual example understandable.

Large examples may use many variables. Small examples should not.

### 7.3 Never comment out runnable code

Any line intended to be evaluated must be active code.

Do not provide disabled alternatives such as:

```supercollider
// { ... }.play
```

Do not ask the reader to uncomment a line.

### 7.4 Comments are allowed only inside a main `(...)` block

A runnable documentation example may contain short explanatory comments only inside its parenthesized main block.

Allowed:

```supercollider
(
{
    var x;
    // Moderate noise allows occasional well-to-well transitions.
    x = DoubleWellProcess.kr(noise: 0.45);
    SinOsc.ar(x.linlin(-2, 2, 180, 900), 0, 0.08)
}.play;
)
```

Do not place a `// Example title` line before the opening parenthesis.

Use ordinary `.schelp` prose headings to explain the example instead.

### 7.5 No commented alternatives inside examples

Even inside `(...)`, comments may explain behavior but may not contain alternate executable code.

### 7.6 Buffer examples

Prepare buffers inside a `Routine`, call `s.sync`, and only then create the Synth.

Do not rely on timing accidents between buffer allocation and Synth creation.

### 7.7 Self-contained examples

Examples must use only SuperCollider core plus Stochastica.

Do not require external Quarks, private helper functions, or local sound files.

---

# Part II — Why this round exists

## 8. Conceptual goal

The first Stochastica round established a broad vocabulary of:

- discrete chaotic maps;
- continuous nonlinear flows;
- independent probability distributions;
- scalar stochastic processes;
- self-exciting events;
- fractional processes;
- Markov state systems;
- correlated random vectors.

The second round should not simply add more equations of the same type.

Its purpose is to add **new forms of stochastic organization**.

The eight classes should make the library capable of expressing:

- **interaction** — one event stream changes the probability of events in another stream;
- **inhibition** — events suppress their own immediate recurrence and create anti-clustering;
- **regime switching** — a process jumps between persistent discrete modes rather than merely wandering continuously;
- **stochastic impulses** — random events accumulate as a continuous physical-looking signal;
- **correlated motion** — several continuously evolving variables remember the past while sharing stochastic innovations;
- **metastability and stochastic resonance** — noise can move a nonlinear system between stable regions and can reveal a weak periodic drive;
- **circular statistics** — randomness lives on a circle, where `-pi` and `pi` describe the same direction;
- **collective synchronization** — many oscillatory agents can spontaneously move from disorder toward coordinated phase behavior.

That conceptual expansion is the central criterion for this round.

A successful implementation should make Stochastica feel less like an extended catalogue of noises and attractors and more like a **real-time stochastic-process toolkit for composition, synthesis, interaction, and generative systems**.

---

# Part III — General guide update

## 9. Update `HelpSource/Guides/Stochastica.schelp`

Do not replace the existing guide. Extend it.

Keep the current introductory distinction among distributions, processes, maps, continuous systems, state/vector processes, and sonifiers.

Add the new classes to the class overview table, then add a new subsection immediately after the existing category explanation.

Suggested subsection title:

```text
subsection:: Forms of stochastic organization
```

The guide must explain the following concepts in plain language before presenting equations.

---

## 10. Required conceptual guide content

The final guide should express the following ideas clearly. The wording may be edited to fit `.schelp`, but the conceptual content must remain.

### 10.1 Interaction

Independent random streams do not affect each other. `MultiHawkesTrig` introduces **interaction**: an event in one stream can temporarily raise the probability of events in another stream. This permits contagion, call-and-response, cascades, swarms, and networks in which activity propagates between agents.

The important compositional distinction is that correlation is not merely observed in the outputs; one process is causally changing another process's future event intensity.

### 10.2 Inhibition

`SelfCorrectingTrig` introduces **negative feedback in event time**. As time passes without an event, the probability of an event rises. When an event occurs, its own probability is pushed back down. The result remains stochastic, but closely spaced events become less likely.

This produces anti-clustering, stochastic regularity, spacing, refractory behavior, and rhythms that correct their own local density without becoming a metronome.

The guide should explicitly contrast this with `HawkesTrig`:

```text
HawkesTrig            event -> more near-future events
SelfCorrectingTrig    event -> fewer near-future events
```

### 10.3 Regime switching

`RandomTelegraph` introduces **persistent stochastic states**. Instead of continuously wandering through values, it remains in one regime for a random duration and then switches to another.

This is useful whenever a generative system should alternate between conditions such as open/closed, dense/sparse, bright/dark, consonant/noisy, active/resting, or one algorithm versus another.

The duration of each regime is stochastic, so the switching is neither a fixed sequencer nor memoryless sample-by-sample noise.

### 10.4 Stochastic impulses

`ShotNoise` treats random events as **physical impulses with memory**. Each stochastic arrival contributes energy to a decaying state, and overlapping events accumulate.

This bridges point processes and continuous noise. Sparse arrivals produce isolated impacts; increasing the rate makes them overlap into crackle, rain, granular agitation, or eventually a dense fluctuation field.

The guide should distinguish the event times from the accumulated process signal.

### 10.5 Correlated motion

`MultiOUProcess` extends mean-reverting stochastic motion into a vector. Each component has temporal memory, but the random innovations can also be correlated between components.

This means several parameters can move independently without feeling unrelated. A group of voices may drift separately while still breathing together, or several spatial coordinates may share a tendency without collapsing into the same signal.

The distinction from `MultiGaussNoise` must be explicit:

```text
MultiGaussNoise    correlated values at individual trigger moments
MultiOUProcess     correlated trajectories with memory through time
```

### 10.6 Metastability and stochastic resonance

`DoubleWellProcess` introduces **noise acting inside a nonlinear energy landscape**. The process can spend a long time near one stable region, then noise can push it across a barrier into another region.

This is metastability: the system has multiple locally stable behaviors and stochastic transitions between them.

When a weak periodic drive is too small to cause transitions by itself, an intermediate amount of noise can make transitions become statistically aligned with the drive. This is the central intuition of stochastic resonance. The guide must explain this without implying that more noise is always better.

### 10.7 Circular statistics

`TVonMisesRand` introduces probability on a **periodic domain**. Ordinary Gaussian randomness assumes an infinite line, but angles wrap: `-pi` and `pi` are the same direction.

A von Mises distribution can therefore cluster random phases, directions, azimuths, or cyclic positions around a preferred angle without creating an artificial discontinuity at the wrap boundary.

### 10.8 Collective synchronization

`KuramotoNetwork` introduces **emergent collective behavior**. Each oscillator has its own natural frequency, but coupling encourages phases to align. Weak coupling produces disorder; stronger coupling can create partial or global synchronization.

The synchronization is not imposed by a master clock. It emerges from mutual coupling among the oscillators.

The order parameter provides a compact measurement of this collective state:

```text
r near 0    phases are widely dispersed
r near 1    phases are strongly synchronized
```

---

## 11. Guide table additions

Extend the guide class table without changing existing entries.

Add:

```text
Processes || MultiHawkesTrig, SelfCorrectingTrig, RandomTelegraph, ShotNoise, MultiOUProcess, DoubleWellProcess
Distributions || TVonMisesRand
Continuous / collective systems || KuramotoNetwork
```

If the current table groups all stochastic processes under `Processes`, keep that convention rather than creating gratuitous new category names.

---

# Part IV — Shared implementation additions

## 12. Reuse before adding helpers

Before creating any shared code, inspect:

- the current RNG helper;
- `HawkesTrig` implementation;
- `OUProcess` implementation;
- `MultiGaussNoise` buffer and Cholesky code;
- the existing flow integration helpers;
- the current `MultiOutUGen` language classes;
- the current buffer lock and reload conventions.

Only add a helper if at least two of the new classes need it or if it materially reduces correctness risk.

---

## 13. Suggested shared helpers

The following helpers may be added if equivalent functionality is not already present.

### 13.1 Exponential hazard threshold

```cpp
double exponentialHazardThreshold(PCG32& rng);
```

Return:

```text
-log(U), with U strictly inside (0, 1)
```

Use this for continuous-time event scheduling in `RandomTelegraph` and `ShotNoise`.

### 13.2 Stable hazard probability

```cpp
double eventProbability(double intensity, double dt);
```

Implement numerically stable:

```text
1 - exp(-intensity * dt)
```

using `expm1` where appropriate.

### 13.3 Circular wrap

If the repository does not already provide the exact required behavior, provide one internal helper returning radians in `[-pi, pi)`.

Do not use this helper for non-circular raw processes.

### 13.4 Symmetric covariance factorization

`MultiOUProcess` should reuse the same Cholesky validation/fallback machinery as `MultiGaussNoise` rather than introduce a separate matrix stack.

---

# Part V — `MultiHawkesTrig`

## 14. Concept

`MultiHawkesTrig` is a network of interacting event processes.

A scalar Hawkes process is self-exciting: one event raises its own near-future event rate. A multivariate Hawkes process generalizes this idea so that an event in source process `j` may raise the intensity of target process `i`.

This allows the UGen to model:

- rhythmic contagion;
- cascading percussion;
- interacting agents;
- call-and-response;
- swarms;
- propagating activity;
- event networks in which some streams lead and others follow.

The key idea is **causal interaction among event streams**, not merely correlation after the fact.

---

## 15. Mathematical model

For `N` processes:

```text
lambda_i(t) = mu_i + sum_j h_ij(t)
```

with exponentially decaying pairwise excitation:

```text
dh_ij/dt = -beta_ij * h_ij
```

and when source process `j` emits an event:

```text
h_ij <- h_ij + alpha_ij
```

Use row-major matrices with:

```text
row i    target process i
column j source process j
```

For a pair `(i, j)`:

```text
alpha_ij = 0
```

means that source `j` does not excite target `i`.

---

## 16. Proposed language API

```supercollider
MultiHawkesTrig.ar(
    numProcesses = 2,
    baseRateBuf = 0,
    excitationBuf = 1,
    decayBuf = 2,
    maxRate = 20000,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr` with the same arguments.

### Outputs

Return exactly `2 * numProcesses` outputs:

```text
[ trig_0 ... trig_N-1, intensity_0 ... intensity_N-1 ]
```

Document this ordering prominently.

The language class may provide no hidden restructuring. Keep the server output explicit and predictable.

---

## 17. Buffer format

### `baseRateBuf`

One channel, at least `N` samples:

```text
mu_0, mu_1, ... mu_N-1
```

Units: events per second.

Negative values become zero when loaded.

### `excitationBuf`

One channel, at least `N * N` samples, row-major:

```text
alpha_00 alpha_01 ... alpha_0N
alpha_10 alpha_11 ... alpha_1N
...
```

Negative excitation values become zero in this UGen.

Do not use negative Hawkes kernels to implement inhibition. `SelfCorrectingTrig` exists to represent inhibitory event organization clearly.

### `decayBuf`

One channel, at least `N * N` samples, row-major.

Each value is `beta_ij` in inverse seconds.

Negative values become zero. A zero decay value intentionally creates persistent excitation; the explicit `maxRate` ceiling keeps event intensity finite. Document this clearly because a zero-decay feedback loop can drive a network toward the ceiling.

---

## 18. Runtime algorithm

Allocate at construction:

- `N` base rates;
- `N * N` excitation coefficients;
- `N * N` decay coefficients;
- `N * N` current excitation states;
- `N` pre-event intensities;
- `N` event flags.

Set a conservative first-version maximum:

```text
1 <= numProcesses <= 16
```

This class is `O(N^2)` per calculation. Do not claim that 64-process audio-rate operation is inexpensive.

For each audio sample or control calculation:

1. exponentially decay every `h_ij` over `dt`;
2. compute all pre-event intensities;
3. apply the explicit `maxRate` ceiling;
4. compute each event probability from its pre-event intensity;
5. draw all event flags before applying any new excitation;
6. add all excitation caused by all source events simultaneously;
7. output event flags and the pre-event intensities.

The simultaneous event step is mandatory. Do not let array iteration order determine whether process 0 can affect process 1 on the same sample while process 1 cannot affect process 0.

New events affect probabilities beginning on the next calculation.

---

## 19. Reset and reload

### Reset

A reset must:

- clear all excitation states to zero;
- restore the original RNG seed;
- output zero event triggers on the reset sample;
- output the current base-rate intensities after the reset.

### Reload

A reload must:

- reread all three buffers under the repository's approved buffer lock;
- validate dimensions;
- copy coefficients into RT-owned arrays;
- leave the current excitation state intact unless dimensions are invalid;
- not reset the RNG.

Malformed buffers must fail safely.

---

## 20. Statistical and behavioral tests

Required tests:

- all-zero excitation approximates independent Poisson streams;
- diagonal excitation with zero off-diagonal entries behaves like independent scalar Hawkes processes;
- a nonzero `alpha_10` increases event density in process 1 following process 0 events;
- `alpha_10 > 0`, `alpha_01 = 0` produces directional influence rather than symmetric coupling;
- event-generation result is independent of internal loop ordering;
- reset reproduces the seeded reference sequence;
- reload changes future interaction coefficients without corrupting state;
- intensity never exceeds explicit `maxRate`;
- no buffer mutation occurs;
- malformed matrices cannot crash scsynth or supernova.

For a stable exponential Hawkes network, the integrated influence matrix is approximately:

```text
G_ij = alpha_ij / beta_ij
```

The guide/helpfile should explain that networks with excessive total feedback can spend most of their time near the explicit rate ceiling. Do not perform an expensive spectral-radius calculation on the audio thread.

---

## 21. Helpfile concept

The first paragraphs must explain:

> `MultiHawkesTrig` models event contagion between several streams. An event is not only an output: it changes the short-term probability of future events elsewhere in the network. This makes the UGen useful for cascades, call-and-response, swarms, and interacting rhythmic agents.

Explicitly contrast it with independent `HawkesTrig` instances.

---

## 22. Minimal helpfile example

Use a two-process buffer setup. Because buffers are necessary, a `Routine` and several variables are justified.

```supercollider
(
Routine {
    var base, excite, decay;
    base = Buffer.loadCollection(s, [1, 0.2]);
    excite = Buffer.loadCollection(s, [0, 0, 12, 0]);
    decay = Buffer.loadCollection(s, [10, 10, 10, 10]);
    s.sync;
    {
        var x = MultiHawkesTrig.ar(2, base, excite, decay);
        Decay2.ar(x.keep(2), 0.001, 0.05).sum * 0.1
    }.play;
}.play;
)
```

This example should make process 0 occasionally provoke process 1.

---

## 23. Creative helpfile example — propagating percussion

Create a four-process directional network in which activity tends to move through four voices rather than all streams firing independently.

Use different resonators or oscillator frequencies for the four trigger outputs.

The example should demonstrate **interaction and propagation**. Do not make it merely a four-channel random drum machine.

A larger block with descriptive variables is appropriate here.

---

## 24. Documentation visualization

Generate a static documentation figure containing:

- four horizontal event rasters;
- one or two representative intensity curves;
- visible directional cascades after source events.

Do not use an arbitrary decorative plot. The figure must reveal cross-excitation.

---

# Part VI — `SelfCorrectingTrig`

## 25. Concept

`SelfCorrectingTrig` is the conceptual opposite of a self-exciting point process.

Time without an event gradually increases event pressure. An event then suppresses that pressure.

The process therefore remains random while resisting clusters.

Musically, this is useful for:

- irregular but well-spaced rhythms;
- refractory event streams;
- probabilistic pulse trains that avoid bursts;
- sparse textures that naturally refill empty temporal regions;
- stochastic timing that does not collapse into metronomic periodicity.

---

## 26. Parameterization

Use a musically transparent normalized self-correcting form:

```text
lambda(t) = rate * exp(z(t))
```

Between events:

```text
dz/dt = correction * rate
```

At each event:

```text
z <- z - correction
```

For constant parameters this corresponds to:

```text
lambda(t) = rate * exp(correction * (rate * t - N(t)))
```

This parameterization makes `rate` the long-run target event density and keeps the API more meaningful than exposing the raw historical `mu` and `alpha` form.

With:

```text
correction = 0
```

the process becomes a constant-rate Poisson process.

---

## 27. Proposed API

```supercollider
SelfCorrectingTrig.ar(
    rate = 1,
    correction = 1,
    maxRate = 20000,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

```text
[ trigger, preEventIntensity ]
```

`rate`, `correction`, and `maxRate` are nonnegative.

`maxRate` is an explicit numerical and musical ceiling, not a hidden clamp.

---

## 28. Implementation

Store the correction state in log-intensity space.

Do not repeatedly multiply intensity directly, because long event-free intervals can overflow.

At each calculation:

1. advance `z` by `correction * rate * dt`;
2. compute intensity from `rate * exp(z)` using an overflow-safe path;
3. apply `maxRate`;
4. sample an event with the stable hazard probability;
5. if an event occurs, subtract `correction` from `z`;
6. output the intensity used to generate the event.

When `rate <= 0`, output no events and zero intensity.

When `correction <= 0`, behave as constant-rate Poisson.

If the explicit `maxRate` ceiling implies a maximum useful `z`, keep the log state within the equivalent documented ceiling rather than allowing useless overflow.

---

## 29. Tests

Required:

- `correction = 0` matches constant-rate Poisson statistics;
- long-run event rate approaches `rate` for moderate parameters;
- increasing correction reduces short inter-onset clustering;
- event-count variance over fixed windows becomes lower than Poisson in appropriate regimes;
- `rate = 0` remains silent;
- reset reproduces the deterministic seeded sequence;
- output intensity remains finite under extreme valid parameters.

---

## 30. Minimal helpfile example

```supercollider
{ Decay2.ar(SelfCorrectingTrig.ar(6, 1.5)[0], 0.001, 0.04) * 0.1 }.play
```

---

## 31. Creative example — inhibition versus excitation

The helpfile should contain one larger stereo comparison:

- left: `HawkesTrig` configured to cluster;
- right: `SelfCorrectingTrig` at a comparable average density.

Use the same short percussive sound on both sides so the timing organization is the perceptual difference.

The example should make this idea audible:

```text
self-excitation -> bursts
self-correction -> spacing
```

Do not hide the timing difference behind different timbres.

---

## 32. Visualization

Show event rasters and intensity trajectories for:

- a Poisson-like low-correction case;
- a stronger self-correcting case.

The visualization should reveal that events push intensity down and silence lets it rebuild.

---

# Part VII — `RandomTelegraph`

## 33. Concept

`RandomTelegraph` is a continuous-time two-state Markov process.

The output persists at one value, remains there for a random dwell time, then switches to the other value.

Unlike ordinary sample-and-hold noise, the process has a semantic state and separate transition rates in each direction.

This creates **regime switching**.

Possible uses include:

- active/resting behavior;
- bright/dark filter modes;
- switching between synthesis algorithms;
- probabilistic gates with memory;
- long/short asymmetry;
- random binary control that has meaningful dwell times.

---

## 34. Proposed API

```supercollider
RandomTelegraph.ar(
    low = -1,
    high = 1,
    upRate = 1,
    downRate = 1,
    initialState = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

```text
[ value, switchTrigger ]
```

`initialState <= 0` starts low.

`initialState > 0` starts high.

`upRate` is the low-to-high hazard in transitions per second.

`downRate` is the high-to-low hazard.

Zero rate makes the corresponding state absorbing.

---

## 35. Event scheduling

Do not generate a fresh Bernoulli choice on every sample with no timing state.

Use an exponential hazard threshold:

```text
H = -log(U)
```

Accumulate integrated hazard for the currently active state.

This supports changing `upRate` and `downRate` while preserving continuous-time semantics under the per-calculation piecewise-constant approximation.

When a threshold is crossed inside one calculation interval:

1. determine the fractional time of the switch;
2. switch state;
3. draw a new exponential threshold;
4. continue through the remaining fraction of the calculation interval using the new state's rate.

Use a fixed maximum internal transition count per calculation, for example:

```cpp
constexpr int kMaxTelegraphTransitionsPerCalc = 8;
```

If the limit is reached, preserve the remaining time/hazard state for the next calculation rather than looping further.

The switch output is `1` when one or more switches occurred in the calculation, otherwise `0`.

---

## 36. Tests

For constant rates:

```text
mean low dwell  = 1 / upRate
mean high dwell = 1 / downRate
```

Long-run high-state occupancy should approach:

```text
upRate / (upRate + downRate)
```

Also test:

- symmetric rates produce approximately equal occupancy;
- `upRate = 0` traps the low state;
- `downRate = 0` traps the high state after it is reached;
- reset restores initial state and seed;
- very high rates never cause an unbounded loop.

---

## 37. Minimal helpfile example

```supercollider
{ SinOsc.ar(RandomTelegraph.kr(220, 660, 1, 0.4)[0], 0, 0.08) }.play
```

This makes the persistent two-regime behavior immediately audible.

---

## 38. Creative example — stochastic form switching

Create one larger example in which `RandomTelegraph` switches a patch between two genuinely different regimes, for example:

- sparse resonant impulses and dense noise;
- low-pass and high-pass behavior;
- harmonic and inharmonic oscillator banks.

Use the telegraph state as the regime selector and its switch trigger to articulate the transition.

The point of the example is persistence: the system should remain in each condition long enough for the listener to perceive it as a state rather than as ordinary modulation.

---

## 39. Visualization

Show a time series with only two levels and visibly unequal dwell durations for asymmetric rates.

Annotate mean dwell time conceptually in the caption rather than cluttering the image.

---

# Part VIII — `ShotNoise`

## 40. Concept

`ShotNoise` converts random events into a continuous stochastic signal by adding a decaying response for every arrival.

Each event is a shot. Many overlapping shots create the process.

This is different from ordinary noise because its texture can be understood in terms of discrete physical-like events plus memory.

At low rate:

```text
isolated impacts
```

At moderate rate:

```text
crackle / droplets / particles
```

At high rate:

```text
dense fluctuating texture
```

---

## 41. Mathematical model

Use exponentially decaying shots:

```text
x(t) = sum_k A_k * exp(-(t - t_k) / decay),    t >= t_k
```

Event times `t_k` are Poisson arrivals.

Marks `A_k` use a Gaussian distribution:

```text
A_k ~ Normal(ampMean, ampStd^2)
```

This gives a compact but meaningful marked shot-noise model.

With `ampStd = 0`, all events have the same amplitude.

---

## 42. Proposed API

```supercollider
ShotNoise.ar(
    rate = 10,
    decay = 0.05,
    ampMean = 0,
    ampStd = 1,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

```text
[ signal, eventTrigger ]
```

`rate` is events per second.

`decay` is the exponential time constant in seconds.

`ampStd` uses its absolute magnitude.

The output is intentionally unbounded.

---

## 43. Scheduling and accumulation

Use exponential event scheduling rather than testing a uniform random number for every sample when no event is near.

The implementation should:

1. maintain remaining integrated Poisson hazard/time to the next event;
2. analytically decay the current state over event-free time;
3. when an event occurs inside a calculation interval, decay to the event time;
4. add the Gaussian mark;
5. continue through the remainder of the interval;
6. support a small fixed maximum number of arrivals inside one calculation.

Suggested bound:

```cpp
constexpr int kMaxShotEventsPerCalc = 8;
```

If the limit is reached, carry remaining event timing state into the next calculation.

The event output is binary: `1` if one or more arrivals occurred in the calculation.

For musically interpretable individual shots, document that event rates should normally remain far below the calculation rate.

---

## 44. Decay edge cases

For `decay > 0`, use the analytical exponential multiplier.

For `decay <= 0`, define the process as an instantaneous shot held only for the current calculation and return to zero before the next calculation.

Do not divide by zero or silently invent a long time constant.

---

## 45. Tests

For stationary exponential shot noise with Poisson rate `lambda` and marks `A`:

```text
E[x]   = lambda * E[A] * decay
Var[x] = lambda * E[A^2] * decay / 2
```

Use these relations for long statistical renders when parameters are in a numerically well-behaved range.

Also test:

- `rate = 0` produces zero after existing state decays;
- `ampStd = 0` produces fixed-size marks;
- event timing is deterministic with explicit seed;
- reset clears accumulated signal and event schedule;
- nonpositive decay follows the documented instantaneous behavior;
- no high-rate input can create an unbounded event loop.

---

## 46. Minimal helpfile example

```supercollider
{ ShotNoise.ar(12, 0.03, 0, 0.5)[0] * 0.1 }.play
```

---

## 47. Creative example — particle excitation

Use `ShotNoise` as the excitation source for a resonant structure such as `Ringz`, `DynKlank`, or a small filter bank.

The example should expose the continuum:

```text
separate particles -> overlapping texture
```

A slowly moving event rate is appropriate if it clearly reveals the transition from isolated impacts to dense material.

---

## 48. Visualization

Show:

- event ticks;
- individual exponential shots;
- their summed signal.

The image should visually explain why shot noise is simultaneously event-based and continuous.

---

# Part IX — `MultiOUProcess`

## 49. Concept

`MultiOUProcess` generates several mean-reverting stochastic trajectories whose random innovations are correlated.

It combines two forms of structure:

1. **temporal structure** — every channel remembers its previous state and is pulled toward a mean;
2. **cross-channel structure** — random motion in one channel can be statistically related to random motion in another.

This is useful for:

- cohesive ensemble drift;
- coordinated spatial motion;
- related modulation of several synthesis parameters;
- populations that move together without becoming identical;
- multivariate control signals that possess both memory and covariance.

---

## 50. Mathematical model

Use the multivariate SDE:

```text
dX = -Theta (X - mu) dt + B dW
```

For the first implementation, `Theta` is diagonal:

```text
Theta = diag(theta_0 ... theta_N-1)
```

The user provides diffusion covariance:

```text
Sigma = B B^T
```

For a finite calculation interval `dt`, each component has deterministic decay:

```text
E_i = exp(-theta_i * dt)
```

The exact innovation covariance is:

```text
Q_ij = Sigma_ij * (1 - exp(-(theta_i + theta_j) * dt)) / (theta_i + theta_j)
```

when:

```text
theta_i + theta_j > 0
```

and:

```text
Q_ij = Sigma_ij * dt
```

for the Brownian limit.

The exact update is:

```text
X_next = mu + E * (X - mu) + L z
```

where:

```text
L L^T = Q
```

and `z` is a vector of independent standard normal values.

---

## 51. Proposed API

```supercollider
MultiOUProcess.ar(
    numChannels = 2,
    covarianceBuf = 0,
    meanBuf = -1,
    reversionBuf = -1,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Outputs

Exactly `numChannels` process values.

### Buffer defaults

If `meanBuf < 0`:

```text
mu_i = 0
```

If `reversionBuf < 0`:

```text
theta_i = 1
```

The covariance buffer is required.

---

## 52. Buffer formats

### Covariance

One channel, row-major `N * N` diffusion covariance matrix.

Reuse the same symmetrization, jitter, Cholesky, and fallback strategy already used by `MultiGaussNoise` where mathematically appropriate.

### Mean

One channel, at least `N` samples.

### Reversion

One channel, at least `N` samples.

Negative reversion values become zero.

---

## 53. Factorization policy

Because `dt` is fixed for a particular UGen rate and server configuration, compute `E`, `Q`, and the Cholesky factor at:

- construction;
- explicit reload.

Do not factor a matrix per sample.

`reload` snapshots current buffers into RT-owned state.

Changing a Buffer without sending `reload` need not alter the process.

---

## 54. Reset

Reset must:

- restore RNG state;
- restore the process state to the loaded mean vector;
- keep the currently loaded matrix parameters;
- output the mean vector on the reset calculation.

Do not draw a random stationary sample on reset unless the API is explicitly changed before implementation. Deterministic mean reset is easier to reason about and test.

---

## 55. Limits

Use the existing `MultiGaussNoise` maximum dimension if practical.

If no shared constant exists, set:

```text
1 <= numChannels <= 32
```

Profile audio-rate operation at high dimensions.

Do not reduce the dimension arbitrarily without a benchmark demonstrating the need.

---

## 56. Tests

Required:

- `N = 1` agrees statistically with scalar `OUProcess` under equivalent parameters;
- zero reversion produces the correlated Brownian finite-step limit;
- with positive reversion, empirical mean approaches `mu`;
- stationary covariance approaches:

```text
C_ij = Sigma_ij / (theta_i + theta_j)
```

for constant parameters;

- cross-correlation sign and relative magnitude follow the covariance matrix;
- reset returns exactly to the mean vector;
- reload changes future covariance without reallocating in the calculation function;
- malformed buffers use the documented safe fallback;
- seeded reference vectors are reproducible.

---

## 57. Minimal helpfile example

Because this class requires a covariance buffer, use a compact `Routine`.

```supercollider
(
Routine {
    b = Buffer.loadCollection(s, [1, 0.8, 0.8, 1]);
    s.sync;
    { MultiOUProcess.kr(2, b).poll }.play;
}.play;
)
```

---

## 58. Creative example — ensemble breathing

Create four oscillators whose pitch, filter, or amplitude drift is driven by a four-dimensional `MultiOUProcess`.

Use a covariance matrix with strong positive off-diagonal values but not perfect correlation.

The musical result should make the conceptual point:

> the voices move separately, but they belong to the same stochastic weather system.

Do not replace the correlated vector with four independent `OUProcess` instances in the example.

---

## 59. Visualization

For a two-dimensional configuration, show a phase-plane trajectory plus marginal time series.

The trajectory should reveal both:

- temporal continuity;
- covariance geometry.

A static scatter cloud alone is insufficient because that would look like `MultiGaussNoise`.

---

# Part X — `DoubleWellProcess`

## 60. Concept

`DoubleWellProcess` is a stochastic differential system with two preferred regions separated by a nonlinear barrier.

It introduces **metastability**.

The process may fluctuate near one stable well for a long period, then a stochastic fluctuation can push it into the other well.

This creates behavior fundamentally different from mean-reverting noise:

```text
OUProcess          one preferred region
DoubleWellProcess  two competing preferred regions
```

The UGen should also make stochastic resonance easy to explore: a weak periodic force that cannot cross the barrier alone may become effective when an appropriate amount of noise is present.

---

## 61. Mathematical model

Use:

```text
dx = (a*x - b*x^3 + drive) dt + noise dW
```

This corresponds to the potential:

```text
V(x) = b*x^4/4 - a*x^2/2 - drive*x
```

For:

```text
a > 0
b > 0
```

and zero drive, stable wells occur near:

```text
x = +/- sqrt(a / b)
```

with the origin separating them.

Do not force `a` or `b` positive. Values outside the double-well regime may intentionally produce other nonlinear dynamics. The helpfile should explain that the class name refers to the common positive-parameter regime.

---

## 62. Proposed API

```supercollider
DoubleWellProcess.ar(
    drive = 0,
    a = 1,
    b = 1,
    noise = 0.35,
    timeScale = 1,
    initial = -1,
    substeps = 1,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

`substeps` is init-rate and clipped to a documented small range such as `1..16`.

### Output

One raw process state.

Do not additionally output a hidden binary well classification. A user can derive the sign or any other decision rule from the raw state.

---

## 63. Numerical method

Use Euler-Maruyama with additive noise and fixed substeps.

For each substep:

```text
dtSub = calculationDuration * timeScale / substeps
x <- x + (a*x - b*x^3 + drive) * dtSub
       + abs(noise) * sqrt(dtSub) * Normal(0, 1)
```

If `timeScale <= 0`, hold the state.

Sample the current `drive`, `a`, `b`, `noise`, and `timeScale` values at the calculation and hold them across its internal substeps.

If the state becomes non-finite, restore `initial` and restore a usable finite process state. Do not clip the nonlinear trajectory into an arbitrary audio range.

---

## 64. Tests

Required:

- with `noise = 0`, `drive = 0`, positive `a,b`, trajectories near each stable fixed point remain there;
- deterministic fixed points agree with `+/-sqrt(a/b)`;
- origin is unstable in the canonical regime;
- moderate noise produces occasional sign changes over long renders;
- very low noise produces fewer transitions than moderate noise;
- reset restores initial state and seed;
- increasing substeps improves agreement with an offline high-resolution reference;
- extreme valid parameters cannot create NaN propagation or an unbounded loop.

Stochastic resonance should be demonstrated qualitatively in documentation and optionally in an offline analysis test, but do not make a fragile resonance metric a merge-blocking unit test.

---

## 65. Minimal helpfile example

```supercollider
{ DoubleWellProcess.kr(noise: 0.4).poll }.play
```

---

## 66. Creative example — metastable timbre

Map the process to two clearly different spectral regions while preserving continuous motion inside each region.

The result should alternate between long-lived timbral states with irregular transitions.

Do not convert the process immediately to `sign` in the only creative example; doing so would hide the within-well fluctuation that distinguishes a metastable process from a binary switch.

---

## 67. Creative example — stochastic resonance

Include one larger comparison in which the same weak periodic drive feeds two instances:

- one with no or extremely little noise;
- one with a moderate noise level.

Place the results in stereo or otherwise make them directly comparable.

The periodic drive must be intentionally too weak to cause regular barrier crossings in the low-noise system.

The moderate-noise side should show more drive-related switching without implying perfect synchronization.

Comments explaining the comparison may appear inside the parenthesized block.

---

## 68. Visualization

The documentation figure should show:

- the double-well potential curve;
- the current stochastic trajectory or representative state positions;
- several noise-induced transitions between wells.

A second panel may show switching under a weak periodic tilt.

---

# Part XI — `TVonMisesRand`

## 69. Concept

`TVonMisesRand` samples a probability distribution on a circle.

It solves a problem that ordinary Gaussian randomness does not solve correctly: angles wrap.

For example:

```text
-pi and +pi are neighboring directions
```

not opposite extremes of an infinite line.

The von Mises distribution therefore belongs naturally to:

- oscillator phase;
- azimuth;
- cyclic sequencing;
- rotation;
- spatial direction;
- any parameter whose endpoints are identical.

---

## 70. Distribution

Use the von Mises density:

```text
p(theta) proportional to exp(kappa * cos(theta - mean))
```

where:

```text
mean  = preferred angle in radians
kappa = concentration >= 0
```

At:

```text
kappa = 0
```

the distribution is uniform around the circle.

As `kappa` increases, samples cluster more strongly around the mean direction.

Return all angles in:

```text
[-pi, pi)
```

---

## 71. Proposed API

```supercollider
TVonMisesRand.ar(
    mean = 0,
    kappa = 1,
    trig = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

Follow the existing triggered-distribution constructor/reset convention in Stochastica.

---

## 72. Sampling algorithm

Use a robust bounded implementation of the Best-Fisher rejection method for ordinary `kappa` values.

Requirements:

- a compile-time maximum rejection count;
- `kappa <= epsilon` uses a uniform circular sample;
- negative `kappa` becomes zero;
- mean is wrapped before output;
- extremely large `kappa` must avoid overflow and catastrophic cancellation.

For very large concentration, a wrapped-normal asymptotic approximation is acceptable if:

- the threshold is explicit in source;
- the approximation is documented in `DISCUSSION::`;
- tests verify continuity of behavior around the threshold.

A bounded rejection failure should fall back to the wrapped mean angle.

---

## 73. Tests

Required:

- all outputs are in `[-pi, pi)`;
- `kappa = 0` is circularly uniform;
- circular sample mean approaches the requested mean;
- mean directions close to `pi` and `-pi` behave continuously across the wrap;
- mean resultant length increases with `kappa`;
- empirical resultant length agrees with the theoretical `I1(kappa) / I0(kappa)` within statistical tolerance for selected moderate values;
- reset reproduces the seeded sequence;
- large `kappa` remains finite.

Statistical analysis may use Python/NumPy/SciPy outside the audio thread and outside runtime dependencies.

---

## 74. Minimal helpfile example

```supercollider
{ TVonMisesRand.kr(0, 6, Impulse.kr(4)).poll }.play
```

---

## 75. Creative example — circular spatial concentration

Use `TVonMisesRand` to place short events around a multichannel ring with `PanAz`.

Map:

```text
angle / pi
```

to the `PanAz` position convention used by the example.

The listener should hear events concentrated around one region while still occasionally appearing elsewhere.

A second slowly moving mean angle may demonstrate a moving preferred direction, but keep the random angle itself circular rather than linearly clipped.

---

## 76. Visualization

Use a circular plot or polar histogram.

Show at least:

- `kappa = 0`;
- moderate concentration;
- strong concentration.

A linear histogram from `-pi` to `pi` is insufficient as the only visualization because it visually separates the wrap boundary.

---

# Part XII — `KuramotoNetwork`

## 77. Concept

`KuramotoNetwork` models collective synchronization among phase oscillators.

Each oscillator has its own natural frequency. Coupling encourages phases to align, but no central clock explicitly forces them into synchrony.

The important behavior is the transition between:

- incoherence;
- partial synchronization;
- strong collective synchronization.

This adds an emergent population-level process to Stochastica.

It is useful for:

- rhythm populations;
- coupled LFOs;
- distributed phase modulation;
- ensemble coordination;
- self-organizing polyrhythms;
- transitions between disorder and coherence.

---

## 78. Mathematical model

Use the all-to-all homogeneous Kuramoto model:

```text
dtheta_i/dt = omega_i + K/N * sum_j sin(theta_j - theta_i)
```

Compute the equivalent mean-field order parameter:

```text
r * exp(i*psi) = (1/N) * sum_j exp(i*theta_j)
```

Then:

```text
dtheta_i/dt = omega_i + K * r * sin(psi - theta_i)
```

This reduces each substep from `O(N^2)` to `O(N)`.

Optionally add phase diffusion:

```text
+ noise * dW_i
```

This makes stochastic desynchronization available without requiring another class.

---

## 79. Proposed API

```supercollider
KuramotoNetwork.ar(
    numOscillators = 8,
    frequencyBuf = 0,
    phaseBuf = -1,
    coupling = 0,
    noise = 0,
    timeScale = 1,
    substeps = 1,
    reload = 0,
    seed = 0,
    reset = 0
)
```

Provide `.kr`.

### Buffer units

`frequencyBuf` contains natural frequencies in cycles per model second.

Convert internally:

```text
omega_i = 2*pi*frequency_i
```

`phaseBuf`, when present, contains initial phases in radians.

`coupling` is an angular coupling rate in radians per model second.

`noise` is phase diffusion in radians per square-root model second.

`timeScale` is model seconds per real second.

---

## 80. Outputs

Return exactly:

```text
N phases, synchronization order r, mean phase psi
```

as:

```text
[ theta_0 ... theta_N-1, r, psi ]
```

Wrap each phase and `psi` to:

```text
[-pi, pi)
```

`r` is in:

```text
[0, 1]
```

This ordering must be prominently documented.

---

## 81. Initial phases

If `phaseBuf >= 0` and valid:

- snapshot the first `N` values at construction/reload;
- wrap them into `[-pi, pi)`;
- use them as the reset phases.

If `phaseBuf < 0`:

- generate deterministic uniform phases from the local seed;
- store them as the reset phases.

Reset restores the same initial phase configuration and RNG state.

Do not generate a different random initial network on every reset when the seed is unchanged.

---

## 82. Numerical integration

Use bounded fixed substeps.

For each substep:

1. calculate `cos(theta_i)` and `sin(theta_i)` sums;
2. derive `r` and `psi`;
3. compute each oscillator drift;
4. add independent Gaussian phase diffusion when `noise > 0`;
5. advance all phases simultaneously;
6. wrap phases.

Euler-Maruyama is acceptable for the stochastic version if documented and validated.

For `noise = 0`, a simple deterministic midpoint improvement may be considered only if it can share the same bounded structure cleanly. Do not make the first implementation needlessly complicated.

Suggested limits:

```text
1 <= numOscillators <= 64
1 <= substeps <= 16
```

The all-to-all mean-field algorithm must remain `O(N)` per substep.

---

## 83. Reload

Reload must snapshot current frequency and optional phase buffers.

Reloading frequencies changes future dynamics but must not reset current phases.

Reloading a phase buffer updates the stored reset configuration but must not teleport the current network.

Only reset applies the stored initial phases.

---

## 84. Tests

Required:

- `coupling = 0`, `noise = 0`: each phase advances according to its natural frequency;
- identical frequencies and identical initial phases preserve `r = 1`;
- `r` always remains within `[0,1]` to numeric tolerance;
- a uniformly spread phase configuration has low `r`;
- suitable positive coupling increases long-run order for a narrow frequency distribution;
- phase wrapping is continuous;
- `noise > 0` broadens phase dispersion statistically;
- reset restores the exact initial phase vector;
- reload changes frequencies without resetting current phase;
- high `N` and maximum substeps remain bounded in CPU and memory.

Do not require every random initial network to synchronize at the same coupling. Use controlled frequency and phase buffers for deterministic synchronization tests.

---

## 85. Minimal helpfile example

Because frequencies require a Buffer, use a compact `Routine`.

```supercollider
(
Routine {
    b = Buffer.loadCollection(s, [0.9, 0.95, 1, 1.05, 1.1]);
    s.sync;
    { KuramotoNetwork.kr(5, b, coupling: 3)[5].poll }.play;
}.play;
)
```

The selected output is the order parameter `r` because the first five outputs are phases.

---

## 86. Creative example — audible emergence of synchrony

Create a network of 8–16 oscillators.

Use each phase to control an audible cyclic process rather than merely converting `r` to pitch.

For example:

- each phase may gate or amplitude-modulate one voice;
- natural frequencies should be close but not identical;
- coupling should move slowly from weak to strong and back;
- the order parameter may control a secondary property such as spectral brightness.

The listener should hear the population move from scattered independent motion toward coordinated pulses, then loosen again.

This example may use many variables because it is intentionally a system-level demonstration.

---

## 87. Visualization

The strongest documentation visualization is a phase circle.

Show oscillator phases as points around a unit circle and provide the order vector:

```text
length = r
angle  = psi
```

A second panel or animation frame may compare weak and strong coupling.

If a live visualization is added, keep all GUI storage and drawing in the language process and transfer state only at display rate. Never move GUI logic onto the audio server.

---

# Part XIII — Cross-process creative examples for the general guide

## 88. General rule

The general guide should not become a gallery of twenty patches.

Add a small number of larger examples that combine processes for conceptual reasons.

Each large example must have a sentence explaining the stochastic idea before the code.

Comments inside code are allowed only within the parenthesized block.

---

## 89. Interaction versus inhibition

Create one example that places comparable sounds in stereo:

```text
left  = event clustering / contagion
right = event spacing / self-correction
```

Use `HawkesTrig` or one channel of `MultiHawkesTrig` on the left and `SelfCorrectingTrig` on the right.

Keep average event density reasonably similar so that the difference is temporal organization, not simply event count.

---

## 90. Regime switching plus metastability

Create an example in which `RandomTelegraph` changes a slow bias applied to `DoubleWellProcess`.

The telegraph process should alter which well is favored, while the double-well noise determines whether and when the state actually crosses the barrier.

This demonstrates a useful distinction:

```text
RandomTelegraph     explicit discrete regime
DoubleWellProcess   continuous metastable state with stochastic transitions
```

Do not reduce both to the same binary output.

---

## 91. Correlated motion plus circular statistics

Use `MultiOUProcess` to create slowly related motion for several voices while `TVonMisesRand` determines the circular position of transient events around them.

The example should contrast:

- continuous correlated drift;
- independent circular event placement around a preferred direction.

This is a good multichannel example and may use `PanAz`.

---

## 92. Interaction plus synchronization

Create one advanced example in which event activity from `MultiHawkesTrig` changes `KuramotoNetwork` coupling.

A cascade can temporarily increase synchronization; quiet periods allow the network to become less coherent.

This example explores two different meanings of interaction:

```text
MultiHawkesTrig   interaction among event probabilities
KuramotoNetwork   interaction among oscillator phases
```

Keep this example in the general guide rather than duplicating it in either individual helpfile.

---

## 93. Stochastic impulse material

Use `ShotNoise` to excite a resonant system while `SelfCorrectingTrig` slowly changes a higher-level parameter such as resonator set or density regime.

The purpose is to show that Stochastica processes can organize both:

- microscopic material events;
- macroscopic temporal form.

---

# Part XIV — Individual helpfile requirements

## 94. Helpfile structure

Follow the repository's existing `.schelp` convention.

Each new class page must contain, in the current project order:

```text
class::
summary::
categories::
related::

description::

subsection:: conceptual or mathematical visualization

classmethods::
method:: ar
arguments...
method:: kr

instancemethods::

examples::
```

Do not introduce a different documentation grammar just for this round.

---

## 95. Required conceptual explanation

Every helpfile must answer these questions before diving into implementation detail:

1. What kind of stochastic or dynamical object is this?
2. What behavior does it create that ordinary independent noise does not?
3. What is the intuitive meaning of its main parameter(s)?
4. What musical or generative situations naturally fit this behavior?
5. What existing Stochastica class is most useful to compare it with?

Avoid empty phrases such as:

```text
creates interesting randomness
controls chaos
changes randomness amount
```

Use precise concepts such as:

```text
cross-excitation
refractory spacing
mean dwell time
impulse accumulation
cross-covariance
barrier crossing
circular concentration
phase coherence
```

---

## 96. Example count

Recommended:

- `SelfCorrectingTrig`: 3 examples;
- `RandomTelegraph`: 3 examples;
- `ShotNoise`: 3 examples;
- `TVonMisesRand`: 3 examples;
- `MultiHawkesTrig`: 2 buffer examples plus 1 larger musical example;
- `MultiOUProcess`: 2 buffer examples plus 1 larger musical example;
- `DoubleWellProcess`: 3 or 4 examples because stochastic resonance deserves a comparison;
- `KuramotoNetwork`: 2 buffer examples plus 1 larger system example.

Do not exceed five examples per page without a concrete reason.

---

# Part XV — Documentation images and analysis scripts

## 97. Use mathematically meaningful visuals

The current Stochastica documentation already uses visual explanation. Continue that practice for processes where a figure materially improves understanding.

Suggested figures:

| Class | Figure |
|---|---|
| `MultiHawkesTrig` | multichannel raster plus selected intensities |
| `SelfCorrectingTrig` | event raster plus intensity suppression/recovery |
| `RandomTelegraph` | two-level state timeline with dwell durations |
| `ShotNoise` | event ticks, individual decays, summed process |
| `MultiOUProcess` | 2-D continuous trajectory plus time series |
| `DoubleWellProcess` | double-well potential plus stochastic switching trajectory |
| `TVonMisesRand` | polar sample histograms for several `kappa` values |
| `KuramotoNetwork` | phase circle and order vector for weak/strong coupling |

Keep analysis/figure-generation scripts outside the audio plugin runtime.

Do not add Python, SciPy, or plotting libraries as runtime dependencies.

---

# Part XVI — Testing strategy

## 98. Four test layers

Use the same four layers as the first implementation document.

### 98.1 Pure C++ reference tests

Test:

- circular wrapping;
- von Mises sampler;
- event hazard calculations;
- telegraph transition logic;
- shot decay logic;
- multivariate OU covariance construction;
- double-well integrator;
- Kuramoto order parameter;
- Hawkes matrix state update.

### 98.2 Server integration tests

For every class:

- plugin loads;
- `.ar` and `.kr` construct when specified;
- output count is correct;
- reset works;
- explicit seed is reproducible;
- node freeing is safe;
- repeated create/free cycles leak no RT memory;
- malformed buffers fail safely;
- scsynth and supernova both run the class.

### 98.3 Statistical tests

Use deterministic offline renders and external analysis.

Do not run SciPy or heavy statistical analysis on the audio thread.

Test statistics described in the individual UGen sections.

### 98.4 Robustness/performance tests

Render long runs with:

- maximum documented process count/dimension;
- dense event rates;
- extreme but valid parameters;
- repeated resets;
- repeated reloads;
- missing or too-small buffers;
- maximum substeps;
- audio-rate modulation of permitted scalar inputs.

Monitor:

- non-finite outputs;
- deadline spikes;
- server crashes;
- allocations after construction;
- leaks;
- runaway event loops.

---

# Part XVII — Performance budgets

## 99. `MultiHawkesTrig`

Complexity:

```text
O(N^2) per calculation
```

First-version maximum:

```text
N = 16
```

Benchmark both `.ar` and `.kr`.

---

## 100. `MultiOUProcess`

Per calculation:

```text
O(N^2)
```

Matrix factorization occurs only on construction/reload.

Target maximum:

```text
N = 32
```

subject to existing project limits and benchmark results.

---

## 101. `KuramotoNetwork`

Use mean-field reduction:

```text
O(N * substeps)
```

not naive:

```text
O(N^2 * substeps)
```

Target maximum:

```text
N = 64
```

---

## 102. Event catch-up loops

`RandomTelegraph` and `ShotNoise` must have a fixed small maximum number of internal events/transitions per calculation.

The performance test must explicitly stress rates above normal musical use and verify that CPU time remains bounded.

---

# Part XVIII — Source layout

## 103. Recommended file placement

Fit the implementation into the current repository rather than creating a new top-level module.

Suggested additions:

```text
processes/
    AdvancedEventProcesses.cpp
    ShotNoise.cpp
    MultiOUProcess.cpp
    DoubleWellProcess.cpp

distributions/
    CircularDistributions.cpp

flows/
    KuramotoNetwork.cpp
```

If the existing source organization uses one class per file or a different grouping convention, follow the repository convention instead of these exact filenames.

The conceptual placement should remain:

```text
MultiHawkesTrig       processes
SelfCorrectingTrig   processes
RandomTelegraph      processes
ShotNoise            processes
MultiOUProcess       processes or state, following existing vector-process convention
DoubleWellProcess    processes
TVonMisesRand        distributions
KuramotoNetwork      flows / collective continuous systems
```

---

## 104. Language classes

Add the new classes to the same `Classes` organization used by current Stochastica classes.

Use `MultiOutUGen` where output count exceeds one.

Output counts must be fixed when the SynthDef is built.

Init-rate arguments include at least:

- `numProcesses` for `MultiHawkesTrig`;
- `numChannels` for `MultiOUProcess`;
- `numOscillators` and `substeps` for `KuramotoNetwork`;
- `substeps` for `DoubleWellProcess`.

Do not allow a running Synth to change output dimension.

---

# Part XIX — Suggested implementation order

## 105. Milestone 0 — repository audit

Before writing code:

1. build current main branch;
2. run existing tests;
3. inspect current stochastic helper APIs;
4. record existing compiler warnings without changing them;
5. identify exact registration and class-file conventions;
6. identify current documentation image-generation workflow;
7. create a short implementation checklist mapping each new class to source, class, help, test, and guide files.

Acceptance condition: no source edits before the current structure is understood.

---

## 106. Milestone 1 — scalar event/process foundations

Implement:

1. `SelfCorrectingTrig`;
2. `RandomTelegraph`;
3. `ShotNoise`.

Why first:

- they establish reusable hazard scheduling;
- they have small state;
- they make event-process testing mature before multivariate Hawkes work.

Each class must arrive with helpfile and tests immediately.

---

## 107. Milestone 2 — circular distribution

Implement:

```text
TVonMisesRand
```

Acceptance:

- bounded rejection;
- circular statistics pass;
- wrap behavior is correct;
- spatial example runs;
- polar documentation figure is generated.

---

## 108. Milestone 3 — correlated continuous motion

Implement:

```text
MultiOUProcess
```

Reuse covariance factorization from `MultiGaussNoise`.

Acceptance:

- exact finite-step covariance construction verified;
- scalar equivalence test passes;
- buffer reload is safe;
- correlated ensemble example runs.

---

## 109. Milestone 4 — interacting event network

Implement:

```text
MultiHawkesTrig
```

Do this only after scalar hazard/event infrastructure and buffer-backed matrix tests are established.

Acceptance:

- directional interaction test passes;
- simultaneous-event semantics are order independent;
- four-stream propagation example runs;
- raster visualization clearly demonstrates cascades.

---

## 110. Milestone 5 — nonlinear stochastic potential

Implement:

```text
DoubleWellProcess
```

Acceptance:

- deterministic fixed-point behavior verified;
- stochastic transitions demonstrated;
- stochastic-resonance example runs;
- no hidden state clipping.

---

## 111. Milestone 6 — collective synchronization

Implement:

```text
KuramotoNetwork
```

Do this last among the algorithms because it combines:

- multichannel output;
- buffers;
- numerical integration;
- optional stochastic diffusion;
- collective statistics;
- larger CPU surface.

Acceptance:

- `O(N)` mean-field implementation;
- deterministic synchronization tests;
- order parameter tests;
- large audible synchronization example;
- phase-circle figure.

---

## 112. Milestone 7 — general guide conceptual pass

Update `HelpSource/Guides/Stochastica.schelp` only after all eight individual class helpfiles exist.

Required final pass:

- add all new class links;
- add `Forms of stochastic organization`;
- explain all eight concepts;
- add selected cross-process examples;
- verify every guide code block follows the documentation comment rule;
- remove no existing guide content unless duplication becomes exact and unnecessary;
- verify every class link resolves in SCDoc.

---

# Part XX — Pull-request strategy

## 113. Keep PRs conceptually reviewable

Suggested sequence:

### PR 1

```text
Stochastica: add inhibitory and switching event processes
```

Contains:

- `SelfCorrectingTrig`;
- `RandomTelegraph`;
- shared bounded hazard helper if needed;
- tests;
- helpfiles.

### PR 2

```text
Stochastica: add marked exponential ShotNoise process
```

Contains:

- `ShotNoise`;
- tests;
- helpfile;
- visualization script/asset.

### PR 3

```text
Stochastica: add von Mises circular distribution
```

Contains:

- `TVonMisesRand`;
- circular statistical tests;
- helpfile;
- polar visualization.

### PR 4

```text
Stochastica: add multivariate Ornstein-Uhlenbeck process
```

Contains:

- `MultiOUProcess`;
- covariance tests;
- helpfile;
- correlated-motion visualization.

### PR 5

```text
Stochastica: add interacting multivariate Hawkes process
```

Contains:

- `MultiHawkesTrig`;
- matrix-buffer tests;
- helpfile;
- raster visualization.

### PR 6

```text
Stochastica: add stochastic double-well process
```

Contains:

- `DoubleWellProcess`;
- numerical/statistical tests;
- helpfile;
- potential visualization.

### PR 7

```text
Stochastica: add Kuramoto oscillator network
```

Contains:

- `KuramotoNetwork`;
- network tests;
- helpfile;
- phase visualization.

### PR 8

```text
Stochastica: expand guide for stochastic organization
```

Contains only:

- general guide conceptual integration;
- cross-process examples;
- class table updates;
- any guide-specific images.

Do not combine unrelated bug fixes with these PRs.

---

# Part XXI — Definition of done

## 114. Per-class checklist

A new UGen is complete only when:

- [ ] mathematical definition is written before implementation;
- [ ] server implementation exists;
- [ ] every member is initialized;
- [ ] no RT allocation occurs in calculation functions;
- [ ] every loop has a fixed maximum;
- [ ] reset semantics are deterministic;
- [ ] explicit seed reproduces a reference sequence;
- [ ] malformed inputs cannot crash or hang the server;
- [ ] output count is fixed and documented;
- [ ] `.ar` and `.kr` behavior is documented where both exist;
- [ ] language class exists;
- [ ] helpfile exists;
- [ ] first example is minimal;
- [ ] no runnable documentation line is commented out;
- [ ] comments in runnable examples appear only inside `(...)` blocks;
- [ ] conceptual creative example exists;
- [ ] pure reference tests pass;
- [ ] server integration tests pass;
- [ ] statistical tests pass where applicable;
- [ ] long robustness render passes;
- [ ] scsynth build passes;
- [ ] supernova build passes;
- [ ] Linux CI passes;
- [ ] macOS CI passes;
- [ ] Windows CI passes.

---

## 115. Round-level acceptance criteria

The round is complete only when all of the following are true:

1. Existing Stochastica UGen behavior is unchanged.
2. `MultiHawkesTrig` demonstrates directional cross-excitation.
3. `SelfCorrectingTrig` demonstrates anti-clustering rather than merely lower event rate.
4. `RandomTelegraph` has statistically correct asymmetric dwell behavior.
5. `ShotNoise` clearly connects discrete arrivals to a continuous accumulated signal.
6. `MultiOUProcess` exhibits both temporal memory and cross-channel covariance.
7. `DoubleWellProcess` exhibits metastable well occupancy and noise-driven transitions.
8. `TVonMisesRand` treats angles as circular data with correct wrap behavior.
9. `KuramotoNetwork` exhibits emergent synchronization measured by a valid order parameter.
10. Every class has a conceptual explanation, not merely an equation and argument list.
11. Every class has at least one example that audibly or visibly demonstrates its defining concept.
12. The general Stochastica guide explains interaction, inhibition, regime switching, stochastic impulses, correlated motion, metastability/stochastic resonance, circular statistics, and collective synchronization.
13. Documentation examples preserve the strict minimalism and comment rules from the first implementation document.
14. No unrelated bug-fix commit is included.

---

# Part XXII — Future ideas explicitly out of scope

## 116. Do not implement these in this round

Keep these as candidates for a later expansion:

- `FitzHughNagumo`;
- `HindmarshRose`;
- `WilsonCowan`;
- `Lorenz96`;
- `LevyFlight`;
- `JumpDiffusion`;
- `CIRProcess`;
- `TStudentTRand`;
- `TDirichletRand`;
- `PolyaUrn`;
- `HiddenMarkov`;
- `MultiTelegraph`;
- `TVonMisesFisherRand`.

Do not allow implementation convenience to expand the scope into these classes during this round.

---

# Part XXIII — Implementation references

## 117. Mathematical references

Use the original or standard literature to verify equations and sampling algorithms before coding.

### Hawkes processes

A. G. Hawkes, *Spectra of Some Self-Exciting and Mutually Exciting Point Processes*, Biometrika 58(1), 1971.

`https://doi.org/10.1093/biomet/58.1.83`

### Self-correcting point process

V. Isham and M. Westcott, *A self-correcting point process*, Stochastic Processes and their Applications 8(3), 1979.

`https://doi.org/10.1016/0304-4149(79)90008-5`

### Von Mises sampling

D. J. Best and N. I. Fisher, *Efficient Simulation of the von Mises Distribution*, Applied Statistics 28(2), 1979.

Verify the exact algorithm from a reliable copy before implementation rather than reconstructing it from secondary pseudocode.

### Kuramoto synchronization

Use a standard Kuramoto reference and verify that the mean-field reduction is mathematically equivalent to homogeneous all-to-all coupling.

### Stochastic resonance

Use standard double-well stochastic-resonance literature to verify terminology and the qualitative documentation example. Do not claim that one chosen parameter set is universally optimal.

---

## 118. SuperCollider references

Before implementation, consult the current versions matching the target SuperCollider release:

- `Writing Unit Generators`;
- Server Plugin API documentation;
- current `MultiOutUGen` implementations;
- buffer-locking examples;
- `sc3-plugins` and Stochastica CMake conventions;
- scsynth and supernova plugin registration patterns.

The current repository is authoritative for local style.

---

# Part XXIV — Final instruction to the implementation agent

## 119. Build behaviors, not a catalogue

The objective is not to maximize UGen count.

Each new class must occupy a clear conceptual role:

```text
MultiHawkesTrig      interaction
SelfCorrectingTrig  inhibition
RandomTelegraph     regime switching
ShotNoise           stochastic impulses with memory
MultiOUProcess      correlated continuous motion
DoubleWellProcess   metastability and stochastic resonance
TVonMisesRand       circular probability
KuramotoNetwork     collective synchronization
```

When choosing between a more feature-rich implementation and a clearer mathematically faithful one, prefer the clearer model.

When choosing examples, make the defining behavior perceptible.

Do not hide stochastic structure behind unnecessarily elaborate synthesis.

Do not make every example sound like generic random bleeps.

The success criterion is that a user should hear or see why each process exists and understand what kind of organization it contributes to a generative system.

