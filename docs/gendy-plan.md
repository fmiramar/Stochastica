# Stochastica — GENDY / Dynamic Stochastic Synthesis Expansion

## Agentic implementation plan for historical reconstruction, historically informed extensions, and new stochastic waveform models

**Target repository:** `https://github.com/fmiramar/Stochastica`

**Scope:** add a dedicated Dynamic Stochastic Synthesis / GENDY subsystem to Stochastica without modifying or bug-fixing SuperCollider's existing `Gendy1`–`Gendy5` classes. The round must implement historically grounded models where the historical evidence is sufficiently clear, explicitly label historically informed reconstructions where it is not, port important post-Xenakis research extensions, and add new Stochastica extensions that generalize the principle of stochastic waveform construction.

**New classes planned for this round:**

1. `Gendy1977`
2. `Gendy1991`
3. `GendyS709`
4. `GendyField`
5. `GendyPerc`
6. `GendyInterp`
7. `GendyDiffusion`
8. `GendyCoupled`

**Language:** C++17 server-plugin code, SuperCollider language classes, `.schelp` helpfiles, deterministic reference scripts, and the current Stochastica testing/build infrastructure.

**Central conceptual goal:** use stochastic processes not merely to modulate an oscillator, but to generate and continuously reorganize the waveform itself.

---

# Part I — Non-negotiable rules

## 1. Inspect the current repository before editing

The agent must begin by reading the repository as it exists at implementation time. Do not assume that the layout in this plan is still exact.

At minimum inspect:

- `Stochastica.cpp`;
- `CMakeLists.txt`;
- `include/` and all common helpers;
- `maps/`, `flows/`, `processes/`, `distributions/`, `state/`, and `sonifiers/`;
- `Classes/`;
- `HelpSource/Classes/`;
- `HelpSource/Guides/Stochastica.schelp`;
- all current tests;
- RNG, trigger, finite-check, interpolation, range/fold, buffer, and RT-allocation helpers;
- the current implementations of `OUProcess`, `TStableRand`, `BoundedWalk`, `MultiGaussNoise`, `MarkovChain`, and any classes added in Rounds 2 and 3;
- the official SuperCollider `Gendy1`, `Gendy2`, and `Gendy3` implementation and helpfiles;
- the sc3-plugins `Gendy4` and `Gendy5` implementation and helpfiles.

Reuse correct Stochastica infrastructure. Do not create duplicate RNG, fold, trigger, interpolation, or allocation frameworks merely because a local version is convenient.

Create a new `gendy/` source category only if the repository still lacks a natural home for this family. The GENDY subsystem is conceptually substantial enough to justify its own directory.

---

## 2. This round must not patch the existing SuperCollider Gendy UGens

Do **not** change:

- SuperCollider core `Gendy1`;
- SuperCollider core `Gendy2`;
- SuperCollider core `Gendy3`;
- sc3-plugins `Gendy4`;
- sc3-plugins `Gendy5`;
- their seeded behavior;
- their argument order;
- their interpolation behavior;
- their documentation.

Those classes are historical context and comparison points only.

The new Stochastica classes must use distinct names and coexist with the existing family.

Do not copy implementation code from SuperCollider/sc3-plugins unless the license and attribution path is explicitly reviewed. Prefer a clean implementation from mathematical descriptions and cite the source of the algorithm.

---

## 3. Historical provenance must be represented explicitly in code and documentation

Every class in this round belongs to one of four provenance categories.

### A. Historical reconstruction

The algorithm is intended to implement a documented Xenakis-era procedure closely enough that the class can be described as a reconstruction of that method.

Classes:

- `Gendy1977`
- `Gendy1991`

### B. Historically informed reconstruction

The class reconstructs a documented historical idea, but the surviving/publicly verified information is insufficient to claim bit-exact or complete reconstruction.

Classes:

- `GendyS709`
- `GendyField`

### C. Post-Xenakis research extension

The class implements a later published extension by another author/researcher.

Classes:

- `GendyPerc` — Andrew R. Brown / IDSS line of work
- `GendyDiffusion` — Emilio L. Rojas and Rodrigo F. Cádiz / DDSS

`GendyInterp` also draws on Brown's extensions and the existing SC family, but is deliberately generalized beyond any one historical implementation.

### D. New Stochastica extension

The class is a new design proposed for this project and must never be attributed to Xenakis or later authors.

Class:

- `GendyCoupled`

The provenance category must appear in every relevant helpfile near the top.

Never write wording such as “Xenakis's `GendyCoupled`” or imply that a new Stochastica mechanism existed historically.

---

## 4. Historical uncertainty must remain visible

Do not fill documentary gaps by guessing.

When sources disagree:

1. identify the disagreement;
2. prefer primary material when available;
3. otherwise prefer peer-reviewed/scholarly reconstructions;
4. state the uncertainty in the documentation;
5. choose implementation semantics explicitly and label them as Stochastica's choice.

The specific case of `GendyS709` is mandatory: public descriptions verify that S.709 used parameter modulation and that simple periodic shapes such as sine/sawtooth were involved, but they do not provide enough information in the sources currently verified for this plan to advertise a bit-exact S.709 synthesizer.

Therefore its summary must say substantially:

> S.709-informed dynamic stochastic synthesis with internal parameter modulation; not a bit-exact reconstruction of Xenakis's original program.

If the agent later gains access to stronger primary evidence, it may tighten that statement, but only with a citation.

---

## 5. Direct quotations require source verification

The user explicitly requires historical quotation and attribution.

Before inserting **any** direct quotation into a helpfile or guide, the agent must verify the exact words against a source it can inspect directly.

For every quotation record:

- author;
- publication/title;
- year;
- page or stable section/location where possible;
- DOI or stable URL when available;
- whether the quotation is from the original source or quoted through a secondary source.

Never quote from memory.

Never fabricate page numbers.

Never silently modernize wording inside quotation marks.

If only a secondary source is available, write, for example:

> Xenakis 1992, quoted in Luque 2006, p. 24.

Do not pretend that a secondary reproduction was inspected in the original book.

Keep direct quotations short. Historical documentation should be extensive through **paraphrase, explanation, equations, and source comparison**, not through long copyrighted excerpts.

---

## 6. Reference hierarchy for this round

Use the following source hierarchy.

### Tier 1 — Primary / near-primary

- Iannis Xenakis, *Formalized Music*, revised edition, 1992 — when a legally available copy is available to the agent.
- Marie-Hélène Serra, “Stochastic Composition and Stochastic Timbre: GENDY3 by Iannis Xenakis,” *Perspectives of New Music* 31(1), 1993, 236–257.
- Brigitte Robindoré, “Eskhaté Ereuna: Extending the Limits of Musical Thought—Comments On and By Iannis Xenakis,” *Computer Music Journal* 20(4), 1996, 11–16.

### Tier 2 — Scholarly reconstructions and extensions

- Peter Hoffmann, “The New GENDYN Program,” *Computer Music Journal* 24(2), 2000, 31–38.
- Peter Hoffmann, “Something rich and strange: Exploring the Pitch Structure of GENDY3,” *Journal of New Music Research* 33(2), 2004, 137–144.
- Sergio Luque, *Stochastic Synthesis: Origins and Extensions*, Institute of Sonology master's thesis, 2006.
- Sergio Luque, “The Stochastic Synthesis of Iannis Xenakis,” *Leonardo Music Journal* 19, 2009, 77–84, DOI `10.1162/lmj.2009.19.77`.
- Andrew R. Brown and Greg Jenkins, “The Interactive Dynamic Stochastic Synthesizer,” 2004.
- Andrew R. Brown, “Extending Dynamic Stochastic Synthesis,” ICMC 2005, 111–114.
- Emilio L. Rojas and Rodrigo F. Cádiz, “A Physically Inspired Implementation of Xenakis's Stochastic Synthesis: Diffusion Dynamic Stochastic Synthesis,” *Computer Music Journal* 45(2), 2021, 48–66, DOI `10.1162/comj_a_00606`.

### Tier 3 — Curated technical/historical resources

- Peter Hoffmann, “Stochastic Synthesis,” *A Xenakis Dictionary*, 2023, Iannis Xenakis official site.
- current SuperCollider documentation for `Gendy1`, `Gendy2`, `Gendy3`, `Gendy4`, and `Gendy5`.

Tier 3 can establish what **SuperCollider currently claims/implements**. It must not override stronger historical evidence.

---

## 7. Real-time safety

All server-side code must be bounded and RT-safe.

Mandatory:

