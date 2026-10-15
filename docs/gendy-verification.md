# GENDY verification

All eight planned classes are implemented and locally verified, continuing Stochastica from Round 3 commit `8db8cc0` on `gendy-waveform-synthesis`. The complete Rojas–Cádiz paper was supplied on 2026-10-14 and audited before the GendyDiffusion API was fixed. Its published equation and explicit numerical reconstruction choices are recorded in [the source audit](research/GendyDiffusion-source-notes.md). Nothing was pushed, published, tagged or released; no hosted workflow was started.

## Acceptance evidence

| Check | Result and scope |
| --- | --- |
| Native build | SuperCollider 3.14.1, macOS x86_64, Apple Clang 17, strict Release; scsynth and supernova bundles build without project warnings |
| Clean source archive | Both server bundles, all 12 suites, documentation lint and extension staging pass from a Git archive of implementation commit `a45f8b0` |
| Cross-build | Both bundles compile for macOS arm64; native arm64, Linux and Windows runtime remain untested |
| CTest | All 12 suites pass: eight earlier suites plus GENDY mathematical reference, statistical, DDSS equation/convergence and actual-UGen ABI tests |
| Sanitizers | All 12 suites pass with AddressSanitizer and UndefinedBehaviorSanitizer; host servers themselves are not instrumented |
| Independent oracle | Python integer-PCG and iterative-mirror implementation supplies 144 stored internal point records across first-order, second-order and coupled cases; all coordinates agree within stated floating tolerances. A second oracle supplies 204 DDSS radius/reflection/amplitude records at dimensions 1, 2 and 3 |
| Boundary math | Repeated positive/negative reflection, degenerate and extreme finite bounds, hand-computed first/second-order states, duration conversion, endpoint interpolation, cycle seam continuity and synchronous neighbor proposals |
| Rendering | Modes share identical breakpoint histories; interpolation changes latch at segment boundaries; hold deliberately jumps; smooth modes remain within endpoint bounds |
| Lifecycle | Constructors on deliberately nonzero raw storage, forced RT allocation failure, no callback RT allocation or leaked RT blocks, seeded replay, reset replay, held reset and audio-rate edges inside blocks |
| Seeds | Explicit and parent-derived seeds remain local during calculation/reset; voice-count expansion preserves the existing field streams |
| Extension limits | Zero coupling, linear interpolation, zero S709 depths and static zero-phase sine modulation exactly match the first-order baseline; constant percussion mobility matches the specified second-order engine |
| Percussion | Zero final ranges freeze the evolved coordinates; equal endpoints remain exactly constant; keep-state retriggers retain waveform/RNG history |
| Actual servers | Both servers render 25 integration channels, including eleven raw outputs, eleven seeded differences and three uncoupled/unmodulated reference differences; all finite, difference channels exactly zero |
| Stress | Both servers render ten seconds with 2/64-point waveform units, 32 dense field gates, rapid resets, high-frequency saturation and 100 extra construction/free cycles |
| Endurance | Both servers render 120 seconds at 48 kHz with no non-finite output: 5,760,064 scsynth frames and 5,760,000 supernova frames |
| Earlier behavior | Saved pre-GENDY plugins and new plugins produce sample-identical legacy 26-channel and Round 3 112-channel renders on both servers, including resets |
| Installed documentation | User extension installed; class library compiles; SCDoc.indexAllDocuments(true) and actual rendering of all 62 class pages and both guides finish without SCDoc warnings/errors |
| Runnable examples | All 19 new class/guide blocks execute on both servers, three seconds per block, with hardware output muted on a dedicated test server |
| Existing docs | Pre-GENDY class pages are unchanged; the general guide's runnable blocks are unchanged; all classes appear in the inventory and assets/internal links resolve |

Integration tests compare each server against itself; they do not assert last-bit equivalence between platforms. K2A interpolates the field gates, so rendered gates are checked in [0,1], not mistaken for a new continuous field model. An offline render does not establish a universal real-time deadline.

