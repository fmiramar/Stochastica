# GENDY source audit

Audited 2026-10-13. This is a new, independent mathematical implementation in the existing Stochastica repository. No upstream Gendy implementation is being replaced. Downloaded reference documents and upstream files stay in ignored build output; their text, code and figures are not redistributed.

## Verified claims and consequences

| Claim | Inspected source and location | Confidence / implementation consequence |
| --- | --- | --- |
| 1977 DSS evolves amplitude and inter-breakpoint duration independently, with reflection and linear connections across cycles | Luque 2006, printed pp. 22–24, method steps 1–9 | High for the described reconstruction; use duration in samples, not a linearly mapped frequency state |
| 1991 uses primary walks whose positions increment secondary walks | Luque 2006 pp. 25–28; Hoffmann 2000 pp. 32–34 | High; keep four state values per point, reflect both levels; credit Xenakis and Marie-Hélène Serra |
| Primary duration range can be a ratio of secondary duration width | Luque 2006 p. 27, method steps 4–5 | High; expose independent primary range and innovation size |
| PARAG time fields use exponential durations and Bernoulli sound/silence choices | Luque 2006 p. 29, referring to Serra 1993 pp. 252–253; Hoffmann 2000 p. 34 | High; GendyField is the timing subset, not the original fixed section/voice tables |
| S.709 uses time-dependent synthesis parameters | Hoffmann 2000 p. 31; Luque 2006 p. 31 names sine and sawtooth shapes | High for modulation, not its exact routing or coefficients |
| S.709 directly drives breakpoint walks while Gendy3 uses an intermediary walk | Hoffmann 2023, paragraphs beginning “In S.709” and “In other words” | High for this author's distinction; choose first order for GendyS709 and state the source |
| IDSS has two walk stages in each dimension; percussion contracts time-step mobility | Brown 2005 §§3, 4.1; Brown & Jenkins 2004 interface description identifies A/T Step1/2 and Max1/2 | High; GendyPerc uses two levels and contracts primary bounds, not merely an output envelope |
| IDSS offers linear, cosine, square interpolation | Brown 2005 §4.2 | High for names; exact square transfer is unspecified; do not label hold as an exact IDSS square implementation |
| Gendynish is an Arduino approximation, distinct from IDSS | Brown 2019, “Gendynish Algorithm” | High; cite as later context, not an algorithm source for historical classes |
| DDSS introduces physical diffusion and drift into breakpoint amplitude motion | Rojas & Cádiz abstract; CMJ 45(2), 48–66, DOI below | Abstract verified only. Full equations NOT verified. Class/API gated |
| Neighbor coupling is an experimental extension | This project's plan and discrete ring Laplacian | New Stochastica design, never attribute it to Xenakis |

## Short verified quotation ledger

Each quotation was checked in the indicated document. These are optional editorial anchors, not specifications.

- Xenakis 1992 p. 293, **as quoted in Luque 2006 p. 25**: “it bears the seed of future exploration.” The original book page was not inspected; retain the secondary attribution.
- Hoffmann 2023, waveform discussion: “sound characteristics are not governed by waveform shape per se”.
- Brown 2005 abstract: “a new class of percussive stochastic synthesis sound gestures”.
- Brown 2019, IDSS subsection: “rapid change from a bright complex timbre to a fixed and stable tone”.
- SC 3.14.1 Gendy2 help: “Random walk is of the amplitude and time delta”.
- sc3-plugins Gendy4 help: “a cubic-interpolated version”; Gendy5 help: “a non-interpolating version”.
- Rojas & Cádiz abstract: “includes a drift in the Brownian trajectories”. Do not infer its equation from this sentence.

## Read-only software audit

