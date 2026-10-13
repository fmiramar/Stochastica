# Round 3 verification

All eight Round 3 additions are implemented and locally verified against
SuperCollider 3.14.1. Work continues the existing Stochastica repository from
Round 2 commit `3d8bb06` on `round3-memory-and-structure`. This remains unreleased:
no push, PR, tag, hosted workflow or release was initiated. Linux/Windows runtime
and native arm64 runtime remain untested here.

## Acceptance results

| Check | Result and scope |
|---|---|
| Native build | macOS x86_64, Apple Clang 17, strict Release; both scsynth and supernova plugins build without project warnings |
| Cross-build | macOS arm64 bundles for both servers compiled; no arm64 runtime claim |
| CTest | All eight suites pass: original and Round 2 suites plus Round 3 reference, statistical and actual-UGen ABI tests |
| Sanitizers | All eight suites pass with AddressSanitizer and UndefinedBehaviorSanitizer; host servers themselves are not instrumented |
| ABI lifecycle | 1,600 create/free cycles, forced RT allocation failures, no calculation-time RT allocation or leaked RT blocks; constructors run on deliberately nonzero raw storage |
| Seeds and reset | Stored references for all eight classes at both rates; exact paired replay and constructor/reset replay; stochastic parent RNG isolation after construction, including seed=0 |
| Safety | NaN/infinite/extreme parameters, invalid/short/non-mono buffers, zero rows/weights, maximum dimensions, repeated reset/reload edges, held triggers and documented numeric recovery |
| Server integration | Both servers, .ar and .kr, 112 raw/reference-difference channels: all finite, seeded pair differences exactly zero, output ordering/invariants checked |
| Stress | Both servers render 10 seconds with 32-channel Dirichlet, 64-state urn/MMPP, 64-cell Lorenz96 at 16 substeps, eight simultaneous WilsonCowan instances at 16 substeps, extreme inputs, invalid buffers and 100 extra create/free cycles |
| Endurance | Both servers render 600 seconds at 48 kHz: 28,800,064 scsynth frames and 28,800,000 supernova frames; all raw model outputs pass CheckBadValues; graph produces nonzero output |
| Installed help | User extension installed; class library compiles; SCDoc.indexAllDocuments(true) and actual rendering of all 54 class pages plus the guide finish without warnings/errors |
| Examples | All 26 runnable blocks from the eight new pages and guide execute on both servers with core SC plus Stochastica, shortened cleanup waits and muted hardware output |
| Earlier examples | All 160 pre-existing runnable blocks are retained unchanged after removing comments/whitespace for comparison |
| Inventory and links | All 54 language classes have correctly named help pages and appear in the guide; related links are before methods/examples; local links and figure files resolve against installed core help |
| Legacy behavior | Gamma, stable, Poisson, MarkovChain, SemiMarkov, OU and MultiGauss .ar/.kr: all 26 channels match the saved pre-extraction plugins sample for sample on both servers |

Offline endurance uses the normal integration configurations: three allocation/
urn/MMPP states, Matérn smoothness=2, Levy alpha=1.3, jump diffusion, WilsonCowan
with four substeps, and eight Lorenz cells with four substeps, at both rates.
Maximum supported dimensions are exercised by the separate stress test and ABI
tests. These renders establish stability of the tested configurations, not a
universal real-time deadline or proof that all finite inputs give useful dynamics.

The WAV checker samples control-output invariants at block boundaries because
K2A interpolates between control values. The interpolation ramps are not UGen
state indices or event counts. Seeded pair differences are checked on every
sample. The example runner uses its own server port and leaves any existing
local server alone.

## Numerical evidence

- **Matérn:** exact transition/noise semigroup identities, stationary Lyapunov
  covariance identity and analytic autocovariance for all modes over intervals
  from 1e-12 to 100. Monte Carlo variances with target 2 were 2.0435, 2.0144 and
  2.0076 after burn-in; local increment variance fell from 0.04149 through
  0.000644 to 0.000356 as smoothness rose. Covariance is checked at three lags at
  both audio and control intervals. Zero variance consumes no random draws;
  block-rate coefficient sampling is covered. Correlation-time changes preserve
  physical derivatives in the implementation.