## Statistical results

All six innovation laws pass empirical-CDF checks against independent analytic CDFs at shapes 0.2 and 1, using 20,000 draws per case: maximum observed discrepancy **0.00807934**, below tolerance 0.018. Equal-mobility and independently frozen waveform dimensions have explicit coverage.

Fixed-seed tests run 100,000 waveform cycles (10,000-cycle burn-in) over 12 points. With the documented small-innovation configurations, second-order amplitude boundary residence is **4.59305 times** the first-order case. Coupling 0.25 reduces mean squared neighbor difference to **0.000512096** of the independent case in this experiment. These are configuration-specific checks of model behavior, not universal musical ratios.

A 500,000-tick field experiment requests mean 0.1 seconds and sound probability 0.3. Measured mean field duration is **0.099679 seconds**, activity occupancy **0.300584**, with a separate cross-voice independence check. Probability endpoints, dense scheduling and bounded work also pass. Consecutive equal gate values can span more than one field.

## DDSS equation and numerical checks

The hand-calculated equation tests cover outward drift, dimension-one zero drift, D=0 and dt=0 freeze, origin reflection, the denominator floor, repeated amplitude reflection, angular projection and exact zero-ended polygon interpolation. Independent Python records validate the PCG/Gaussian sequence and all radius/mapping states. Callback output matches the tested engine, including float control conversion; in-block reset, seed-zero isolation, allocation failure and no callback allocation are covered by the ABI harness.

A common-noise Euler refinement experiment uses 2,000 paths, D=0.1, d=3, initial radius 1 and simulation horizon 0.1, away from the origin singularity. Relative to a 256-step reference, the mean absolute endpoint error at 64 steps is **0.210934** of the 16-step error. This is a local convergence check, not proof of global accuracy for the regularized singular equation.

A separate 20,000-path ensemble gives radial second moment **1.05862**, versus the analytic Bessel value **1.06** at that horizon. The mean separation between dimension 3 and dimension 1 under common noise is **0.0200173**, consistent with outward drift. Six million rendered samples cover 2/64 particles, dimensions 1/2/16 and extreme valid D/dt/frequency controls, checking bounded audio and finite nonnegative physical radii. The actual-server endurance mix also includes DDSS.

The original DDSS figure shows mean/single-particle radial traces, a final waveform and spectra. Its simplified DSS baseline uses the same Gaussian increments, fixed spacing, reflection and headroom, but no radial drift, origin reflection or angular projection. This compares the reconstruction's mechanism; it does not claim to reproduce a copyrighted figure or the unavailable Max implementation.

## Callback timing

The benchmark invokes actual callbacks with four waveform instances, 64-sample blocks, and sample rates 44.1/48/96 kHz. Point counts are 12/32/64. Typical settings use default uniform innovations; stress settings use one-sample segments and hyperbolic-secant innovations where available, cosine rendering for GendyInterp, and uniform two-level percussion. DDSS uses fixed pitch at its maximum one-sample interval, D=10, d=16 and dt=0.01 in the stress case. Timing excludes construction, output validation, host routing and scheduling. The figures are a single local optimized x86_64 run, not portable guarantees. Cycle updates batch O(N) work at one boundary, so average cost is not peak callback latency.

| UGen | Typical 48 kHz / 12 points, ns per sample per voice | Largest measured ns/sample | Largest four-voice core share |
| --- | ---: | ---: | ---: |
| Gendy1977 | 5.82 | 138.57 | 5.32% |
| Gendy1991 | 6.81 | 155.63 | 5.87% |
| GendyS709 | 8.88 | 141.64 | 5.44% |
| GendyPerc | 7.14 | 43.79 | 1.68% |
| GendyInterp | 5.86 | 141.47 | 5.43% |
| GendyCoupled | 5.77 | 138.46 | 5.26% |
| GendyDiffusion | 9.12 | 42.17 | 1.49% |