SC **3.14.1**, `server/plugins/GendynUGens.cpp`, `SCClassLibrary/Common/Audio/Gendyn.sc`, and Gendy1–3 help were inspected. Nick Collins's GPL-2.0-or-later implementation uses parent RGen, random initial points, normalized duration state mapped linearly to frequency, and numbered distributions 0–6. Gendy2's amplitude innovations additionally use a Lehmer-style generator. Gendy3 normalizes a cycle's durations to an explicit pitch. These are important differences from this round's local PCG, flat amplitude start, physical duration state and two-level reconstruction. None of the three classes or files is edited.

sc3-plugins `main`, `source/BhobUGens/BhobNoise.cpp` and Gendy4/5 help downloaded 2026-10-13, GPL-2.0-or-later, credit Bhob Rainey and Nick Collins. Gendy4 documentation says cubic, while the inspected code updates midpoint/slope/curve and clamps to at least two samples. Gendy5 holds values. GendyInterp's explicit zero-tangent Hermite mode does not claim equivalence to Gendy4. Upstream files are recorded by SHA-256 in the verification record, not copied into project source.

Existing Stochastica audit: SDCommon finite/clamp/RisingEdge; SDRandom PCG; SDEventProcesses strictly open double uniform; SDServer RT allocation, seed and rate-aware inputs; SDRange repeated folding; MapSonifiers hold/linear/smoothstep rendering. OUProcess, BoundedWalk, triggered stable sampling, MarkovChain and MultiGaussNoise keep their current code and seeded sequences. Round 2/3 tests are baseline regressions. Shared GENDY interpolation is implemented separately because the existing private sonifier function does not expose an endpoint API. No new RNG library or framework is needed.

## Unresolved history, explicitly excluded from fidelity claims

Original random generators, exact distribution parameterizations, integer quantization and initial values differ between versions. Hoffmann 2000 pp. 34–35 describes integer rendered amplitudes and historical bugs; these are not reproduced. Sources do not give sufficient S.709 modulation routing/depth settings or complete PARAG tables. Historical seed/audio equivalence is not promised. Serra 1993, Robindoré 1996 and Formalized Music were not inspected in their original editions; citations through inspected authors must say so. Brown's “square” interpolation remains unspecified.

## Sources

- [Luque 2006, author's thesis and download](https://sergioluque.com/stochastics), *Stochastic Synthesis: Origins and Extensions*, Sonology master's thesis.
- [Luque 2009](https://doi.org/10.1162/lmj.2009.19.77), *The Stochastic Synthesis of Iannis Xenakis*, LMJ 19, 77–84. Later stochastic concatenation is context, outside this implementation.
- [Hoffmann 2000, full article](https://rohandrape.net/ut/rttcc-text/Hoffmann2000a.pdf), *The New GENDYN Program*, CMJ 24(2), 31–38.
- [Hoffmann 2023, A Xenakis Dictionary](https://www.iannis-xenakis.org/en/stochastic-synthesis/).
- [Brown 2005, institutional full text](https://research-repository.griffith.edu.au/server/api/core/bitstreams/dc7c2e4f-bc64-58c7-8ee0-bd7d219f39a2/content), *Extending Dynamic Stochastic Synthesis*, ICMC, 111–114.
- [Brown & Jenkins 2004, author-uploaded text](https://www.researchgate.net/publication/27467610_The_Interactive_Dynamic_Stochastic_Synthesizer), ACMC, 18–22.
- [Brown 2019, institutional full text](https://research-repository.griffith.edu.au/server/api/core/bitstreams/84af0c9a-6a3c-42e5-b505-ccc0ff198af1/content), *Gendynish*.
- [Rojas & Cádiz, CMJ article](https://doi.org/10.1162/comj_a_00606), issue year 2021; the authors' university also lists 2022 publication metadata.
- [SuperCollider source, 3.14.1](https://github.com/supercollider/supercollider/blob/Version-3.14.1/server/plugins/GendynUGens.cpp); [sc3-plugins BhobNoise](https://github.com/supercollider/sc3-plugins/blob/main/source/BhobUGens/BhobNoise.cpp).
