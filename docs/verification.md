# Verification record

Date: 2026-07-29

Generated build trees and renders are ignored.

## Build and reference tests

```bash
cmake -S . -B build -DSC_PATH=/path/to/supercollider-source \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DSTRICT=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Result: both `sd_reference_tests` and `sd_statistical_tests` passed in the strict release build and again in a local AddressSanitizer/UndefinedBehaviorSanitizer debug build. The strict build completed without project warnings. The statistical suite covers distribution moments, renewal means, OU mean/variance/autocorrelation, subcritical Hawkes rate, correlated Gaussian covariance, and fractional-increment persistence ordering.

The CI definition also runs the sanitized targets on Linux. That remote job has been configured but was not executed by this local verification pass.

## SuperCollider checks

The final pass recompiled the class library, serialized `.ar` and `.kr` SynthDefs, ran deterministic non-real-time renders, exercised malformed buffers and dense resets, and rendered every changed SCDoc page.

Results:

- Class library: all 37 public classes compiled; every documented rate and variable output-count constructor serialized.
- scsynth smoke: 96,064 frames at 48 kHz, finite non-silent output, peak 0.133063.
- scsynth deterministic pairs: 96,064 frames, exact zero difference across all seeded classes before and after repeated reset.
- scsynth stress: 192,064 frames, finite output, peak 0.06. It used sample-dense triggers, repeated reset, a 64-cell lattice, 32-channel Gaussian output, quality 32, fBm memory 4096, and malformed/missing buffers.
- supernova smoke: 96,000 frames at 48 kHz, finite non-silent output, peak 0.133057.
- supernova deterministic pairs: 96,000 frames, exact zero difference.
- SCDoc: all 37 class pages and the project guide indexed, parsed, and rendered with no SCDoc warning or error.
- Install parity: the complete staged and user-installed extension trees matched; both installed plugin binaries matched their staged SHA-256 hashes.

The maximum-size stress graph needs more than scsynth's default 64 interconnect buffers, so its command explicitly uses `scsynth -w 2048`. This is server graph capacity, not UGen memory allocation.

## Release-platform status

- macOS scsynth: strict build and NRT verification passed.
- macOS supernova: strict build and NRT verification passed.
- Linux: not yet validated.
- Windows: not yet validated.

The local maximum-parameter stress render is four seconds. A ten-minute endurance render remains required before treating the specification's full performance checklist as complete.

## Visualization atlas update — 2026-07-31

The atlas was checked separately because it adds language-side GUI behavior and binary documentation assets without changing server DSP.

- `python3 -m py_compile` passed for `plot_chua_state_space.py`, `plot_deterministic_dynamics.py`, and `plot_stochastic_dynamics.py`.
- All three scripts completed from the project root and reproduced 35 RGB PNG assets using only Python's standard library. The final set is two Chua images, fourteen deterministic map/flow images, and nineteen stochastic/process/state/distribution images.
- Visual inspection covered the final Chua axis/projection layout, discrete and dense map portraits, cobweb, lattice, delay/neuron panels, process/event figures, path and boundary comparisons, transition/covariance figures, continuous densities, discrete mass plots, and heavy-tail truncation/smoothing. The images use a light plot-like palette, restrained teal/blue/plum accents, and no embedded text.
- The Chua real-time subsection at `Chua.schelp` lines 69–212 retains SHA-256 `3de75c54abba1c6cc369f785a2f91b966642f8e3c5fdf0f9a4d58c3a655d2f7b`, confirming it remained byte-for-byte unchanged.
- `cmake --build build --parallel 4` completed after CMake refreshed the documentation asset glob. `ctest --test-dir build --output-on-failure` passed both reference and statistical tests.
- `sclang tests/server/compile_check.scd` compiled the installed SuperCollider class library and reported `STOCHASTIC_DYNAMICS_CLASS_LIBRARY_OK`.
- `sclang tests/server/phase_examples_compile.scd` serialized all eleven new live-example sound graphs and reported `STOCHASTIC_DYNAMICS_PHASE_EXAMPLES_OK`.
- `sclang tests/server/phase_view_smoke.scd` booted local scsynth, created a real Hénon `SDPhaseView`, received state and produced sound for two seconds, then verified that window, Synth, and OSC responder references were all cleared. It reported `STOCHASTIC_DYNAMICS_PHASE_VIEW_OK`.
- The staged extension and SuperCollider user extension each contain the final 35 PNG assets. The installed tree contains only `chua-state-space-3d.png` and `chua-state-space-projections.png`; the three obsolete separate projection PNGs were removed.
- `SCDoc.indexAllDocuments(true)` indexed 2,873 documents. All 38 class pages, including `SDPhaseView`, and the project guide parsed and rendered with no SCDoc warning or error. Rendered Hénon and Chua HTML were additionally checked for the expected image paths and native bold captions.
