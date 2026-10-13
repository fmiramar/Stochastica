# Round 2 verification

All eight Round 2 UGens are implemented, documented, and locally verified against
SuperCollider 3.14.1. The expansion is unreleased. Hosted Linux, macOS, and Windows
CI remains an outstanding acceptance gate: no branch, PR, or release was published
during this work.

## Results

| Check | Result and scope |
|---|---|
| Native build | macOS x86_64, Release, strict warnings, scsynth and supernova; no project warnings |
| Cross-build | macOS arm64 scsynth and supernova bundles compiled; arm64 runtime not tested |
| CTest | All five suites passed: original reference/statistical tests and new reference/statistical/server ABI tests |
| Sanitizers | All five suites passed with AddressSanitizer and UndefinedBehaviorSanitizer |
| Server integration | Both servers rendered every new class at audio and control rates; 72 raw/reference-difference channels remained finite; matching-seed differences were exactly zero |
| Stress | Both servers passed a 10-second render with maximum dimensions and substeps, overloaded event rates, invalid buffers, reset/reload modulation, and 100 additional create/free cycles |
| Endurance | Both servers rendered 600 seconds at 48 kHz, with 28,800,064 scsynth frames and 28,800,000 supernova frames; all raw UGen outputs passed `CheckBadValues`, and the graph produced nonzero audio |
| Allocation and lifecycle | Actual new UGen constructors/calculators/destructors exercised through the SC ABI test harness: 1,600 create/free cycles, forced allocation failures, no calculation-time RT allocation and no leaked RT blocks |
| Help | Installed class library compiled; `SCDoc.indexAllDocuments(true)` and rendering of all eight class pages plus the guide completed without warnings or errors |
| Examples | All 31 runnable examples in the eight new pages and project guide compiled and ran with core SuperCollider plus Stochastica; shortened cleanup waits and muted hardware output were used for the smoke test |
| Existing behavior | Original smoke-test audio from the new and published-baseline plugins matched sample for sample on both servers |

The endurance graph uses the maximum 16 Hawkes streams, 32 OU dimensions, and 64
Kuramoto oscillators. Kuramoto runs at audio rate with one substep and at control
rate with 16 substeps; the shorter stress render additionally exercises 64
oscillators with 16 audio-rate substeps. Double-well integration uses 16 substeps.
These are offline renders, not proof that every combination meets a live deadline.
The ABI harness tracks the plugin's RT allocations; sanitizer checks cover the
test executables, not an instrumented build of the SuperCollider host.

## Numerical and statistical evidence

Tests use fixed seeds and tolerances for mathematical expectations. Stored output
fixtures cover every new class at both rates. Exact replay is checked within a
build; a small float tolerance accommodates platform math-library differences in
the stored fixtures.

- **Inhibition:** event-count Fano factor was 0.983 for the Poisson limit and
  0.089 with self-correction. The short-interval fraction also decreased from
  0.140 to 0.070, testing anti-clustering rather than simply a lower rate.
- **Regime switching:** asymmetric telegraph high-state occupancy was 0.651
  against an expected 2/3; mean dwell times were 0.507 and 0.947 seconds against
  0.5 and 1.0 seconds.
- **Shot noise:** measured mean 2.091 and variance 1.293 agree with analytic
  expectations 2.1 and 1.275 within the deterministic test tolerances.
- **Circular statistics:** resultant lengths at concentration 0, 1, 2, and 5
  were 0.0024, 0.4465, 0.6967, and 0.8938. Tests also cover wrapping, concentrated
  samples, and continuity near the wrapped-normal approximation boundary.
- **Correlated motion:** OU covariance was -0.335 against -1/3, with variances
  2.032 and 0.502 against 2 and 0.5. Tests cover exact innovation covariance,
  scalar equivalence, zero diffusion, the Brownian limit, singular jitter, and
  indefinite-matrix fallback.
- **Interaction:** one-way Hawkes coupling raised only the target rate (about
  1.96 to 3.49 events/second), while the source stayed about 1.95. Tests cover
  conditional post-event activity, diagonal coupling, simultaneous decisions,
  and target-row/source-column orientation.
- **Metastability:** the controlled low-noise run had no well crossings and
  the moderate-noise run had 367. Fixed points, deterministic convergence, and
  recovery from divergent updates are checked separately.
- **Synchronization:** mean order parameter increased from 0.108 uncoupled to
  0.999 coupled, and decreased to 0.744 with stronger diffusion. Tests compare
  mean-field updates with direct pairwise coupling and check phase/order bounds.

The ABI tests additionally check held triggers, reset priority, reset replay,
seed-zero isolation from the parent RNG after construction, malformed buffer
snapshots, and explicit reload semantics. In particular, valid Hawkes reloads
preserve history, OU reloads preserve current state, and phase-buffer reloads do
not teleport a running Kuramoto network.

## Cost and practical limits

An optimized x86_64 helper benchmark measured the following costs. These are local
measurements of the shared numerical kernels, excluding host scheduling, output
routing, and buffer locks. Rate percentages project the same per-calculation cost
to 48 kHz audio rate or 750 Hz control rate (64-sample blocks).

