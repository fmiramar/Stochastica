# Implementation log

Date: 2026-07-29

Created `04-birosca/StochasticDynamicsUGens` as a standalone BiroSCa server plugin from `../SC_Stochastic_Dynamics_New_UGens_Implementation.md`. No legacy chaos, noise, Brownian, Gaussian, Gendy, or demand-rate class was edited.

## Work completed

- Added a C++17 CMake project using SuperCollider's server-plugin helper and one plugin entry point.
- Split server code into maps, flows, processes, distributions, state processes, and explicit sonifiers.
- Added header-only PCG32, trigger, range, map, flow, distribution, buffer, and server helpers.
- Used double recurrence state, float outputs, finite restoration, and fixed calculation bounds.
- Used RT allocation only in constructors for Mackey–Glass, fractional Brownian motion, lattice, and multivariate Gaussian storage.
- Used shared server buffer locks without retaining data pointers across calls.
- Defined seed conversion as `uint32(abs(trunc(seed)))`; reset restores each local generator.
- Capped rejection loops at 64 attempts.
- Shared raw recurrence functions between maps and sonifiers.
- Added classes, one helpfile per UGen, one project guide, source notes, and layered tests.
- Added repository-owned Linux x64, macOS x64/arm64, and Windows x64 CI staging with owner-prefixed architecture artifacts.
- Added a Linux AddressSanitizer/UndefinedBehaviorSanitizer CI job for the standalone reference and statistical tests.
- Removed machine-local SDK defaults: configuration now requires an explicit matching `SC_PATH`.

Chua uses RK4; Izhikevich uses the canonical two-half-step voltage update; Mackey–Glass uses cubic delay interpolation and Heun integration; OU uses its exact finite transition; fractional noise uses a normalized OU bank; finite-memory fBm uses normalized fractional coefficients; and multivariate Gaussian uses bounded-jitter Cholesky factorization.

The final local pass built both scsynth and supernova plugin variants. Because the exact local SuperCollider 3.14.1 source snapshot lacked its `nova-tt` submodule, the supernova build used a temporary copy of that snapshot supplemented only with `nova-tt` from another complete local SuperCollider checkout; neither upstream checkout was modified.

The final audit found 37 registered server units, 37 language classes, and 37 filename-matched class helpfiles. It removed the remaining delay-index `while` loops, applied strict warnings to both C++ test targets, compiled every documented language constructor, reran deterministic and stress NRT renders on scsynth and supernova, rendered all class pages plus the guide, and confirmed that the staged and installed extension trees match.

Recorded commands, exact render results, and the remaining release-platform and endurance work are maintained in `verification.md`.

## Visualization prototype — 2026-07-31

- Added a reproducible static Chua state-space figure with one oblique 3D view and the three orthogonal phase-plane projections.
- Added a runnable Chua help example that sonifies and visualizes the same UGen state in real time.
- Limited state transport to 30 `SendReply` messages per second and trajectory storage to 1,200 language-side points; no GUI work occurs on the server audio thread.
- Added `docs/visualization.md` to record the appropriate visual grammar for every class family and the cases where animation would be misleading or redundant.
- Ran the exact help block against local scsynth: the server booted, the animated window drew, the sonification ran, and the scheduled window-close cleanup completed without example errors.
- Replaced the initial dark composite documentation graphic with a light-theme, text-free oblique view and one wide horizontal figure containing the `xy`, `xz`, and `yz` projections. Figure titles and axis descriptions use native SCDoc captions; the successful live animation was intentionally left unchanged.

## Visualization atlas — 2026-07-31

- Restored coordinate arrows to Chua's oblique view, using restrained red, green, and blue directions for x, y, and z. Combined its three orthogonal projections into one horizontal figure and retained the existing live animation byte-for-byte.
- Added `SDPhaseView`, a reusable language-side exact-state phase viewer. It bounds trail memory, sends only at display rate, draws outside the audio thread, passes the same UGen state to visualization and sonification, and owns window/Synth/OSC cleanup.
- Added runnable `SDPhaseView` examples to all ten raw two-state maps and Izhikevich. CircleMap, CoupledMapLattice, and Mackey–Glass deliberately use a cobweb, space-time field, and time/delay view instead of artificial two-coordinate animations.
- Added text-free, light-theme deterministic figures for ten two-state maps, CircleMap, CoupledMapLattice, Mackey–Glass, and Izhikevich. Discrete-map orbits are drawn as points, not misleading continuous curves.
- Added process-specific figures for OU, Renewal, Hawkes, FractionalNoise, FBrownianMotion, and BoundedWalk; state/covariance figures for MarkovChain, SemiMarkov, and MultiGaussNoise; and density or probability-mass figures for all ten triggered distribution classes.
- Added sonifier documentation that links to the corresponding raw-map visualization while explaining that an independently clocked map is not the sonifier's hidden internal orbit.
- Added three dependency-free standard-library renderers. Together they generate 35 PNG assets with 2x supersampling where appropriate; explanatory labels and color mappings remain in native SCDoc captions rather than inside the images.
- Updated the project guide, README, visualization design note, build installation asset glob, SCDoc verification list, and dedicated phase-view/phase-example smoke checks.
- Installed the complete atlas and `SDPhaseView` into the SuperCollider user extension. Removed only the three obsolete installed Chua `xy`, `xz`, and `yz` PNGs after replacing them with the horizontal projections asset.