- no heap allocation in the calculation function;
- no file I/O;
- no mutex acquisition;
- no exceptions escaping into the server;
- no console printing from the sample loop;
- no unbounded rejection sampling;
- no unbounded catch-up loops;
- no per-sample vector resizing;
- no dynamic output count after construction;
- all breakpoint arrays allocated once through the repository's RT-safe construction mechanism;
- explicit upper bound on control points/voices;
- finite checks after state updates;
- deterministic fallback for invalid internal state.

Suggested initial hard limits, subject to benchmark:

- single oscillator control points: `2..64`;
- `GendyField` voices: `1..32`;
- `GendyCoupled` control points: `2..64`.

Do not choose a higher limit simply because memory permits it. Benchmark worst-case update rates.

---

## 8. Seed and reset semantics

Use the current Stochastica local PCG32 convention unless the repository has deliberately changed it.

For stochastic GENDY classes:

- `seed = 0` derives a seed from the parent Synth;
- nonzero seed gives deterministic local behavior;
- reset restores the original local RNG state;
- reset restores all breakpoint states, primary/secondary walks, segment counters, interpolation state, modulation phases where documented, and event state;
- after construction the class must not consume the parent Synth RNG.

A reset must be atomic.

Do not reset only amplitudes while retaining old duration/interpolation state.

---

## 9. Elastic barriers / reflection semantics

Historically, reflecting barriers are part of DSS itself, not a post-processing convenience.

Implement one tested repeated-reflection helper for the new subsystem.

A value that crosses more than one interval width must still fold/refelect correctly into the legal interval. Do not implement a single reflection followed by clamping.

Pseudocode:

```cpp
double reflectPeriodic(double x, double lo, double hi);
```

Requirements:

- finite for finite inputs;
- works for arbitrarily large overshoot;
- handles `lo == hi` as a frozen state;
- no while-loop whose iteration count depends on overshoot magnitude;
- implemented using periodic arithmetic/modulo;
- covered by exhaustive tests.

This helper is for the new Stochastica subsystem only. Do not use this round to alter legacy SuperCollider/sc3-plugins behavior.

---

## 10. Breakpoint continuity

For waveform-producing classes, the last segment of one waveform/cycle must connect continuously to the first breakpoint of the next cycle unless the class explicitly documents a discontinuous mode.

No hidden click should be introduced merely because a cycle state was regenerated.

The test suite must check sample continuity at cycle transitions under deterministic test sequences.

---

## 11. Documentation examples remain minimal

Preserve the existing Stochastica rule:

- first examples should be extremely small;
- avoid many variables unless the idea genuinely requires a network, comparison, or visualization;
- do not create elaborate tutorial patches just to demonstrate one argument;
- larger examples are appropriate for `GendyField`, historical comparisons, and interacting/coupled systems.

---

## 12. Runnable code must never be commented out

Any code intended to be evaluated must be active.

Never publish examples where the user is expected to uncomment lines.

Never put alternate executable examples in comments.

---

## 13. Comments may appear only inside a main `(...)` example block

Do not place:

```supercollider
// try this
{ ... }.play;
```

before the runnable code.

Use SCDoc prose/subsection headings outside code.

Inside a complete runnable block this is allowed:

```supercollider
(
{
    var mobility = Line.kr(1, 0.001, 2);
    // The waveform stabilizes because the stochastic step size collapses.
    GendyPerc.ar(Impulse.kr(0.5), 2, mobility: mobility) * 0.1
}.play;
)
```

Do not comment out executable alternatives inside the block.

---

## 14. `See Also` must never be at the bottom

Mandatory repository-wide rule for every file touched by this round:

Use top metadata:

```text
related:: Guides/Stochastica, Guides/Stochastica-GENDY, Classes/Gendy1991
```

If a literal prose section named `See Also` is ever justified, place it near the top after the conceptual introduction and before methods/examples.

Forbidden:

```text
examples::
...
section:: See Also
...
```

Never append `See Also` after examples.

A references/bibliography section **may** appear at the bottom. The prohibition applies specifically to `See Also`.

---

# Part II — Historical and conceptual foundation that must appear in the documentation

## 15. Why a dedicated GENDY guide is required

Create:

```text
HelpSource/Guides/Stochastica-GENDY.schelp
```

The general `Stochastica.schelp` should contain a concise overview and link to the dedicated guide.

The dedicated guide must be substantial. It should explain Dynamic Stochastic Synthesis historically, mathematically, conceptually, and in relation to the eight Stochastica classes.

The guide must not be merely an API index.

---

## 16. Core conceptual explanation

Explain DSS in plain conceptual language before equations.

The essential idea:

A conventional oscillator usually defines a waveform first and repeats it. Dynamic Stochastic Synthesis instead defines a cycle by a collection of breakpoints in time-amplitude space and lets stochastic processes move those breakpoints from one cycle to the next.

Thus the waveform is not merely **played**; it is an evolving stochastic object.

The user should understand the difference between:

```text
noise -> oscillator parameter
```

and:

```text
stochastic process -> geometry of the waveform itself
```

This is the conceptual center of the round.

Explain that this is one reason DSS is often described as a nonstandard synthesis approach: it starts from direct time-domain waveform organization instead of beginning from a fixed oscillator model or a Fourier decomposition.

---

## 17. Breakpoints, random walks, and elastic barriers

The guide must explain three layers clearly.

### Breakpoints

A waveform cycle consists of ordered points:

\[
(t_i, A_i), \quad i=0,1,\ldots,N-1.
\]

The samples between adjacent points are generated by interpolation.

### Random walks

At each evolution step the breakpoint time/duration and amplitude can change according to a stochastic increment.

For a first-order walk:

\[
x_{n+1}=R(x_n+\Delta_n),
\]

where `R` is the reflecting-barrier operation.

### Elastic barriers

The barriers are part of the stochastic state dynamics. They define the region within which the random walk can evolve.

Explain conceptually:

- narrow barriers constrain movement;
- wide barriers permit larger structural change;
- coincident barriers freeze a parameter;
- barrier placement can be more perceptually important than the precise random distribution.

Peter Hoffmann's official Xenakis Dictionary entry states that GENDYN's **“sound characteristics are not governed by waveform shape per se”** but by the stochastic change of that shape. Cite this precisely as Hoffmann 2023, *A Xenakis Dictionary*, “Stochastic Synthesis.”

Do not quote additional words from this source elsewhere unless still within the repository's quotation policy.

---

## 18. The 1977 method

The guide must explain the method associated with the stochastic synthesis portions of *La Légende d’Eer*.

Based on Luque's reconstruction:

- a waveform is represented by breakpoints;
- each breakpoint has a duration/time component and an amplitude component;
- each component follows a first-order random walk;
- probability distributions provide random walk increments;
- reflecting barriers constrain the walks;
- breakpoints are connected by linear interpolation;
- the evolved points define the next waveform.

Conceptually:

> **Randomness moves the waveform.**

The stochastic increment directly perturbs the breakpoint's state.

This must be the conceptual explanation of `Gendy1977`.

Include a short historically sourced quotation. Luque reproduces Xenakis's characterization of the method as something that **“bears the seed of future exploration.”** Attribute as Xenakis 1992, quoted in Luque 2006, p. 24.

Do not claim that the exact Stochastica implementation reproduces every implementation detail of Xenakis's original 1977 software unless independently verified.

---

## 19. The 1991 method and second-order walks

The guide must carefully distinguish first-order and second-order motion.

According to Luque's reconstruction, the major algorithmic difference in the later GENDY procedure is second-order random walking.

For one state component, use conceptual notation:

\[
p_{n+1}=R_p(p_n+\epsilon_n)
\]

\[
s_{n+1}=R_s(s_n+p_{n+1})
\]

where:

- `epsilon` is drawn from a probability distribution;
- `p` is the primary random walk;
- the primary position becomes the step of the secondary walk;
- `s` is the secondary state that determines the breakpoint coordinate.

Conceptually:

```text
1977:
randomness -> position

1991:
randomness -> movement -> position
```

Or more precisely:

> the random process changes the velocity/tendency of another random walk rather than moving the breakpoint directly.

This produces persistence and barrier-oriented behavior not present in the same way in a first-order random walk.

The guide must explain that second-order dynamics can create prolonged tendencies, reversals, and zones of relative stability without making the process deterministic.

Luque's 2006 thesis contains the warning that the available literature can be incomplete/conflicting and describes the 1991 method as a probability distribution feeding primary and secondary random walks. Use the thesis as a key reconstruction source.

The historical help must also credit Marie-Hélène Serra. Luque describes GENDY as a BASIC program written by Xenakis with Serra's assistance; current SuperCollider documentation explicitly highlights Serra's contribution to the BASIC program. Do not erase this contribution.

---

## 20. GENDY3 is not only an oscillator story: PARAG and macrostructure

The guide must explain the crucial micro/macro distinction.

Historical accounts describe a second program, PARAG, which organized GENDY-generated material into multiple voices and time fields.