- **Dirichlet:** support and sum invariants, mean/variance/covariance expectations,
  tiny-concentration corner behavior and asymmetric mean share. For symmetric
  concentrations 0.2, 1 and 8, mean largest shares were 0.820, 0.612 and 0.435.
  This verifies sparse versus balanced behavior, not just one output mean.
- **Pólya urn:** pre-reinforcement probability, post-reinforcement entropy,
  zero-reinforcement categorical sampling, reset/reload and weight rescaling
  against an unscaled reference. Across 1,200 equal-start runs of 500 choices,
  mean share was 0.3341 with between-run variance 0.0582: the ensemble stays
  balanced while histories diverge.
- **Hidden regimes:** old-state event ordering, transition row frequencies,
  rate ceilings and zero-row behavior. Conditional measured rates were 399.8,
  1190.8 and 2350.0 events/second for requested 400, 1200 and 2400 at 48 kHz,
  compared against the actual finite-step hazard expectation. Residence times
  match the tested transition opportunities within sampling tolerance.
- **Lévy transport:** Gaussian alpha=2 normalization, symmetry, independent
  increment signs, heavy-tail quantile ratios and finite-time characteristic
  functions after one interval versus 64 subintervals. Q99/Q50 of magnitudes
  was 3.78 at alpha=2 and 29.26 at alpha=1.2. Finite double states beyond float
  range are retained internally; non-finite numeric updates recover explicitly.
- **Jump diffusion:** zero-rate and deterministic-mark limits, count/aggregate
  ordering, Poisson counts and mean/variance over equal total time. Measured
  displacement means 0.1511/0.1484 and variances 0.2910/0.2826 match targets
  0.15/0.289 for 64 versus one step. Unsupported count means produce the
  documented reset-to-initial recovery with no spurious jump diagnostics.
- **Wilson–Cowan:** independent derivative values, uncoupled analytic relaxation,
  stable logistic extremes, fourth-order convergence and .ar/.kr equal-time
  agreement. Figure settings with wEE=0 settle; wEE=16 gives sustained E range
  approximately 0.031..0.975 with inputExc=3, tauExc=0.02, tauInh=0.04, wEI=12,
  wIE=15, wII=3, thresholds 4/3 and unit gains. These are verified settings for
  this normalized variant, not universal bifurcation thresholds.
- **Lorenz–96:** independently indexed cyclic derivatives, rotation equivariance
  of derivatives and trajectories at 4/8/64 cells, uniform equilibrium,
  fourth-order convergence, .ar/.kr equal-time agreement and template-only
  reload. Non-finite updates restore the complete template, not individual cells.

Tests use fixed seeds and finite-sample tolerances. Stored float fixtures allow
small platform math-library rounding differences; within-build replay is exact.
Model limits are documented rather than hidden by musical normalization.

## Callback costs and limits

The benchmark calls actual UGen callbacks in blocks of 64 at 48 kHz or one
calculation at 750 Hz. It excludes construction, output validation, host
scheduling and audio routing. It checks that calculation does not allocate RT
memory. Values below are one strict Release x86_64 run in microseconds per
calculation, not portable performance guarantees.

| UGen/configuration | .ar typical | .ar large case | .kr large case |
|---|---:|---:|---:|
| TDirichletRand, 3 then 32 channels, alternating triggers | 0.072 | 0.784 | 0.794 |
| PolyaUrn, 3 then 64 states, alternating triggers | 0.030 | 0.333 | 0.340 |
| MarkovModulatedTrig, 3 then 64 states, dense opportunity rate | 0.024 | 0.055 | 0.149 |
| MaternNoise, smoothness=2 | 0.060 | 0.060 | 0.069 |
| LevyFlight, alpha=1.3 | 0.138 | 0.140 | 0.140 |
| JumpDiffusion, rate=5 then 100000 | 0.039 | 0.066 | 0.079 |
| WilsonCowan, 4 then 16 substeps | 0.530 | 2.104 | 2.101 |
| Lorenz96, 8 cells/4 steps then 64 cells/16 steps | 0.673 | 18.947 | 18.819 |

