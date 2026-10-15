# GendyDiffusion — full-paper equation audit

Emilio L. Rojas and Rodrigo F. Cádiz, *A Physically Inspired Implementation of Xenakis’s Stochastic Synthesis: Diffusion Dynamic Stochastic Synthesis*, **Computer Music Journal 45(2), 48–66**, issue dated 2021, copyright 2022. [DOI](https://doi.org/10.1162/comj_a_00606).

The user supplied the complete 19-page article on 2026-10-14, resuming the class deferred on 2026-10-13. All pages were read, with equations 19 and 20 also checked in the rendered PDF. The supplied article and extracted text are private research inputs, excluded from distribution. This audit precedes the API and implementation.

## Published model (printed page numbers)

- **pp. 53–56, equations 1, 4, 8, 12–19:** ordinary isotropic diffusion with diffusivity D is transformed into a radial Bessel process. Equation 19 is `dr = D(d−1)/r dt + sqrt(2D) dW`. Here d is spatial dimension, D is diffusivity, and W is a standard Wiener process (increment variance dt, p. 54). This is neither OU mean reversion nor constant drift.
- **p. 56, figures 3–4:** Euler–Maruyama solves equation 19. The illustrative paths start near r=1; figure 3 uses D=1, d=3. The singularity at r=0 is acknowledged but no numerical treatment is given. The mean of 10,000 paths demonstrates outward tendency.
- **p. 57:** synthesis uses n independent paths, D=1, d=2, and 1,500 steps. There is **no numerical timestep or explicit initialization instruction**. The figures’ starting radius motivates, but does not establish, our initial radius of 1.
- **p. 57, equation 20:** `phi[k] = 2*pi*k/n` with zero-based k. Coordinates are `x=r*cos(phi)`, `y=r*sin(phi)`. y gives amplitude; 0.1*x gives the amplitude of an additional oscillation around y. Its waveform, rate and phase are **not specified**.
- **pp. 57–58, equation 21:** two zero endpoints surround the n particles. Four inserted linear interpolation samples between each pair give `s=5*(n+2)−4` table samples. A phasor reads the waveform at pitch nu. After each complete cycle the next solution step supplies the amplitudes. Temporal breakpoint positions are fixed (also p. 59).
- **p. 60, table 2, step 1:** solution values beyond ±1 are reflected back. It does **not say whether this is a stored/output copy or feedback into the numerical solver**, nor define repeated overshoots. We use output-copy reflection, consistent with generating paths before reading them (p. 51); the physical radius keeps its own state.
- **pp. 58–59, equations 22–23:** a separate additive oscillator-bank application is described. It is outside this waveform UGen’s scope. The apparent sign inconsistency in equation 23 does not affect this implementation.
- **pp. 59–62:** compare with simplified amplitude-only DSS driven by the same Brownian increments. Their spectral observations depend on their settings and mapping; they are not universal guarantees. The angle range written on p. 62 conflicts with equation 20; we follow the explicit full-circle equation and figure 6.
- **pp. 64–65:** variable D and d are proposed controls. Fractional d is a mathematical Bessel extension, not a literal fractional physical space.

## Companion implementation audit

The [authors’ site](https://diffusiondss.github.io) currently contains only a welcome message. [DiffusionDSS/DDSS](https://github.com/DiffusionDSS/DDSS/tree/c431f41b6d94de69d32ca46154aadaf795b7f792), inspected 2026-10-14, contains only a two-line README at commit `c431f41b6d94de69d32ca46154aadaf795b7f792`; no Max patches or solver code resolve the omissions. No code or figures were copied.

## Explicit real-time reconstruction choices

This class implements the **published DDSS waveform concept**, not a bit-exact recreation of the authors’ Max instrument. The following choices are ours, not claims about the paper:

1. Initialize every physical radius to 1. Advance once per waveform cycle, starting with the initial shape before the first update. Independent standard normals use two open-uniform PCG32 draws per particle, Box–Muller cosine branch; no cached second variate. Draws are retained even when D or dt is zero.
2. Use the paper’s Euler–Maruyama increment `r + D*(d−1)*dt/max(r,1e−6) + sqrt(2*D*dt)*z`. Reflect a negative proposed radius at zero using absolute value. The denominator floor and origin reflection regularize the singular SDE; they change behavior near zero and prevent negative physical radii. They are not an exact Bessel transition sampler. With d=1 the drift vanishes, leaving an origin-reflected Wiener path. D=0 or dt=0 freezes radii exactly.
3. Fold a **copy** of each radius repeatedly into [−1,1] for table-2 amplitude reflection. Do not feed this copy back into equation 19. Thus a negative mapped value is not a negative physical radius. Unbounded physical radius is allowed; valid controls give finite increments, and tests cover long runs and the singularity.
4. Choose a common sine for the unspecified x modulation, with phase initially zero and user-controlled `modFreq`. Use `amp=(q*sin(phi)+0.1*q*cos(phi)*sin(modPhase))/sqrt(1.01)` where q is the reflected copy. Fixed headroom bounds all amplitudes by 1 without clipping. `modFreq=0` suppresses this modulation from reset.
5. Render the same straight lines analytically, rather than materializing the four redundant intermediate table samples. The n moving points plus two zero endpoints define n+1 equal intervals per cycle. This preserves the polyline, while avoiding assumptions about Max buffer-index interpolation. No stochastic duration walk is added.
6. Read freq, D, d, dt and modFreq at cycle boundaries. Reset is sample-accurate and rewinds radii, phases, remembered amplitudes and the private generator. Bound work to one cycle update per sample; at most 64 particles, allocated once on construction. No external buffer, simulation preload, additive bank, or 1,500-step playback loop.

## Frozen API after the audit

`GendyDiffusion.ar(freq=220, diffusivity=1, dimensions=2, timeStep=0.001, modFreq=0.2, numPoints=36, reset=0, seed=0)`.

- freq: Hz, clipped to [0.001, sampleRate/(numPoints+1)].
- diffusivity: D, [0,10], squared radius units per simulation-time unit.
- dimensions: d, [1,16], real valued; d=1 removes the drift without removing diffusion.
- timeStep: dt, [0,0.01], simulation-time units per waveform cycle. It is **not** audio sample duration. Physical evolution per wall-clock second consequently depends on pitch. Smaller values improve local Euler resolution; large D*d*dt near zero is a numerical effect, not evidence of physical accuracy.
- modFreq: common x-modulation sine rate in Hz, [0, sampleRate/2], sampled into the shape once per cycle; use much less than freq to avoid modulation aliasing.
- numPoints: construction-time particle count, integer [2,64], excluding the two zero endpoints. Default 36 follows the paper’s example. With only two particles and zero x modulation, the full-circle y projection is degenerate (effectively silent).
- reset/seed: the GENDY family rising-edge reset and private seed contract.

The provisional constant `drift` control is replaced by `dimensions`, and `diffusion` by physical `diffusivity`. New tests must check the bulk equation, origin and repeated-output reflection, angular mapping, zero limits, coupled-noise timestep refinement, independent reference traces, long-run finite state, and a newly generated comparison with a simplified DSS baseline.