Luque's reconstruction states that PARAG split voices into time fields; field duration was generated with an exponential distribution and a Bernoulli trial determined sounding versus silence.

This means stochastic organization operated at more than one scale:

```text
micro:
breakpoint motion -> waveform/timbre/pitch behavior

macro:
field durations + sound/silence decisions -> texture/form
```

This is the conceptual foundation of `GendyField`.

Do not imply that `GendyField` is the historical PARAG program. It implements one documented and musically important subset: stochastic activity/silence fields for multiple voices.

If the agent verifies the original fixed voice count used in a specific historical implementation, it may mention it with citation. The Stochastica class itself must remain configurable.

---

## 21. GENDY3 and S.709 must not be conflated

The guide must explicitly explain that the sources distinguish the behavior of *Gendy3* and *S.709*.

Verified points:

- *Gendy3* is associated with second-order random walks and relatively stable section-level parameter settings;
- after *Gendy3*, Xenakis extended the program with parameter modulation;
- Luque reports simple periodic modulation shapes such as sine and sawtooth for the S.709-era program;
- S.709 therefore explores stochastic synthesis whose **control parameters themselves move through time**.

Conceptually:

```text
Gendy1991:
stochastic state evolves under comparatively stable meta-parameters

GendyS709:
the meta-parameters themselves become time-varying processes
```

This is hierarchical modulation:

\[
\text{control process}
\rightarrow
\text{parameters of stochastic process}
\rightarrow
\text{waveform}.
\]

The guide must also state that the exact original routing/values are not sufficiently specified by the sources verified for this project, hence `GendyS709` is historically informed rather than bit-exact.

---

## 22. Percussive DSS: stochastic stabilization becomes articulation

Andrew R. Brown's IDSS work must be explained as a post-Xenakis extension.

The essential idea is not “put an envelope on GENDY.” Instead, the stochastic mobility itself changes through time.

Brown's 2005 abstract describes **“a new class of percussive stochastic synthesis sound gestures.”**

Brown's later Gendynish summary describes step-size automation producing a **“rapid change from a bright complex timbre to a fixed and stable tone.”**

These are separate Brown sources; keep the quotations attributed separately.

Conceptually:

```text
large stochastic step sizes
        -> rapid waveform change / spectral complexity
        -> decreasing step sizes
        -> reduced movement
        -> quasi-frozen waveform
```

This is a profound synthesis idea:

> articulation can result from the changing freedom of the stochastic system, not merely from an amplitude envelope.

This is the conceptual foundation of `GendyPerc`.

---

## 23. Interpolation is part of the synthesis model

The existing SC family already demonstrates that interpolation changes the sonic identity:

- `Gendy1` uses the classic linearly connected breakpoint conception;
- `Gendy4` is documented as a **“cubic-interpolated version of Nick Collins's Gendy1 UGen”**;
- `Gendy5` is documented as a **“non-interpolating version of Nick Collins's Gendy1 UGen”**.

Brown's IDSS research also explored alternative interpolation, including cosine interpolation, to control harmonic complexity.

Explain conceptually:

> the stochastic process chooses/evolves the points; interpolation determines the geometry of the path between them.

These are independent dimensions.

`GendyInterp` must make interpolation explicit instead of encoding it indirectly in numbered class names.

Do not implement any historically named interpolation mode unless its formula is verified. If Brown's “square” interpolation cannot be reconstructed unambiguously from source/code, omit it or label it as a new Stochastica curve rather than claiming historical fidelity.

---

## 24. Diffusion Dynamic Stochastic Synthesis

The guide must give Rojas and Cádiz explicit authorship.

Their DDSS paper describes a published extension in which solutions of a diffusion equation map particle positions to waveform breakpoint amplitudes.

Their abstract notes that DDSS **“includes a drift in the Brownian trajectories”** experienced by breakpoints.

Conceptually, ordinary DSS can be understood as constrained random movement of breakpoint coordinates. DDSS replaces/generalizes that movement with a physically motivated diffusion process containing both random diffusion and systematic drift.

A compact SDE form is:

\[
dX_t = \mu(X_t,t)\,dt + \sigma(X_t,t)\,dW_t,
\]

but the implementation must use the exact model/equations specified by Rojas and Cádiz after the agent verifies the full article. Do not infer missing DDSS equations from the generic SDE above.

This class must be a clean implementation from the published mathematical description. Do not copy Max patch/code unless its license is explicitly compatible and attribution is handled.

---

## 25. Coupled stochastic breakpoints

`GendyCoupled` is a new Stochastica design.

Its conceptual question is:

> What happens if breakpoints are not independent stochastic agents, but a locally interacting population?

Use local Laplacian coupling as the initial model:

\[
L_i = x_{i-1} - 2x_i + x_{i+1}
\]

with periodic indexing.

A generic coupled update can add:

\[
\kappa L_i
\]

to the stochastic breakpoint evolution.

Conceptually:

```text
coupling = 0
    independent stochastic breakpoints

small coupling
    local correlation and shared motion

larger coupling
    collective/smoother waveform deformation
```

This is not historical GENDYN. State that clearly.

The class should connect the GENDY subsystem with Stochastica's broader interest in interacting systems, coupled maps, diffusion, and collective dynamics.

---

# Part III — Verified historical quotation dossier

## 26. Quotes that may be used in the repository documentation

The following quotations were verified during preparation of this plan. The agent should still re-open the source before committing documentation, because source availability/versioning can change.

### 26.1 Xenakis via Luque 2006

Quote:

> “it bears the seed of future exploration.”

Attribution:

- Iannis Xenakis, *Formalized Music* (1992), p. 293;
- quoted in Sergio Luque, *Stochastic Synthesis: Origins and Extensions* (2006), p. 24.

Use to frame DSS as a starting point rather than a closed recipe.

### 26.2 Peter Hoffmann 2023

Quote:

> “sound characteristics are not governed by waveform shape per se”

Attribution:

- Peter Hoffmann, “Stochastic Synthesis,” *A Xenakis Dictionary*, 2023;
- official Iannis Xenakis site.

Use when explaining that evolution/dynamics matter more than a static initial waveform.

### 26.3 Andrew R. Brown 2005

Quote:

> “a new class of percussive stochastic synthesis sound gestures”

Attribution:

- Andrew R. Brown, “Extending Dynamic Stochastic Synthesis,” ICMC 2005.

Use in `GendyPerc` historical context.

### 26.4 Andrew R. Brown 2019

Quote:

> “rapid change from a bright complex timbre to a fixed and stable tone”

Attribution:

- Andrew R. Brown, “Gendynish: Stochastic Synthesis on the Arduino,” 2019.

Use to explain stochastic stabilization as articulation.

### 26.5 Sergio Luque 2009

Quote:

> “stochastic concatenation of dynamic stochastic synthesis”

Attribution:

- Sergio Luque, “The Stochastic Synthesis of Iannis Xenakis,” *Leonardo Music Journal* 19 (2009), DOI `10.1162/lmj.2009.19.77`.

This round does **not** implement `GendyConcat`, but the guide should mention it as an important later extension and future direction.

### 26.6 Rojas and Cádiz 2021

Quote:

> “includes a drift in the Brownian trajectories”

Attribution:

- Emilio L. Rojas and Rodrigo F. Cádiz, “A Physically Inspired Implementation of Xenakis's Stochastic Synthesis: Diffusion Dynamic Stochastic Synthesis,” *Computer Music Journal* 45(2), 2021, DOI `10.1162/comj_a_00606`.

Use in `GendyDiffusion`.

### 26.7 Current SuperCollider `Gendy2` documentation

Quote:

> “Random walk is of the amplitude and time delta”

Attribution:

- SuperCollider 3.14 documentation, `Gendy2`.

Use only when explaining the current SC family. Do not present this documentation as primary evidence of Xenakis's historical implementation.

### 26.8 Current `Gendy4` / `Gendy5` documentation

Quotes:

> “cubic-interpolated version of Nick Collins's Gendy1 UGen”

> “non-interpolating version of Nick Collins's Gendy1 UGen”

Attribution:

- sc3-plugins / SuperCollider help for `Gendy4` and `Gendy5`.

Use only for current implementation comparison.

---

## 27. Historical claims that require careful wording

The agent must not state the following without qualification.

### “Gendy1 is the 1977 algorithm”

Do not say this. Current `Gendy1` is a SuperCollider implementation/generalization inspired by Xenakis's descriptions. The new `Gendy1977` should be based on the reconstructed historical method rather than defined merely as “Gendy1 with another name.”

### “Gendy2 is exactly GENDY3”

Do not say this. Current SC docs say `Gendy2` is **closer** to Hoffmann's presentation and moves amplitude/time deltas. “Closer” is not “bit-exact.”

