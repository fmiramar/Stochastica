# Round 3 implementation record

The supplied [plan](round3-plan.md) extends local Round 2 commit `3d8bb06` on
`round3-memory-and-structure`. Work remains local: no push, PR, or release.
Before edits, strict Release builds for both servers and all five existing CTest
suites passed. Existing server binaries were retained for audio regression checks.

## Repository audit

Round 2 is present. Gamma, normal, Poisson, and stable samplers already have shared
implementations in `SDRandom.hpp` / `SDDistributions.hpp`; no sampler extraction is
needed. `SDFlow.hpp` supplies fixed-size RK4, and Round 2 supplies safe snapshots,
open double uniforms, and stable hazard probabilities. The only necessary legacy
code extraction is categorical selection from `StateProcesses.cpp`: retain its
operation order, uniform conversion, zero-row behavior, and RNG consumption. Test
old Markov/SemiMarkov output against pre-extraction server renders.

Existing metadata already separates raw maps, flows, stochastic processes,
distributions, and state models. The guide needs richer behavioral explanations;
metadata changes should be limited to inconsistencies, chiefly Round 2 categories.
Keep all old runnable examples. New See also subsections belong before methods,
as required by Round 3, alongside top-level `related::` metadata.

| Class | Source | Class file | Mathematical role |
|---|---|---|---|
| TDirichletRand | distributions/TDirichletRand.cpp | SDTDirichletRand.sc | Gamma-normalized allocation |
| PolyaUrn | state/PolyaUrn.cpp | SDPolyaUrn.sc | Additive reinforcement |
| MarkovModulatedTrig | processes/MarkovModulatedTrig.cpp | SDMarkovModulatedTrig.sc | Latent-state event intensity |
| MaternNoise | processes/MaternNoise.cpp | SDMaternNoise.sc | Exact half-integer Matérn Gaussian motion |
| LevyFlight | processes/LevyFlight.cpp | SDLevyFlight.sc | Symmetric stable transport |
| JumpDiffusion | processes/JumpDiffusion.cpp | SDJumpDiffusion.sc | Gaussian diffusion plus compound Poisson shocks |
| WilsonCowan | flows/WilsonCowan.cpp | SDWilsonCowan.sc | Deterministic population feedback |
| Lorenz96 | flows/Lorenz96.cpp | SDLorenz96.sc | Deterministic spatial flow |

## Mathematical decisions before implementation

- Dirichlet draws reuse bounded Gamma sampling. Subunit shapes use its
  shape-raising identity in log space, then subtract the largest log variate
  before normalization. This preserves corner concentration when ordinary Gamma
  values underflow. Invalid alpha sets produce uniform weights; reset uses the
  concentration mean and consumes no draw. Construction follows that reset rule.
- Pólya selection uses pre-reinforcement probability and post-reinforcement
  entropy. Invalid base weights fall back to equal weights. Rescale weights above
  total `2^40`, tracking a common unit scale that also scales every future
  reinforcement. Rescaling weights alone would change the additive urn model.
- Markov modulation draws event and opportunity decisions from the old state,
  then samples its transition row. Intensity reports that old state's capped rate;
  stateIndex reports the resulting state. Invalid snapshots hold a silent state.
- Matérn uses derivative coordinates `[y, y'/lambda, y''/lambda^2]` as needed,
  `lambda=sqrt(2*nu)/correlationTime`, and dimensionless time `u=lambda*dt`.
  The companion generator B has `(B+I)^p=0`, so its transition is the finite
  polynomial `exp(-u)*sum(u^k*(B+I)^k/k!)`, with p=1,2,3. The noise covariance
  integrates the exponential-polynomial impulse response exactly. Small u uses
  scaled moments and a bounded convergent series to avoid cancellation; the
  existing Cholesky routine factors matrices of at most 3x3. Stationary covariance
  in these coordinates is variance times `[1]`, `diag(1,1)`, or
  `[[1,0,-1/3],[0,1/3,0],[-1/3,0,1]]`. Coefficients are cooked once per block;
  correlation-time changes preserve physical derivatives. Reset zeroes derivatives.
- Lévy increments use the existing symmetric S0 sampler, alpha floor 0.01,
  and `scale*dt^(1/alpha)`. Its alpha=2 convention has variance `2*scale^2*dt`.
  Double state is preserved without musical clipping; float serialization uses
  the existing finite conversion at machine limits. Non-finite updates reset
  state and emit zero auxiliaries while retaining the runtime RNG position.
- Jump diffusion draws one Poisson count and aggregates Gaussian marks with
  `K*jumpMean + sqrt(K)*jumpStd*Z`, plus an independent Gaussian diffusion draw.
  No per-jump loop is needed. Unsupported extreme count means must be rejected
  before the existing uint32 sampler; document the numerical limit explicitly.
- Wilson–Cowan uses the requested normalized logistic equations (not the full
  refractory-factor model in the original paper), stable logistic evaluation,
  and existing fixed-substep RK4. Time constants floor at 1e-6 model seconds.
- Lorenz–96 uses the requested cyclic nearest-neighbor derivative and O(N) RK4
  stages. Reload changes only the reset template. Default resets use block-sampled
  forcing plus a 0.01 perturbation of cell zero.

All new constructors initialize every member explicitly because SC allocates raw
unit storage. Reset wins over reload/trigger and still consumes their edge histories.
Buffer locks are held while validating and copying, never nested. Runtime loops
are bounded by dimensions (32/64), substeps (16), or existing sampler attempts (64).

## Reference verification

Matérn amplitudes and companion form were checked against Särkkä and Solin,
*Applied Stochastic Differential Equations*, section 12.3, equations 12.33–12.35
and example 12.7 ([author copy](https://users.aalto.fi/~asolin/sde-book/sde-book.pdf)).
The dimensionless transition and small-step covariance evaluation above are
independently derived from that linear SDE.

Other definitions were checked against the
[GSL Dirichlet reference](https://www.gnu.org/software/gsl/doc/latex/gsl-ref.pdf),
[Pemantle's reinforcement survey](https://arxiv.org/abs/math/0610076),
[Fischer and Meier-Hellstern's MMPP paper](https://doi.org/10.1016/0166-5316(93)90035-S),
[Nolan's stable-distribution chapter](https://edspace.american.edu/jpnolan/wp-content/uploads/sites/1720/2020/09/Chap1.pdf),
[Siegrist's compound-Poisson derivation](https://www.randomservices.org/random/poisson/Compound.html),
[Wilson and Cowan](https://doi.org/10.1016/S0006-3495(72)86068-5), and
[Lorenz's predictability paper](https://www.ecmwf.int/sites/default/files/elibrary/1995/10829-predictability-problem-partly-solved.pdf).
No external source code or paper text is copied into runtime implementations.

## Status

Implementation and verification are in progress. Results and numerical limitations
will be recorded separately from any unrun hosted-platform checks.
