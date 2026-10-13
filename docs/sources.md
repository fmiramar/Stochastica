# Sources and technique notes

StochasticDynamicsUGens implements `../SC_Stochastic_Dynamics_New_UGens_Implementation.md`. Integration was developed against the matching SuperCollider 3.14.1 unit-generator guide, plugin-interface headers, server-plugin CMake helper, multi-output class pattern, RT allocation API, and shared `SndBuf` locks.

The module does not copy or modify legacy SuperCollider chaos or random UGen implementations.

## Mathematical sources

- Hénon: Michel Hénon, “A two-dimensional mapping with a strange attractor,” 1976.
- Ikeda: Kensuke Ikeda's optical ring-cavity recurrence.
- Lozi: René Lozi's 1978 piecewise-linear Hénon modification.
- Circle map: the normalized sine circle map used in mode-locking studies.
- Coupled lattice: periodic diffusive coupling of logistic-map images.
- Rulkov: Nikolai Rulkov, “Regularization of synchronized chaotic bursts,” 2001.
- Mackey–Glass: Michael Mackey and Leon Glass, “Oscillation and chaos in physiological control systems,” 1977.
- Chua: Leon Chua and collaborators' canonical piecewise-linear circuit equations.
- Izhikevich: Eugene Izhikevich, “Simple model of spiking neurons,” 2003.
- Ornstein–Uhlenbeck and Hawkes processes: their standard mean-reverting diffusion and self-exciting point-process definitions.
- Stable sampling: Chambers, Mallows, and Stuck's 1976 method, expressed in Nolan S0 form.
- Gamma sampling: Marsaglia and Tsang's 2000 method.
- Poisson sampling: Knuth's product method and Hörmann's PTRS transformed rejection.
- Inverse normal: Peter Acklam's rational approximation.
- PCG32: Melissa O'Neill's permuted congruential generator design.

The Gingerbreadman, Latoocarfian, Tinkerbell, Clifford, and Peter de Jong equations are written directly in their helpfiles because variants circulate. Help and server code use the same recurrence definitions.

## Adaptations

- Raw maps are trigger driven and contain no sonification.
- Continuous equations use double recurrence state and finite-state restoration.
- Rejection loops stop after 64 attempts and use finite fallbacks.
- Fractional noise is a stationary finite-band OU-bank approximation.
- Fractional Brownian motion is a finite-memory control-rate approximation.
- Sonifiers share raw recurrence functions but explicitly add folding, mapping, bounds, and interpolation.


## Round 2 sources and adaptations

All eight additions are original implementations of the stated mathematical
models, integrated with Stochastica's existing helpers. No source code from the
papers or an external simulation framework was copied into the plugin.