An earlier optimized callback run measured 21.47 microseconds for maximum
audio-rate Lorenz96. These runs project roughly 91–103% of one core at 48 kHz
before host overhead, versus roughly 1.4% at 750 Hz. Bounded cost does not imply
real-time viability at the largest audio-rate setting. Use control rate or fewer
cells/substeps when appropriate. Dense reloads additionally copy O(N squared)
MMPP matrices; Matérn recooks a matrix of at most 3x3 when block parameters change.
The benchmark measures ordinary callbacks, not worst-case simultaneous reloads
or a maximum-attempt rejection sequence. Reload/dimension cases are included in
safety/stress checks; sampler loops have an explicit attempt bound.

## Reproduction

Supply a matching SuperCollider source tree/SDK, keeping generated output ignored:

```sh
cmake -S . -B build-round3 -DSC_PATH=/path/to/supercollider-source \
  -DCMAKE_BUILD_TYPE=Release -DSTRICT=ON -DBUILD_TESTING=ON -DSUPERNOVA=ON
cmake --build build-round3 --parallel
ctest --test-dir build-round3 --output-on-failure
cmake --install build-round3 --prefix /path/to/SuperCollider/Extensions
```

For sanitizer coverage use a separate Debug build with `-DSD_SANITIZE=ON`.
For the offline tools:

```sh
cmake --build build-round3 --target sd_round3_benchmark sd_round3_analysis
build-round3/sd_round3_benchmark > build-round3/benchmark.csv
build-round3/sd_round3_analysis > build-round3/analysis.csv
python3 -m venv build-round3/plotvenv
build-round3/plotvenv/bin/pip install numpy matplotlib
MPLCONFIGDIR=build-round3/plot-cache build-round3/plotvenv/bin/python \
  scripts/plot_round3.py build-round3/analysis.csv
```

NumPy and Matplotlib are documentation-only dependencies. Final PNGs are tracked;
temporary CSV, executables and render audio remain ignored. The figures use
fixed seeds and the same kernels as the runtime, with explicit burn-in and raw
axis scales. Minor image-layout differences can follow font/library versions.

Create an sclang YAML configuration containing the core class library and the
installed Stochastica extension, with default paths excluded to isolate checks
from other extensions. Set explicit server/plugin paths:

```sh
python3 tests/server/run_round3_checks.py \
  --config /path/to/sclang.yaml --sclang /path/to/sclang \
  --scsynth /path/to/scsynth --supernova /path/to/supernova \
  --plugins /path/to/core/plugins:/path/to/build-round3 \
  --output build-round3/server --endurance 600
python3 tests/server/check_round3_docs.py \
  --core-help /path/to/HelpSource --export build-round3/examples
sclang -D -l /path/to/sclang.yaml tests/server/round3_examples.scd \
  build-round3/examples /path/to/build-round3 /path/to/scsynth
sclang -D -l /path/to/sclang.yaml tests/server/round3_examples.scd \
  build-round3/examples /path/to/build-round3 /path/to/supernova
```

The last example-runner argument may specify a dedicated UDP port if its default
57239 is occupied. Inspect completion markers and logs for errors/warnings.
Legacy preservation can be reproduced by building `3d8bb06` separately, compiling
`tests/server/round3_legacy_nrt.scd` and comparing each server's float-WAV samples
with the Round 3 build. Disable saved SynthDef loading (`-D 0`) in both renders.

The existing expansion CI workflow now builds/tests both rounds and stages
owner-prefixed, architecture-labeled artifacts. Its Linux x64, macOS x64/arm64,
Windows x64 and sanitizer jobs were edited locally but not run on GitHub.