### “S.709 is completely documented”

Do not say this. Treat `GendyS709` as historically informed unless stronger source evidence is obtained.

### “PARAG equals GendyField”

Do not say this. `GendyField` extracts the documented time-field activity/silence concept; PARAG was a broader macro-compositional program.

### “DDSS was invented in Stochastica”

Never say this. Credit Rojas and Cádiz prominently.

### “Coupled GENDY is Xenakis's idea”

Never say this. `GendyCoupled` is explicitly a new Stochastica extension.

---

# Part IV — How the current SuperCollider Gendy family should be explained

## 28. Add a comparison section to `Stochastica-GENDY.schelp`

Explain the five current SC classes before introducing the new ones.

### `Gendy1`

Current SuperCollider's general dynamic stochastic generator. Its help describes amplitude and duration breakpoint perturbation using selectable probability distributions, user-chosen control-point memory size, and frequency constraints.

Important documentation point:

- it is flexible/generalized;
- it should not be used as the sole definition of historical DSS.

### `Gendy2`

Current SC documentation says this version is closer to Hoffmann's New GENDYN presentation and that the random walk applies to amplitude/time deltas.

Conceptually this moves toward a higher-order/persistent stochastic model.

Do not claim exact equivalence to the historical BASIC program.

### `Gendy3`

Current SC class normalizes segment durations per period to enforce a target frequency while retaining stochastic breakpoint perturbation.

Conceptually it separates pitch control from unconstrained total cycle duration more strongly than `Gendy1`.

### `Gendy4`

sc3-plugins extension using cubic interpolation.

### `Gendy5`

sc3-plugins extension without interpolation / sample-and-hold-like behavior.

### Why Stochastica adds named variants

Explain that numbered variants hide the conceptual axes.

Stochastica instead names the model by what changes:

```text
Gendy1977      -> first-order historical method
Gendy1991      -> second-order historical method
GendyS709      -> parameter modulation
GendyField     -> macro time fields
GendyPerc      -> stochastic stabilization/articulation
GendyInterp    -> explicit interpolation geometry
GendyDiffusion -> diffusion/drift breakpoint dynamics
GendyCoupled   -> interacting breakpoint population
```

This naming is intended to make the family pedagogically transparent.

---

# Part V — Common GENDY engine

## 29. Build shared infrastructure first

Do not implement eight independent copies of breakpoint synthesis.

Suggested internal layout:

```text
gendy/
    GendyCommon.hpp
    GendyDistribution.hpp
    GendyReflect.hpp
    GendyInterpolation.hpp
    GendyBreakpointBank.hpp
    Gendy1977.cpp
    Gendy1991.cpp
    GendyS709.cpp
    GendyField.cpp
    GendyPerc.cpp
    GendyInterp.cpp
    GendyDiffusion.cpp
    GendyCoupled.cpp
```

Adapt to repository conventions.

The shared layer should expose small policy/components rather than a giant inheritance tree.

---

## 30. Breakpoint bank

A waveform model needs fixed-size RT-allocated arrays.

Possible state structure:

```cpp
struct GendyBreakpointBank {
    int size;
    double* amp;
    double* dur;
    double* ampPrimary;
    double* durPrimary;
    double* scratchAmp;
    double* scratchDur;
};
```

Not every class uses every array. Allocate only what the specific class needs.

Never allocate a vector during synthesis.

The maximum number of control points is fixed at construction.

If an active point count is supported at runtime, it must remain within the allocated maximum.

---

## 31. Time representation

Use one internal time representation consistently.

Preferred:

- breakpoint segment duration in **samples** or sample fractions internally;
- user API in Hz/range where musically appropriate;
- conversion once per parameter update.

For `N` breakpoints and approximate cycle frequency bounds:

\[
d_{\min} = \frac{f_s}{f_{\max}N}
\]

\[
d_{\max} = \frac{f_s}{f_{\min}N}.
\]

Do not pretend these bounds guarantee exact oscillator frequency when segment durations evolve independently. Explain that they constrain the local segment-duration space and therefore influence the resulting cycle length.

`Gendy3`-style normalized fixed-frequency behavior is not the default historical model for this round.

---

## 32. Distribution layer

The historical classes require a clearly documented distribution enum.

Before freezing the enum/formulas, inspect:

- Xenakis descriptions where legally accessible;
- Hoffmann reconstruction;
- Luque reconstruction;
- current `GendynUGens` source;
- current `Gendy1` documentation.

The likely compatibility set includes:

```text
0 uniform / linear
1 Cauchy
2 logistic
3 hyperbolic-cosine-derived
4 arcsine
5 exponential
```

Do **not** implement names/formulas from memory.

If the exact historical “sinus” behavior cannot be established consistently, do not pretend it is a historical probability law. Provide it only in `GendyInterp`/modern generalized classes if it has a clear, newly documented semantics.

All random-number transforms must:

- use open-uniform values where logarithms/tangents require them;
- be finite-guarded;
- avoid unbounded rejection;
- be deterministic under Stochastica's local seed.

---

## 33. Interpolation layer

Implement shared interpolation between breakpoint endpoints.

Mandatory initial modes:

```text
0 hold / none
1 linear
2 cosine
3 cubic Hermite
```

Add additional modes only with a precise formula and documentation.

Requirements:

- endpoint exactness;
- deterministic behavior;
- no out-of-bounds neighbor reads;
- no hidden overshoot unless documented;
- continuity order documented for each interpolation.

For cubic Hermite, define tangents explicitly. Do not use an ambiguous “cubic” label in code comments/tests.

---

## 34. Segment scheduler

The engine should keep:

```text
current breakpoint index
segment position
segment length
left value
right value
interpolation state
```

At segment completion:

1. advance to the next breakpoint;
2. update the relevant stochastic state for the breakpoint according to the class's model;
3. compute the next segment duration;
4. set interpolation endpoints;
5. preserve cycle continuity;
6. if the cycle wraps, apply class-specific cycle-level behavior.

Avoid per-sample stochastic state updates when the mathematical model updates per breakpoint/cycle.

---

## 35. Diagnostic/reference mode

For development tests, provide a non-RT reference implementation in `tests/reference/` or `scripts/` that can emit breakpoint traces.

Recommended files:

```text
scripts/gendy_reference.py
tests/gendy_reference_sequences.cpp
```

Reference output should include:

```text
cycle
breakpoint index
amp primary
amp secondary
duration primary
duration secondary
rendered segment length
```

This makes historical and mathematical debugging possible without inferring internal state from audio.

Do not expose this diagnostic data as normal audio-rate UGen outputs unless there is a compelling user-facing reason.

---

# Part VI — `Gendy1977`

## 36. Concept

`Gendy1977` implements first-order Dynamic Stochastic Synthesis reconstructed from the method associated with *La Légende d’Eer*.

Conceptual sentence for the helpfile:

> Random increments move the amplitude and duration coordinates of the waveform's breakpoints directly.

This is the baseline model against which later GENDY variants should be understood.

---

## 37. Proposed API

Keep the historical class relatively focused:

