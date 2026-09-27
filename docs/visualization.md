# Visualization strategy

StochasticDynamicsUGens should not give every class the same oscilloscope or attractor plot. The useful image is the one that exposes the state, dependence, or probability structure that makes a class distinct.

## Recommended visual forms

| Class family | Primary static figure | Useful real-time view | Why |
| --- | --- | --- | --- |
| Two-state maps (`HenonMap`, `GbmanMap`, `LatoocarfianMap`, `IkedaMap`, `LoziMap`, `TinkerbellMap`, `CliffordMap`, `DeJongMap`, `CoupledLogisticMap`, `RulkovMap`) | 2D phase portrait, after discarding a documented transient | A fading `x-y` trail with the current point highlighted | The recurrence is a trajectory in a two-dimensional state space. A time plot hides the attractor geometry. |
| Scalar maps (`CircleMap`) | Cobweb diagram plus a short iteration sequence | Circle/phase position or cobweb stepping | A one-dimensional return map is better described by `x[n+1]` against `x[n]` than by a fabricated 2D attractor. |
| Lattices (`CoupledMapLattice`) | Cell-by-iteration space-time heatmap | Scrolling heatmap | Spatial synchronization, waves, and defects are the relevant structures. |
| Three-state flow (`Chua`) | 3D trajectory plus `xy`, `xz`, and `yz` projections | Bounded 3D/phase-plane trail sampled at display rate | Chua's orbit is intrinsically three-dimensional; projections disclose overlaps hidden by any one camera angle. |
| Delayed scalar flow (`MackeyGlass`) | Delay embedding such as `(x(t), x(t-tau), x(t-2*tau))`, accompanied by a time series | Scrolling time series and delayed phase portrait | Its effective state is the delay history, not one scalar output sample. The embedding lag must be documented. |
| Spiking model (`Izhikevich`) | Voltage/recovery phase plane and aligned voltage/spike time plots | Scrolling `v-u` trace with spike flashes | Resets make both the orbit and event timing important. |
| Continuous stochastic processes (`OUProcess`, `FractionalNoise`, `FBrownianMotion`, `BoundedWalk`) | Time series plus an appropriate summary: stationary density, autocorrelation, or log-log spectrum | Scrolling trace; optional histogram or spectrum updated slowly | A phase portrait of successive noisy samples often suggests deterministic geometry that is not present. |
| Event processes (`RenewalTrig`, `HawkesTrig`) | Event raster and inter-event histogram; add intensity over time for Hawkes | Scrolling event raster and intensity curve | Timing and clustering, rather than sample amplitude, define the process. |
| Triggered distributions | Histogram/PDF comparison; ECDF or quantile plot for heavy tails | Accumulating histogram only when sampling is central to the example | A live curve adds little unless it shows convergence. Heavy tails need robust axes or log scales. |
| Markov processes (`MarkovChain`, `SemiMarkov`) | Directed transition graph plus state timeline/dwell histogram | State strip with highlighted graph node | State topology and dwell behavior are discrete, not geometric phase space. |
| Multivariate Gaussian (`MultiGaussNoise`) | 2D covariance ellipse or pair plot; covariance heatmap for more dimensions | 2D scatter trail or slowly updated covariance ellipse | The shape should reveal correlation and scale, with projections for dimensions above two. |
| Explicit map sonifiers | Raw-map phase portrait beside an output/time or spectrum view, only when both are generated from a shared observable state | Add a live orbit only if it is the exact internal orbit | The current sonifier UGens expose audio but not their internal coordinates. A separately clocked map would look plausible while showing a different trajectory, so it should not be presented as the sonifier's state. |

## Terrain views

A terrain surface is appropriate only when the synthesis actually reads a scalar field `z = f(x, y)` (or a three-dimensional volume) at a visible position. Most map and flow classes here produce a trajectory, not a terrain. Drawing a decorative surface behind a Hénon or Chua line would imply a waveterrain lookup that the UGen does not perform.

If a future sonifier explicitly samples a 2D wavetable, its help should show the actual table as a height or color field and overlay the exact read path. The table buffer, coordinate mapping, wrap/fold rule, and interpolation should be shared by the audio and visualization paths so the display cannot silently diverge from the sound.

## Animation policy

Animation belongs in a dedicated subsection for classes whose evolving state is the main concept. It should not be attached automatically to every audible example: doing so makes help pages long, creates unnecessary windows and OSC responders, and can turn statistically meaningful plots into decoration.

Use these constraints for live views:

- Send display data at 20–30 Hz, independently of the audio/control update rate.
- Draw and retain history in the language process, never in the server real-time thread.
- Bound every trail, histogram, and raster buffer.
- Make the sound and image consume the same state. Do not run an unsynchronized visual copy.
- Show axis names, mappings, projection/embedding choices, transients, and clipping.
- Close the Synth, responder, and animation on window close and Cmd-period.
- Keep a non-GUI audible example first so headless and remote-server users are not excluded.

The `Chua` example follows these rules. It uses the exact three outputs being sonified, transmits them at 30 Hz, and retains 1,200 points. The static documentation uses a light-theme oblique figure with three colored coordinate arrows and one wide figure containing the three orthogonal projections. Titles and axis explanations come from native SCDoc captions rather than text embedded in the images.

`SDPhaseView` applies the same exact-state rule to the raw two-state maps and Izhikevich. The deterministic map pages now include point-based phase portraits, while CircleMap uses a cobweb, CoupledMapLattice uses a space-time field, Mackey–Glass uses a time/delay view, and Izhikevich uses voltage-time and v-u views.

The stochastic pages use process-specific static forms: trace/density, event/interval, event/intensity, spectral reference, finite-memory path comparison, boundary-rule comparison, transition and dwell views, covariance geometry, and distribution density or probability-mass plots. These are explanatory realizations from the documented equations and parameter sets, not screenshots or substitutes for the statistical verification suite.

## Regenerating the figures

The checked-in PNGs are generated from the same equations and default coefficients as the UGen:

```bash
python3 scripts/plot_chua_state_space.py HelpSource/Classes
python3 scripts/plot_deterministic_dynamics.py HelpSource/Classes
python3 scripts/plot_stochastic_dynamics.py HelpSource/Classes
```

The scripts have no third-party Python dependency. They rasterize text-free PNGs directly with standard-library code and supersampling, so documentation builds do not depend on matplotlib, Pillow, or a platform GUI renderer.