- **MultiHawkesTrig:** A. G. Hawkes, *Spectra of Some Self-Exciting and Mutually
  Exciting Point Processes*, Biometrika 58(1), 1971,
  [DOI](https://doi.org/10.1093/biomet/58.1.83),
  [paper](https://www.dcscience.net/Hawkes-Biometrika-1971.pdf).
  Exponential kernels, target-row/source-column orientation, simultaneous
  discrete event decisions, and an explicit intensity ceiling.
- **SelfCorrectingTrig:** V. Isham and M. Westcott, *A self-correcting point
  process*, 1979, [DOI](https://doi.org/10.1016/0304-4149(79)90008-5).
  The normalized API uses `rate * exp(correction*(rate*t-N))` before the explicit
  ceiling; its sampled hazard is a finite-calculation approximation.
- **RandomTelegraph:** S. Goldstein, *On diffusion by discontinuous movements,
  and on the telegraph equation*, 1951,
  [DOI](https://doi.org/10.1093/qjmam/4.2.129). The implementation exposes a
  two-state continuous-time Markov chain with independently controllable hazards,
  retaining integrated hazard when rates change.
- **ShotNoise:** N. Campbell, *The study of discontinuous phenomena*, Proceedings
  of the Cambridge Philosophical Society 15, 117–136, 1909,
  [archived volume](https://archive.org/details/proceedingsofcam15190810camb/page/117/mode/1up).
  For a Poisson random measure, integrating an exponential response and its square
  gives the stationary mean and variance used by the tests. Marks are Gaussian.
- **MultiOUProcess:** G. E. Uhlenbeck and L. S. Ornstein, *On the Theory of the
  Brownian Motion*, 1930, [DOI](https://doi.org/10.1103/PhysRev.36.823).
  Integrating `exp(-Theta*u) * Sigma * exp(-Theta*u)` from 0 to dt gives the exact
  finite-step innovation covariance for diagonal Theta. The implementation reuses
  Stochastica's Cholesky factor and its bounded jitter/diagonal-fallback policy.
- **DoubleWellProcess:** R. Benzi, *Stochastic Resonance: from climate to biology*,
  2007, [paper](https://arxiv.org/abs/nlin/0702008); L. Gammaitoni, P. Hänggi,
  P. Jung and F. Marchesoni, *Stochastic resonance*, 1998,
  [DOI](https://doi.org/10.1103/RevModPhys.70.223). Fixed Euler–Maruyama substeps
  model overdamped diffusion in a quartic potential. The example is qualitative;
  it makes no universal claim about an optimal noise strength.
- **TVonMisesRand:** D. J. Best and N. I. Fisher, *Efficient Simulation of the
  von Mises Distribution*, Applied Statistics 28(2), 152–157, 1979,
  [DOI](https://doi.org/10.2307/2346732),
  [author-uploaded paper](https://www.researchgate.net/publication/246035131_Efficient_Simulation_of_the_von_Mises_Distribution).
  Section 4's wrapped-Cauchy rejection construction was checked before coding.
  The implementation rationalizes rho to avoid small-concentration cancellation,
  bounds rejection to 64 attempts, and explicitly switches to a wrapped-normal
  asymptote above concentration 1e6.
- **KuramotoNetwork:** S. H. Strogatz, *From Kuramoto to Crawford: exploring the
  onset of synchronization in populations of coupled oscillators*, 2000,
  [DOI](https://doi.org/10.1016/S0167-2789(00)00094-4),
  [paper](https://web.mit.edu/~jadbabai/www/ESE680/Strogatz_Kuramoto.pdf).
  Equations 3.1–3.3 give the all-to-all model, complex order parameter and equivalent
  mean-field reduction. Additive independent phase diffusion and bounded
  Euler–Maruyama substeps are explicit extensions.

The SuperCollider 3.14.1 `WritingUGens.schelp`, `SC_Unit.h`, `SC_InterfaceTable.h`,
`SC_Rate.h`, and `MultiOutUGen` conventions determine construction, buffer locking,
output shape and calculation duration. The old Stochastica classes are unchanged.

## Round 3 sources and adaptations

Round 3 independently implements equations and reuses the existing bounded
Gamma, Gaussian, Poisson and stable samplers. No external simulator or paper code
is included. The categorical helper's default path retains old seeded arithmetic;
new models explicitly opt into the double-uniform path.

- **MaternNoise:** Särkkä and Solin, *Applied Stochastic Differential Equations*,
  section 12.3, equations 12.33–12.35 and example 12.7
  ([author text](https://users.aalto.fi/~asolin/sde-book/sde-book.pdf)).
  Exact half-integer state-space transitions, derivative-scaled coordinates and
  integrated innovation covariance; a bounded small-step series prevents
  cancellation before Cholesky. Variance is stationary output variance, with
  lambda=sqrt(2*nu)/correlationTime. Reset is deterministic, requiring warmup.
- **TDirichletRand:** the independent-Gamma construction documented in the
  [GSL reference](https://www.gnu.org/software/gsl/doc/latex/gsl-ref.pdf),
  section 20.28. The existing Gamma primitive supplies shape-boosted draws;
  log normalization preserves corner behavior at tiny concentrations. There is
  no GSL linkage or copied implementation.
- **PolyaUrn:** Pemantle, *A survey of random processes with reinforcement*,
  [arXiv](https://arxiv.org/abs/math/0610076). Standard additive reinforcement
  with a configurable amount; private rescaling also rescales future increments
  to preserve the model. Probability is measured before reinforcement, entropy
  after it. This is accumulated-history preference, not a Markov transition row.
- **MarkovModulatedTrig:** Fischer and Meier-Hellstern, *The Markov-modulated
  Poisson process (MMPP) cookbook*, 1993,
  [DOI](https://doi.org/10.1016/0166-5316(93)90035-S).
  A finite-calculation approximation draws independent event and transition
  hazards from the old state, then chooses a destination. It resolves at most
  one event and one opportunity per calculation, with explicit rate ceilings.
- **LevyFlight:** Nolan's *Stable Distributions: Models for Heavy Tailed Data*,
  [author chapter](https://edspace.american.edu/jpnolan/wp-content/uploads/sites/1720/2020/09/Chap1.pdf).
  Symmetric S0 increments scale as dt^(1/alpha); alpha=2 has variance
  2*scale^2*dt. Finite double state is retained without musical clipping, with
  documented recovery when a numerical path cannot be represented.
- **JumpDiffusion:** Siegrist's
  [compound-Poisson derivation](https://www.randomservices.org/random/poisson/Compound.html).
  Independent Gaussian diffusion plus uncompensated Gaussian-marked Poisson
  displacement; conditional aggregation replaces an unbounded per-jump loop.
  The count mean must be at most 1e8 per calculation; unsupported values recover
  explicitly rather than silently changing the rate.
- **WilsonCowan:** Wilson and Cowan, *Excitatory and inhibitory interactions in
  localized populations of model neurons*, 1972,
  [DOI](https://doi.org/10.1016/S0006-3495(72)86068-5).
  The API implements a normalized logistic variant without the original
  refractory factors. It uses deterministic fixed-substep RK4 and external,
  optional noise sources. Figure settings were checked for fixed-point versus
  sustained oscillatory behavior in this specific parameterization.
- **Lorenz96:** Lorenz, *Predictability: a problem partly solved*,
  [ECMWF paper](https://www.ecmwf.int/sites/default/files/elibrary/1995/10829-predictability-problem-partly-solved.pdf).
  The cyclic local stencil is evaluated at each RK4 stage with linear spatial
  cost. Default initialization perturbs the uniform equilibrium; reloading a
  user vector changes the reset template without jumping current state.

The plotting exporter calls the same numerical kernels as the plugin. Seeds,
burn-in, axes and scales are recorded in the figures. The guide distinguishes
the mathematical models from their finite precision, bounded sampler and
finite-calculation adaptations.