| Kernel | Mean microseconds/calculation | One core at 48 kHz | One core at 750 Hz |
|---|---:|---:|---:|
| Hawkes, 16 streams | 0.409 | 1.96% | 0.031% |
| OU, 32 dimensions | 0.663 | 3.18% | 0.050% |
| Kuramoto, 64 oscillators, 1 substep | 1.962 | 9.42% | 0.147% |
| Kuramoto, 64 oscillators, 16 substeps | 32.375 | 155.4% | 2.428% |
| Telegraph, overloaded rates | 0.076 | 0.36% | 0.006% |
| Shot noise, overloaded rates | 0.222 | 1.06% | 0.017% |

For 64-calculation batches, Kuramoto with 16 substeps took 2.72 ms at the observed
99th percentile and 8.91 ms at the maximum, against a 1.33 ms audio-block duration.
Use control rate, fewer oscillators, or fewer substeps for live work at this size.
OS scheduling affects these wall-clock measurements; they are not hard real-time
guarantees or measurements of other architectures.

OU factorization is excluded from its ordinary update cost. A 32-dimensional
positive-definite factorization averaged 3.88 microseconds; an indefinite matrix
that exercises all eight jitter attempts before diagonal fallback averaged 0.91
microseconds because each attempt fails early. Snapshot, covariance preparation,
and lock costs are additional. Reload deliberately and synchronize buffer edits.

Telegraph and shot scheduling process at most eight transitions/events per
calculation. Rates far beyond the calculation rate carry model time into later
calculations instead of increasing the loop count. Under sustained overload this
delays the model; deferred time uses the next calculation's parameters. Hawkes and
self-correcting events have at most one event per stream/calculation and use a
discrete hazard approximation. Raw states are not normalized or antialiased.

## Reproduction

Use a complete matching SuperCollider source checkout; supernova requires its
`external_libraries/nova-tt` submodule. Build out of tree:

```sh
cmake -S . -B build-round2 -DSC_PATH=/path/to/supercollider \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DSUPERNOVA=ON -DSTRICT=ON
cmake --build build-round2 --config Release --parallel
ctest --test-dir build-round2 -C Release --output-on-failure
cmake --install build-round2 --config Release --prefix /path/to/Extensions
```

For sanitizers, configure another build with `-DSD_SANITIZE=ON`, Debug mode, and
`-DSCSYNTH=OFF -DSUPERNOVA=OFF`, then build and run CTest there. No statistical or
plotting package is needed by these tests or by the plugin.

Use a language configuration including the SC class library and the installed
Stochastica extension. To isolate unrelated installed plugins during verification,
set `excludeDefaultPaths: true` and supply both include paths explicitly. The
runner requires the core and Stochastica server-plugin search paths as well:

```sh
python3 tests/server/run_round2_checks.py \
  --sclang /path/to/sclang --scsynth /path/to/scsynth \
  --supernova /path/to/supernova --config /path/to/sclang_conf.yaml \
  --plugins /path/to/core/plugins:/path/to/Stochastica \
  --output build-round2/verification --endurance 600
```

The runner performs the documentation lint/render, short server integration,
stress render, and optional endurance render, and validates each result. Omit
`--endurance` for the shorter checks. Use the platform's plugin-path separator.
Generated scores, logs, SynthDefs, and audio remain in the ignored output folder.
SCDoc renders the installed pages, so reinstall after changing documentation.

Run the documentation examples separately with an available local audio device:

```sh
python3 tests/server/check_round2_docs.py --export build-round2/examples
sclang -D -l /path/to/sclang_conf.yaml tests/server/round2_examples.scd \
  build-round2/examples /path/to/Stochastica
```

The example runner mutes output and checks server failures and bad values. Generate
the mathematical figures and repeat the kernel benchmark with a C++17 compiler:

```sh
c++ -std=c++17 -O2 -Iinclude scripts/export_round2_analysis.cpp -o build-round2/export
build-round2/export > build-round2/analysis.csv
python3 scripts/plot_round2.py build-round2/analysis.csv
c++ -std=c++17 -O2 -Iinclude tests/performance/round2_benchmark.cpp -o build-round2/benchmark
build-round2/benchmark
```

## CI and remaining platform gates

`.github/workflows/round2.yml` adds a separate verification matrix for Linux x64,
macOS x64/arm64, and Windows x64, plus Linux sanitizer checks. It builds from a
clean checkout, runs the C++ suites and documentation lint, stages the extension,
and names artifacts with the repository owner, project, preview version, platform,
and architecture. Windows builds scsynth; Linux and macOS also build supernova.
The workflow does not run the installed-language/server-render checks above.

This workflow has not run on hosted runners. It is configured for pull requests
and manual dispatch, with read-only repository permissions and no release step.
Existing build/release workflows were left unchanged. Hosted platform results,
Windows runtime verification, and arm64 runtime verification remain pending before
claiming the plan's full cross-platform definition of done.