Field timing covers 4/16/32 voices at control rates corresponding to all three audio rates, with ordinary and minimum mean duration. Full measured rows, including field costs, are in [gendy-benchmark.csv](gendy-benchmark.csv). Dense fields process at most eight boundaries per tick and retain time debt; do not read that cap as eight audible gate transitions.

## Reproduction

Build from the repository root with a matching SuperCollider source/SDK tree:

```sh
cmake -S . -B build-gendy -DSC_PATH=/path/to/supercollider   -DCMAKE_BUILD_TYPE=Release -DSTRICT=ON -DSUPERNOVA=ON -DBUILD_TESTING=ON
cmake --build build-gendy --parallel
ctest --test-dir build-gendy --output-on-failure
cmake --install build-gendy --prefix /path/to/SuperCollider/Extensions
python3 tests/server/check_gendy_docs.py --export build-gendy/examples
```

For sanitizers, configure another build with `SD_SANITIZE=ON`, `SCSYNTH=OFF`, `SUPERNOVA=OFF`, and `BUILD_TESTING=ON`, then run its CTest suite. Independent fixtures can be regenerated with `python3 scripts/gendy_reference_oracle.py` and `python3 scripts/gendy_diffusion_oracle.py`.

Create a language YAML configuration that includes core SC and the installed Stochastica extension. Run the actual server suite with executable paths and a plugin search path containing core plugins and the freshly staged/installed Stochastica folder:

```sh
python3 tests/server/run_gendy_checks.py   --sclang /path/to/sclang --scsynth /path/to/scsynth --supernova /path/to/supernova   --config /path/to/sclang.yaml --plugins /path/to/core/plugins:/path/to/Stochastica   --output build-gendy/server --endurance 120
sclang -D -l /path/to/sclang.yaml tests/server/gendy_examples.scd   build-gendy/examples /path/to/Stochastica /path/to/scsynth
```

Repeat the example command with supernova. Its dedicated port defaults to 57239; an optional fourth script argument changes the port. The hardware output is muted, and it does not stop an existing default server. The plugin search-path separator above is for Unix-like systems.

Figures and timing are optional development targets:

```sh
cmake --build build-gendy --target sd_gendy_analysis sd_gendy_diffusion_analysis sd_gendy_benchmark
build-gendy/sd_gendy_analysis > build-gendy/analysis.csv
python3 scripts/plot_gendy_analysis.py build-gendy/analysis.csv
build-gendy/sd_gendy_diffusion_analysis > build-gendy/diffusion-analysis.csv
python3 scripts/plot_gendy_diffusion.py build-gendy/diffusion-analysis.csv
build-gendy/sd_gendy_benchmark > build-gendy/benchmark.csv
```

The plotting script needs numpy and matplotlib; synthesis does not. CSV includes cycle/index, primary and secondary coordinates, rendered segment duration and audio. Eight original figures illustrate point evolution, persistent increments, internal modulation, mobility collapse, interpolation/spectra, neighboring correlations field activity and DDSS dynamics/spectra. No paper figures are redistributed. Generated audio, downloads, logs and builds stay ignored.

## Upstream audit fingerprints

Read-only sc3-plugins `main` downloads on 2026-10-13, SHA-256:

- BhobNoise.cpp: `4c2004e32c0de3e9923e658a9a9656ae6168ff3e0b899bb2f275c171e7180f61`
- Gendy4.schelp: `2a66ba5e0a6f49e116c54466c3930768424ef8b517fa4f6e72af560e77651a46`
- Gendy5.schelp: `f29c22a1b0437ff0869e66e64c45ad39eef66ff4c5be2afe47621584e073846a`

SC core source/help was audited at 3.14.1. Historical-source confidence and attribution are in [the research dossier](research/GENDY_HISTORICAL_SOURCES.md). The existing expansion workflow now includes the GENDY tests and an architecture-labelled preview artifact name; it remains unexecuted remotely.
