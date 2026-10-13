# GENDY implementation decisions

Status: in progress on a local branch; unpublished. The baseline is Round 3 commit `8db8cc0`; its eight CTest suites pass with strict Release builds of both server plugins.

The research dossier is [here](research/GENDY_HISTORICAL_SOURCES.md). DDSS remains [gated](research/GendyDiffusion-source-notes.md). Seven independently specified models can proceed without it.

## Common waveform contract

Audio-only waveform UGens, 2–64 construction-time points. One RT allocation contains amplitude, duration, primary walks and proposal workspace. Cycle updates visit indices 0..N-1, amplitude draw then duration draw. All point proposals are computed before any ring coupling. Parameters affecting walks are sampled at cycle entry; interpolation choice at segment entry. Reset is a sample-accurate rising edge, rewinds the construction seed and all state, and emits the first sample of the fresh waveform on the reset sample. Seed zero consults the parent generator only at construction.

Durations are sample counts: lower = fs/(N*maxFreq), upper = fs/(N*minFreq). Frequencies are sorted, clipped to 0.001..fs/N, yielding segments of at least one sample. Fractional segment lengths retain their remainder; at most one transition per sample. No fixed-cycle normalization. Initial amplitudes/primary states are zero; durations are midpoint. All-zero motion therefore starts silent. Current segment endpoints are carried across cycle replacement; the new first point never replaces the endpoint already being rendered.

Repeated reflection uses periodic folding, with reduced operands to avoid overflow on large overshoots. Closed barriers freeze. Inputs are finite-conditioned and bounded before integer conversion. These safety policies are modern adaptations.

Distribution IDs 0..5: uniform, truncated symmetric Cauchy, truncated symmetric logistic, truncated symmetric hyperbolic secant (historical hyperbolic-cosine density), arcsine, mapped truncated exponential. Shapes 0.0001..1. The new transforms are specified by quantiles in code/tests and do not reproduce historical RNGs or every SC transform. No ambiguous SINUS distribution selector. Steps are fractions of the corresponding full secondary interval, 0..1. Zero steps freeze the relevant first-order walk. Each point still consumes two innovations when frozen, preserving random stream alignment.

For 1991: primary half-width = rangeRatio * secondary width; primary innovation = primaryStep * full primary width * normalized draw; reflected primary position increments the secondary state. `ampPrimaryRange` and `durPrimaryRatio` are range ratios. This follows Luque's ratio interpretation with explicit scaling.

S709 uses first-order walks and one sine/saw phase, initially zero, advancing each audio sample. Mobility and both frequency bounds multiply by 2^(depth*modulator). Depths 0..4, frequency 0..fs/2; safety caps apply afterward. These particular routes/scales are new choices. Zero depth exactly matches Gendy1977 with shape 1.

Perc uses the 1991 engine with fixed primary innovation fractions 0.1 and time-varying primary range ratios. Ratios progress geometrically between positive endpoints; if either endpoint is zero use linear interpolation. `decay` is the finite time to reach the final mobility, not an amplitude decay constant. Rising trigger restarts time; `resetState=1` also rewinds waveform/RNG. Construction starts a gesture. No automatic volume envelope.

Interpolation modes: hold (left endpoint), linear, cosine ease, zero-tangent cubic Hermite (smoothstep). Smooth modes reach both endpoints and do not overshoot; hold deliberately jumps. This Hermite specialization is documented rather than presented as SC Gendy4's algorithm.

Coupling: form unconstrained first-order proposals, synchronously add kappa*(left-2*center+right), then reflect; kappa in 0..0.5, ring topology. Zero coupling is sample-identical to Gendy1977. O(N) per cycle.

Field: control-rate only, 1..32 gates; independently seeded voice streams, exponential field duration, Bernoulli activity at each new field. At most eight boundaries per tick; carry time debt. Mean is floored to one control interval. Consecutive fields can have equal gates, so audible gate dwell is not field duration. Gates need external fades for audio.