```supercollider
Gendy1977.ar(
    ampDist: 0,
    durDist: 0,
    ampShape: 1,
    durShape: 1,
    minFreq: 100,
    maxFreq: 1000,
    ampStep: 0.1,
    durStep: 0.1,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

Also `.kr` only if the existing plugin architecture makes a control-rate waveform generator meaningful and the behavior can be defined clearly. Audio rate is the primary target.

Do not add an interpolation argument here: historical baseline is linear. General interpolation belongs to `GendyInterp`.

---

## 38. State update

For each breakpoint amplitude:

\[
A_i' = R_A(A_i + s_A \Delta_A)
\]

For duration:

\[
D_i' = R_D(D_i + s_D \Delta_D)
\]

where distributions generate normalized deltas and step-scale arguments control mobility.

The exact mapping of distribution output to step space must be documented and reference-tested.

---

## 39. Initialization

Luque notes several possible historical initialization strategies. Stochastica must choose one deterministic default and state it explicitly.

Recommended default:

- amplitudes initialized to zero;
- durations initialized to the midpoint of their permitted interval;
- first cycle begins from a flat/regular state and evolves stochastically.

If an `initMode` is added, keep it initialization-rate only and define modes precisely.

Do not add many initialization options in the first version merely to imitate every historical possibility.

---

## 40. Required tests

- fixed artificial delta sequence reproduces hand-computed first-order walk;
- reflection is exact after large overshoot;
- amplitude remains inside barriers;
- duration remains positive and inside barriers;
- same nonzero seed -> same breakpoint trace;
- reset -> exact initial trace;
- last-to-first segment is continuous;
- `ampStep = 0` freezes amplitudes;
- `durStep = 0` freezes durations;
- both zero -> periodic fixed waveform;
- `numPoints` min/max limits;
- no NaN under extreme valid inputs.

---

## 41. Minimal documentation examples

First sound:

```supercollider
{ Gendy1977.ar * 0.1 }.play;
```

Stable vs mobile:

```supercollider
{ Gendy1977.ar(ampStep: 0.02, durStep: 0.02) * 0.1 }.play;
```

Large example should compare first- and second-order behavior, not add arbitrary effects.

---

# Part VII — `Gendy1991`

## 42. Concept

`Gendy1991` implements the second-order Dynamic Stochastic Synthesis model associated with GENDY3.

Conceptual sentence:

> Randomness changes a primary walk; the primary walk then becomes the step of the breakpoint's secondary walk.

This is **stochastic motion of stochastic motion**.

---

## 43. Proposed API

```supercollider
Gendy1991.ar(
    ampDist: 0,
    durDist: 0,
    ampShape: 1,
    durShape: 1,
    minFreq: 100,
    maxFreq: 1000,
    ampPrimaryRange: 0.1,
    durPrimaryRatio: 0.5,
    ampPrimaryStep: 0.1,
    durPrimaryStep: 0.1,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

Parameter naming should reflect the two layers rather than copy the ambiguous `ampscale`/`durscale` naming of older classes.

`ampPrimaryRange` defines the barriers of the primary amplitude walk relative to the secondary amplitude range.

`durPrimaryRatio` defines the primary duration-walk barrier width relative to the secondary duration range.

`ampPrimaryStep` and `durPrimaryStep` scale random increments that move the primary walks.

Freeze exact scaling after the historical source audit and reference tests.

---

## 44. State equations

Conceptual amplitude state:

\[
P^A_{i,n+1}=R_P(P^A_{i,n}+\epsilon^A_{i,n})
\]

\[
A_{i,n+1}=R_A(A_{i,n}+P^A_{i,n+1}).
\]

Duration analogously:

\[
P^D_{i,n+1}=R_P(P^D_{i,n}+\epsilon^D_{i,n})
\]

\[
D_{i,n+1}=R_D(D_{i,n}+P^D_{i,n+1}).
\]

The implementation should update old/new state in a clearly defined order and test it.

---

## 45. Required tests

In addition to the baseline tests:

- primary walks stay within primary barriers;
- secondary states stay within secondary barriers;
- fixed delta sequence matches hand-computed second-order trajectory;
- primary zero -> secondary stops moving;
- barrier attraction/tendency can be demonstrated statistically without turning it into a brittle unit test;
- `Gendy1991` and `Gendy1977` do not accidentally share the same state recursion;
- reset restores both walk levels.

Add an offline statistical test that compares residence distributions near secondary barriers for first- and second-order test configurations.

---

## 46. Creative documentation example: movement vs movement-of-movement

Create one medium-size example that alternates or crossfades between `Gendy1977` and `Gendy1991` using matched parameter ranges.

The prose should ask the reader to listen for:

- transient directionality;
- persistence;
- glissando tendencies;
- relative stability;
- barrier-oriented behavior.

Do not bury the comparison under reverb or distortion.

---

# Part VIII — `GendyS709`

## 47. Provenance label

**Historically informed reconstruction.**

Do not call it “the S.709 algorithm.”

Preferred summary:

> S.709-informed Dynamic Stochastic Synthesis in which synthesis meta-parameters are periodically modulated.

---

## 48. Concept

Ordinary DSS changes waveform state.

This class changes the parameters that govern that stochastic change.

Conceptual hierarchy:

```text
periodic control process
        -> stochastic mobility/barriers
        -> breakpoint evolution
        -> waveform
```

This adds a slower level of organization over the stochastic microstructure.

---

## 49. Proposed API

Keep one internal modulator initially.

```supercollider
GendyS709.ar(
    ampDist: 0,
    durDist: 0,
    minFreq: 100,
    maxFreq: 1000,
    ampStep: 0.1,
    durStep: 0.1,
    modFreq: 0.2,
    modShape: 0,
    ampStepDepth: 0.5,
    durStepDepth: 0.5,
    freqRangeDepth: 0,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

Initial `modShape`:

```text
0 sine
1 saw
```

because those shapes are explicitly reported in Luque's discussion of S.709-era modulation.

Do not add triangle/random/etc. to the historically named class unless clearly labeled as Stochastica extensions.

---

## 50. Modulation semantics

Define modulation as a bipolar signal:

\[
m(t)\in[-1,1].
\]

Parameter modulation should preserve validity, for example:

\[
s_A(t)=s_{A0}\,2^{d_A m(t)}
\]

may be preferable to additive modulation because positive step sizes remain positive.

However, the exact transform is a Stochastica design choice unless historical equations are verified. Document this openly.

For frequency-range modulation, modulate log-frequency or a range ratio rather than allowing min/max to cross.

---

## 51. Required tests

- `modFreq = 0` yields static meta-parameters;
- depth zero yields baseline engine behavior;
- sine and saw phase deterministic across reset;
- modulated values never violate parameter domain;
- no minFreq/maxFreq inversion;
- same seed + same modulation phase -> deterministic output;
- modulation itself does not allocate or branch unpredictably.

---

## 52. Creative example: hierarchy of timescales

Use one large example only if needed to make the point audible:

- slow meta-modulation changes stochastic mobility;
- breakpoint evolution creates faster timbral motion;
- the resulting waveform generates audio-rate structure.

The prose should explicitly describe three timescales.

---

# Part IX — `GendyField`

## 53. Provenance label

**Historically informed macrostructure process inspired by documented PARAG time fields.**

It is not the full PARAG program.

---

## 54. Concept

`GendyField` moves the GENDY idea from waveform microstructure to the organization of activity and silence.

Each voice alternates through independently generated fields.

For the initial historical mode:

- field durations follow an exponential distribution;
- each new field is sounding or silent according to a Bernoulli probability.

This yields irregular overlapping blocks of activity.

---

## 55. Proposed API

Control rate only:

```supercollider
GendyField.kr(
    numVoices: 4,
    meanDuration: 1,
    soundProb: 0.5,
    reset: 0,
    seed: 0
)
```

Output: one gate per voice.

`numVoices` is initialization-only because it determines output count and allocation.

Optional future extension: parameter buffers for per-voice mean duration/probability. Do not complicate v1 unless a compelling example requires it.

---

## 56. Scheduler

Each voice stores:

```text
remaining field time
current gate state
RNG state derived from class seed/voice index
```

At field expiration:

1. draw an exponential duration with configured mean;
2. draw Bernoulli activity;
3. update gate;
4. carry any small timing remainder safely;
5. emit at most the bounded number of state transitions permitted by the control calculation.

If a mean duration is smaller than one control block, define a safe minimum and document it.

---

## 57. Required tests

- output count fixed at construction;
- fields remain constant between transitions;
- exponential dwell mean approximately correct offline;
- activity proportion approximately `soundProb`;
- `soundProb=0` always silent;
- `soundProb=1` always active, while field boundaries still occur internally if relevant;
- seeded multivoice independence reproducible;
- reset restarts exact field sequences.

---

## 58. Creative example: stochastic counterpoint of presence

A larger example is justified here.

Use 6–8 independent GENDY voices, each gated by one `GendyField` output.

The example should make the macro idea audible:

```text
voice 0  ====----===-----------
voice 1  ------======----==----
voice 2  ==--------------====--
voice 3  ----===---------------
```

Do not turn this into a conventional sequencer demo. The point is overlapping stochastic fields of activity.

---

# Part X — `GendyPerc`

## 59. Provenance label

**Post-Xenakis extension derived from Andrew R. Brown's IDSS work.**

Credit Brown explicitly in the summary/history.

---

## 60. Concept

A trigger starts a gesture with high stochastic mobility. Mobility decays toward a low/frozen state.

No separate amplitude envelope is required to create the primary articulation.

The sound's “decay” is a reduction in waveform change.

---

## 61. Proposed API

```supercollider
GendyPerc.ar(
    trig: 0,
    decay: 1,
    minFreq: 100,
    maxFreq: 2000,
    startAmpStep: 0.5,
    endAmpStep: 0.001,
    startDurStep: 0.5,
    endDurStep: 0.001,
    numPoints: 12,
    resetState: 1,
    seed: 0
)
```

The exact underlying walk order must be chosen after reviewing Brown's implementation description. If the historical IDSS gesture used a two-level walk, implement that or expose the choice explicitly.

Do not silently assume.

---

## 62. Mobility envelope

On trigger define normalized progress:

\[
u(t)=\operatorname{clip}(t/T,0,1).
\]

Interpolate step sizes exponentially where both endpoints are positive:

\[
s(t)=s_0\left(\frac{s_1}{s_0}\right)^u.
\]

This gives perceptually useful collapse over orders of magnitude.

If an endpoint is zero, handle it explicitly rather than taking `log(0)`.

Do not automatically apply an amplitude envelope. If the user wants amplitude decay, that should normally remain compositional routing outside the UGen.

---

## 63. Trigger behavior

Define clearly:

- `resetState = 1`: trigger restarts waveform state plus mobility trajectory;
- `resetState = 0`: trigger restarts mobility trajectory while continuing current waveform state.

This is musically important.

---

## 64. Required tests

- step scales monotonically approach endpoints;
- trigger restarts mobility trajectory sample/control-block accurately;
- reset mode semantics exact;
- end state remains finite;
- `start == end` produces constant mobility;
- zero/near-zero end handled without NaN;
- repeated triggers do not leak/allocate.

---

## 65. Creative examples

Minimal:

```supercollider
{ GendyPerc.ar(Impulse.kr(0.5), 1.5) * 0.1 }.play;
```

Concept example should compare:

- a conventional amplitude envelope around a continuously mobile GENDY;
- a constant-amplitude `GendyPerc` whose internal stochasticity stabilizes.

This comparison demonstrates why the class exists.

---

# Part XI — `GendyInterp`

## 66. Provenance label

**Generalized Stochastica interpolation framework informed by DSS research and the existing SC `Gendy4`/`Gendy5` variants.**

Do not imply that all interpolation modes were used by Xenakis.

---

## 67. Concept

Stochastic dynamics determine where the control points are.

Interpolation determines what happens between them.

The UGen exists to separate these two ideas explicitly.

---

## 68. Proposed API

Use a first-order DSS engine by default to keep the comparison clear:

```supercollider
GendyInterp.ar(
    interp: 1,
    ampDist: 0,
    durDist: 0,
    minFreq: 100,
    maxFreq: 1000,
    ampStep: 0.1,
    durStep: 0.1,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

Initial modes:

```text
0 hold
1 linear
2 cosine
3 cubic Hermite
```

If a historically verified Brown/IDSS “square” interpolation formula is found, add it with attribution. Otherwise do not invent one under that name.

---

## 69. Interpolation tests

For each interpolation:

- output equals left endpoint at `t=0`;
- output equals right endpoint at `t=1` where appropriate;
- no out-of-range memory access;
- linear exactly matches affine reference;
- cosine derivative is zero at endpoints if that is the chosen formula;
- Hermite tangent policy tested;
- hold changes only at segment boundary;
- live `interp` changes, if supported, have a defined transition policy.

Prefer making `interp` control-rate and switching only at segment boundaries to avoid arbitrary mid-segment discontinuities.

---

## 70. Creative example

Use identical seed and stochastic parameters in four voices, differing only by interpolation mode.

This is one of the few examples where multichannel variables are justified.

The user should hear that identical stochastic breakpoint trajectories can produce different spectra because the curve between points changes.

---

# Part XII — `GendyDiffusion`

## 71. Provenance label

**Implementation of the published DDSS concept by Emilio L. Rojas and Rodrigo F. Cádiz.**

Their names and DOI must appear in the helpfile.

---

## 72. Do not freeze the API before reading the full paper

Unlike the other classes, the precise public API in this plan is provisional because the implementation must follow the published DDSS equations rather than a generic diffusion process guessed from the abstract.

Before coding:

1. obtain/read the complete article;
2. extract the exact state equation, boundary treatment, particle-to-breakpoint mapping, timestep, and parameter definitions;
3. write a short internal `docs/research/GendyDiffusion-source-notes.md` with page references;
4. only then freeze the API.

Do not use generic Brownian motion as a substitute and still call it DDSS.

---

## 73. Provisional API shape

Expected shape only:

```supercollider
GendyDiffusion.ar(
    freq: 220,
    drift: 0,
    diffusion: 0.2,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

The final argument names must match the actual DDSS model.

If DDSS defines additional physical parameters, expose those instead of forcing them into vague `shape` arguments.

---

## 74. Required tests

After the paper-derived equations are fixed:

- reproduce published qualitative/reference cases where possible;
- zero diffusion limit;
- zero drift limit;
- deterministic seeded trajectories;
- boundary semantics;
- particle/breakpoint mapping;
- timestep convergence or stability check;
- no non-finite long-run state under documented valid range;
- spectral/trajectory comparison script against a simplified DSS baseline, inspired by the comparison in the paper without copying copyrighted figures.

Generate new plots from Stochastica output.

---

## 75. Documentation example

The first example should be simple.

A larger example should compare:

```text
Gendy1977
vs
GendyDiffusion
```

with similar breakpoint count/frequency range.

The prose should focus on:

- unbiased/random wandering;
- systematic drift;
- physical-process interpretation;
- how drift changes the long-term geometry of the evolving waveform.

---

# Part XIII — `GendyCoupled`

## 76. Provenance label

**New Stochastica extension.**

No historical attribution beyond saying it extends the general DSS idea.

---

## 77. Concept

Traditional breakpoint walks can be treated as largely independent.

`GendyCoupled` makes neighboring breakpoints interact.

Initial topology: periodic ring.

For amplitude state after stochastic update:

\[
A_i \leftarrow A_i + \kappa_A(A_{i-1}-2A_i+A_{i+1}).
\]

Optional duration coupling:

\[
D_i \leftarrow D_i + \kappa_D(D_{i-1}-2D_i+D_{i+1}).
\]

Then apply the appropriate barrier operation.

The exact update ordering must be documented and tested.

---

## 78. Proposed API

```supercollider
GendyCoupled.ar(
    ampCoupling: 0.1,
    durCoupling: 0,
    ampDist: 0,
    durDist: 0,
    minFreq: 100,
    maxFreq: 1000,
    ampStep: 0.1,
    durStep: 0.1,
    numPoints: 12,
    reset: 0,
    seed: 0
)
```

Initial version should couple first-order breakpoint states for conceptual clarity.

A later extension can support second-order coupled motion if there is a clear need.

---

## 79. Update ordering

Use synchronous coupling:

1. compute all stochastic proposals from old state;
2. store proposals in scratch arrays;
3. compute all coupling terms from the proposal array or old array according to one documented policy;
4. write all new states simultaneously;
5. reflect into legal barriers.

Never update point 0 in place and then use its new value while computing point 1.

Recommended first definition:

- stochastic proposal from old state;
- Laplacian computed on proposal state;
- simultaneous coupled result;
- reflection.

---

## 80. Required tests

- `ampCoupling=durCoupling=0` matches uncoupled first-order engine exactly for same seed/params;
- uniform amplitude state has zero Laplacian;
- coupling with zero stochasticity reduces local roughness over successive updates;
- periodic boundary indexing correct;
- synchronous update invariant to iteration order;
- no negative/zero durations after reflection;
- finite behavior across documented coupling range;
- coupling parameter clamped or rejected outside numerically safe range.

---

## 81. Creative example: independent agents become a medium

Create an example that sweeps amplitude coupling from zero upward while keeping stochasticity fixed.

The prose should describe the transition:

```text
independent breakpoint agitation
    -> local correlation
    -> collective waveform motion
```

A second larger example may map the same state to a phase-view/debug visualization if Stochastica has an appropriate waveform/breakpoint display utility.

Do not require an external visualization package.

---

# Part XIV — General Stochastica Guide integration

## 82. Add a new conceptual family to `Stochastica.schelp`

The general guide must add a section such as:

```text
Dynamic stochastic synthesis / evolving waveform models
```

Explain in approximately this conceptual language:

> Earlier Stochastica classes expose random variables, stochastic trajectories, event processes, state machines, and nonlinear dynamical systems. The GENDY subsystem applies those ideas one level deeper: stochastic dynamics construct and transform the waveform itself.

Then distinguish:

```text
modulation approach:
stochastic process -> parameter of synthesizer

GENDY approach:
stochastic process -> synthesizer's waveform geometry
```

Link to `Guides/Stochastica-GENDY`.

---

## 83. Place the GENDY round in the existing Stochastica conceptual progression

The guide should extend the previous conceptual progression:

```text
Round 1:
What kinds of randomness and nonlinear dynamics can we generate?

Round 2:
How can stochastic agents interact, inhibit, synchronize, switch, and become metastable?

Round 3:
How can randomness acquire memory, hidden organization, geometry, transport, and spatial structure?

GENDY round:
How can those stochastic ideas generate and reorganize the sound waveform itself?
```

Do not treat this merely as version history. Use it to explain increasing levels of structural depth.

---

## 84. Conceptual categorization of the eight classes

Include this table in substance:

| Concept | UGen |
|---|---|
| first-order stochastic waveform motion | `Gendy1977` |
| stochastic motion of stochastic motion | `Gendy1991` |
| hierarchical/meta-parameter modulation | `GendyS709` |
| microstructure extended into macro activity fields | `GendyField` |
| stochastic stabilization as articulation | `GendyPerc` |
| interpolation geometry as an independent synthesis dimension | `GendyInterp` |
| physical diffusion/drift as waveform dynamics | `GendyDiffusion` |
| collective/correlated breakpoint dynamics | `GendyCoupled` |

This categorization is mandatory.

---

## 85. Explain relationships to existing Stochastica processes

The GENDY guide should cross-reference concepts, not merely class names.

Examples:

### `GendyDiffusion` and `OUProcess`

Both introduce stochastic trajectories with drift-like organization, but `OUProcess` outputs a scalar stochastic state while DDSS uses stochastic physical motion to shape a waveform.

Do not claim DDSS is OU unless the paper says so.

### `GendyCoupled` and `CoupledMapLattice`

Both investigate local interaction among many states. One evolves discrete map cells; the other correlates waveform breakpoints.

### `GendyField` and event processes

`GendyField` belongs conceptually near renewal/Bernoulli activity processes, but is historically motivated by PARAG's macrostructure.

### `GendyS709` and hierarchical stochastic systems

It demonstrates that stochastic synthesis can have parameters controlled at a slower organizational level.

### `GendyPerc` and metastability/stabilization

It shows that reducing stochastic freedom itself can create a perceptual gesture.

---

# Part XV — Dedicated `Stochastica-GENDY.schelp` structure

## 86. Required top metadata

Example:

```text
title:: Stochastica — Dynamic Stochastic Synthesis and the GENDY Family
summary:: Historical and conceptual guide to evolving stochastic waveform synthesis in Stochastica
categories:: Stochastica, UGens>Generators>Stochastic
related:: Guides/Stochastica, Classes/Gendy1977, Classes/Gendy1991, Classes/GendyS709, Classes/GendyField, Classes/GendyPerc, Classes/GendyInterp, Classes/GendyDiffusion, Classes/GendyCoupled
```

Do not add `See Also` at the end.

---

## 87. Required guide sections

Recommended order:

```text
1. What dynamic stochastic synthesis is
2. Why stochastic waveform synthesis is different from modulation
3. Historical timeline
4. Breakpoints and elastic barriers
5. 1977 first-order DSS
6. 1991 second-order DSS / GENDY3
7. PARAG and stochastic time fields
8. S.709 and parameter modulation
9. Current SuperCollider Gendy1-Gendy5
10. Post-Xenakis extensions
11. The Stochastica GENDY family
12. First-order vs second-order motion
13. Microstructure vs macrostructure
14. Interpolation geometry
15. Stochastic stabilization and percussion
16. Diffusion and physical stochastic models
17. Coupling and collective breakpoint dynamics
18. Choosing a GENDY class
19. References
```

A `References` section at the end is correct and encouraged.

---

## 88. Historical timeline to include

Only include dates/claims that are sourced.

At minimum:

- early stochastic-synthesis ideas and experiments as background;
- 1977 DSS / *La Légende d’Eer*;
- late-1980s return to stochastic synthesis;
- GENDY / GENDY3 in 1991;
- S.709 in 1994;
- Hoffmann's New GENDYN work;
- Brown/Jenkins IDSS and Brown's 2005 extensions;
- Luque's stochastic concatenation work;
- Rojas/Cádiz DDSS.

Do not turn the timeline into unsourced biography.

---

# Part XVI — Documentation requirements for every new class

## 89. Every helpfile must contain six kinds of explanation

### 1. One-sentence concept

Explain what organization the class adds.

### 2. Provenance

Historical reconstruction, historically informed, post-Xenakis extension, or new Stochastica extension.

### 3. Mathematics

Enough equations to understand the state update.

### 4. Perceptual/compositional interpretation

Explain what the mathematical difference means sonically/structurally.

### 5. Minimal examples

Immediately runnable.

### 6. One concept-driven creative example

Only as large as required to demonstrate the class's defining idea.

---

## 90. Examples must explain behavior, not genre

Avoid examples named only by genre labels such as:

```text
techno bass
IDM lead
ambient pad
```

Prefer concept names:

```text
First-order vs second-order persistence
Stochasticity freezing into periodicity
Overlapping activity fields
Identical breakpoints, different interpolation
Drift versus unbiased diffusion
Independent versus coupled breakpoint motion
```

The library is mathematical/compositional infrastructure, not a preset bank.

---

# Part XVII — Source and citation notes to preserve in the repository

## 91. Create an internal research note

Create:

```text
docs/research/GENDY_HISTORICAL_SOURCES.md
```

This is not the user-facing guide. It is a traceability document for maintainers and future agents.

For each historical claim include:

```text
Claim
Source
Exact page/section
Direct quote if needed
Confidence: high / medium / unresolved
Implementation consequence
```

Example:

```text
Claim:
1991 DSS uses second-order random walks.

Source:
Luque 2006, pp. 24–27; cross-check with Hoffmann and Xenakis Dictionary.

Confidence:
high.

Implementation consequence:
Gendy1991 requires primary and secondary amplitude/duration walks.
```

This file is mandatory because the user explicitly prioritizes non-hallucinated historical documentation.

---

## 92. Track unresolved historical questions

At minimum create entries for:

- exact RNG/distribution formulas of each historical implementation;
- exact initial conditions used in each work/program;
- exact S.709 parameter targets and modulation depths/rates;
- whether the S.709-era version retained second-order walks in every relevant path;
- exact PARAG voice counts/configuration for specific sections;
- exact interpolation variants in Brown's IDSS, especially any “square” curve formula;
- any historical integer/quantized state representation that materially changes behavior.

An unresolved question must never be silently converted into a fact.

---

# Part XVIII — Testing strategy

## 93. Three layers of tests

### Layer A — Mathematical unit tests

Test helpers independently:

- repeated reflection;
- distributions;
- interpolation;
- first-order update;
- second-order update;
- coupling Laplacian;
- modulation mapping;
- field-duration sampling.

### Layer B — Deterministic sequence tests

Use fixed synthetic random sequences and fixed seeds.

Golden tests should verify internal breakpoint traces rather than only audio checksums.

### Layer C — Statistical/behavioral tests

Offline scripts should inspect:

- distribution fit;
- field dwell means;
- Bernoulli occupancy;
- first-order vs second-order residence behavior;
- mobility collapse in `GendyPerc`;
- diffusion drift behavior;
- coupling roughness/correlation.

Do not make statistical tests so tight that they become flaky.

---

## 94. Audio invariants

For oscillator classes verify:

- finite output;
- no accidental DC explosion;
- cycle transition continuity;
- segment length never zero/negative;
- maximum update-rate behavior bounded;
- no illegal memory access for minimum/maximum point counts;
- reproducibility for seeded tests;
- no allocations after construction.

Do not enforce arbitrary `[-1,1]` output if a historical/modern model legitimately allows another range, but document expected amplitude range.

---

## 95. Performance tests

Benchmark at least:

```text
44.1 kHz
48 kHz
96 kHz
```

with:

```text
12 points
32 points
64 points
```

and several simultaneous instances.

Benchmark worst-case frequencies where breakpoint updates occur frequently.

`GendyField` should be benchmarked at 4, 16, and maximum voice count.

`GendyCoupled` requires special attention because coupling introduces O(N) work per update.

No class in this round should introduce O(N^2) work unless there is an explicit, benchmarked justification.

---

# Part XIX — Visualization and analysis tools

## 96. Add development plots, not GUI dependency

Create scripts that can render:

- breakpoint trajectories through time;
- first-order vs second-order walk traces;
- primary/secondary state pairs;
- waveform evolution across cycles;
- stochastic mobility decay;
- DDSS drift trajectories;
- neighbor correlation vs `ampCoupling`;
- field-activity raster for `GendyField`.

Use Python scripts for development/reference if appropriate.

Do not make Python a runtime dependency of the plugin.

All published plots must be generated from project code/data, not copied from papers.

---

# Part XX — Documentation examples to implement

## 97. Required concept examples

The following concept examples should exist somewhere across the Guide/helpfiles.

### A. First order vs second order

Same general ranges, `Gendy1977` and `Gendy1991` side by side.

### B. Frozen parameter

Demonstrate barrier/step settings that make one dimension nearly fixed while another moves.

### C. Hierarchical modulation

`GendyS709` slowly changes the mobility of the waveform.

### D. Micro/macro structure

`GendyField` gates several GENDY voices.

### E. Percussion without primary amplitude-envelope articulation

`GendyPerc` begins noisy/mobile and stabilizes.

### F. Same stochastic points, different interpolation

Four `GendyInterp` voices with same seed.

### G. Diffusion with drift

Compare zero drift and nonzero drift in `GendyDiffusion`.

### H. Collective breakpoints

Sweep `GendyCoupled` from independent to correlated motion.

---

## 98. Example code style

Minimal examples should look like this:

```supercollider
{ Gendy1991.ar * 0.1 }.play;
```

not:

```supercollider
(
var sig;
var out;
var gain;
sig = Gendy1991.ar;
gain = 0.1;
out = sig * gain;
out.dup;
)
```

unless the additional variables communicate a genuine conceptual structure.

Large examples may use many variables if the system itself is multi-level.

---

# Part XXI — Implementation order

## 99. Phase 0 — historical research gate

Before implementation, finish `docs/research/GENDY_HISTORICAL_SOURCES.md` sufficiently to answer:

- first-order recurrence;
- second-order recurrence;
- barrier semantics;
- distribution semantics;
- interpolation assumptions;
- S.709 evidence;
- PARAG field evidence;
- Brown percussion mechanism;
- DDSS equations.

No historical class should be coded while its core state equation remains unresolved.

---

## 100. Phase 1 — common engine

Implement and test:

- reflection;
- breakpoint storage;
- distribution interface;
- linear/cosine/Hermite/hold interpolation;
- segment scheduler;
- seeded RNG integration;
- reset.

No public UGen until common helper tests pass.

---

## 101. Phase 2 — historical core

Implement:

1. `Gendy1977`
2. `Gendy1991`

These establish the reference architecture.

Do not begin modern extensions until both have deterministic trace tests.

---

## 102. Phase 3 — historical hierarchy

Implement:

3. `GendyS709`
4. `GendyField`

Keep their historical limitations explicit.

---

## 103. Phase 4 — performance/timbre extensions

Implement:

5. `GendyPerc`
6. `GendyInterp`

Credit Brown and distinguish generalized Stochastica additions from historical IDSS features.

---

## 104. Phase 5 — physical and collective extensions

Implement:

7. `GendyDiffusion`
8. `GendyCoupled`

DDSS must wait until the full paper equation audit is done.

---

## 105. Phase 6 — documentation

Write/update:

- eight class helpfiles;
- `Guides/Stochastica-GENDY.schelp`;
- general `Guides/Stochastica.schelp`;
- research source notes;
- README class list if the repository mirrors Guide categories there;
- CHANGELOG.

Run SCDoc checks.

Search repository to ensure no touched helpfile has a bottom `See Also` section.

---

# Part XXII — Suggested commit / PR structure

## 106. Keep history reviewable

Suggested commits/PRs:

```text
Docs: add verified GENDY historical source dossier
GENDY: add shared breakpoint and reflection engine
GENDY: implement Gendy1977 first-order DSS
GENDY: implement Gendy1991 second-order DSS
GENDY: add S.709-informed parameter modulation
GENDY: add PARAG-inspired stochastic activity fields
GENDY: add Brown-inspired stochastic percussion gestures
GENDY: add explicit interpolation variants
GENDY: implement Rojas-Cadiz diffusion DSS
GENDY: add coupled breakpoint dynamics
Help: add Stochastica GENDY conceptual and historical guide
Tests: add GENDY traces, statistics, and RT stress coverage
```

Do not submit one giant opaque commit.

---

# Part XXIII — Definition of done

## 107. A class is not finished until all of the following are true

- server implementation complete;
- SC class wrapper complete;
- CMake/registration complete;
- no RT allocation after construction;
- deterministic seed/reset tests pass;
- mathematical reference tests pass;
- extreme-value finite tests pass;
- helpfile contains concept + provenance + math + minimal examples;
- historical claims have source references;
- direct quotes have been re-verified;
- no runnable code is commented out;
- comments appear only inside outer runnable `(...)` blocks;
- no bottom `See Also` section;
- `related::` metadata present near top;
- large example, if any, exists because it demonstrates a genuinely systemic concept;
- build succeeds for scsynth and, where supported by current CI, supernova;
- tests pass on supported platforms.

---

# Part XXIV — Future directions deliberately excluded from this round

## 108. Do not expand scope during implementation

The common engine should make future work possible, but do not implement these now unless a blocking architectural reason demands it:

- `GendyOU` — mean-reverting breakpoint processes;
- `GendyLevy` — stable/Lévy breakpoint increments;
- `GendyConcat` — Luque-style stochastic concatenation of multiple GENDYs;
- quantized/internal-bit-depth GENDY;
- spectral/frequency-domain GENDY;
- arbitrary graph/network breakpoint coupling;
- external buffer-defined initial shapes;
- user-defined distribution buffers;
- GPU implementation;
- automatic parameter inference from recordings.

Mention `GendyConcat` in the guide as an important published extension by Sergio Luque, but leave implementation to another round.

---

# Part XXV — Bibliography / source list for the agent

## 109. Core historical sources

### Xenakis

Xenakis, Iannis. *Formalized Music: Thought and Mathematics in Composition*. Revised English edition. Stuyvesant, NY: Pendragon Press, 1992.

Key areas reported by later scholarship:

- “Dynamic Stochastic Synthesis,” pp. 289–293;
- “More Thorough Stochastic Music,” pp. 295–322.

The agent must verify page content directly before quoting the book.

### Serra

Serra, Marie-Hélène. “Stochastic Composition and Stochastic Timbre: GENDY3 by Iannis Xenakis.” *Perspectives of New Music* 31, no. 1 (1993): 236–257.

### Robindoré

Robindoré, Brigitte. “Eskhaté Ereuna: Extending the Limits of Musical Thought—Comments On and By Iannis Xenakis.” *Computer Music Journal* 20, no. 4 (1996): 11–16.

### Hoffmann

Hoffmann, Peter. “The New GENDYN Program.” *Computer Music Journal* 24, no. 2 (2000): 31–38.

Hoffmann, Peter. “‘Something rich and strange’: Exploring the Pitch Structure of GENDY3.” *Journal of New Music Research* 33, no. 2 (2004): 137–144.

Hoffmann, Peter. “Stochastic Synthesis.” In *A Xenakis Dictionary*, edited by Dimitris Exarchos, 2023. Official Iannis Xenakis site: `https://www.iannis-xenakis.org/en/stochastic-synthesis` (language routing may redirect).

### Luque

Luque, Sergio. *Stochastic Synthesis: Origins and Extensions*. Master's thesis, Institute of Sonology, Royal Conservatory, The Netherlands, 2006. Author-hosted source available from `https://sergioluque.com/stochastics`.

Luque, Sergio. “The Stochastic Synthesis of Iannis Xenakis.” *Leonardo Music Journal* 19 (2009): 77–84. DOI: `10.1162/lmj.2009.19.77`.

### Brown / Jenkins

Brown, Andrew R., and Greg Jenkins. “The Interactive Dynamic Stochastic Synthesizer.” 2004.

Brown, Andrew R. “Extending Dynamic Stochastic Synthesis.” Proceedings of the International Computer Music Conference, 2005, 111–114.

Brown, Andrew R. “Gendynish: Stochastic Synthesis on the Arduino.” 2019.

### Rojas / Cádiz

Rojas, Emilio L., and Rodrigo F. Cádiz. “A Physically Inspired Implementation of Xenakis's Stochastic Synthesis: Diffusion Dynamic Stochastic Synthesis.” *Computer Music Journal* 45, no. 2 (2021): 48–66. DOI: `10.1162/comj_a_00606`.

---

## 110. Current SuperCollider implementation references

Current SuperCollider help:

- `https://doc.sccode.org/Classes/Gendy1.html`
- `https://doc.sccode.org/Classes/Gendy2.html`
- `https://doc.sccode.org/Classes/Gendy3.html`
- `https://doc.sccode.org/Classes/Gendy4.html`
- `https://doc.sccode.org/Classes/Gendy5.html`

Current sc3-plugins repository:

- `https://github.com/supercollider/sc3-plugins`

Inspect the actual source corresponding to the installed/current SC version rather than relying only on rendered help.

---

# Final instruction to the implementation agent

This round is not successful merely because eight UGens compile.

The goal is to make Stochastica's GENDY subsystem simultaneously:

- historically responsible;
- mathematically explicit;
- conceptually understandable;
- real-time safe;
- musically exploratory;
- clearly separated into reconstruction, later research, and new invention.

The documentation must make a reader understand **why** first-order movement, second-order movement, meta-modulation, time fields, stabilization, interpolation, diffusion, and coupling are different forms of organization.

When historical evidence is incomplete, say so.

When an idea comes from Xenakis, Serra, Hoffmann, Brown, Luque, Rojas, Cádiz, or another author, credit them.

When an idea is new to Stochastica, say that instead.

Never improve the story by inventing history.
