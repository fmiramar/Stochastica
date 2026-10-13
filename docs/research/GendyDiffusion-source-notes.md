# GendyDiffusion research gate — not yet satisfied

Required source: Emilio L. Rojas and Rodrigo F. Cádiz, *A Physically Inspired Implementation of Xenakis's Stochastic Synthesis: Diffusion Dynamic Stochastic Synthesis*, Computer Music Journal 45(2) (2021), 48–66. [DOI](https://doi.org/10.1162/comj_a_00606).

The complete article has **not** been obtained. The institutional abstract verifies particle-to-amplitude mapping and drift, but does not define the state equation, drift field, boundary treatment, time increment or physical parameters. Those items and their page references remain unresolved. There is no frozen API or placeholder implementation.

Access audit, 2026-10-13: publisher DOI and Crossref-linked PDF returned HTTP 403; Project MUSE article 856999 and PDF returned HTTP 503; OpenAlex lists no open-access full text. The coauthor's public bibliography additionally links the ICMC 2021 paper *DDSS: a diffusion-based extension of Xenakis’ dynamic stochastic synthesis*, Fulcrum file rn3013782, which also returned 403. No author was contacted and no purchase or account access was attempted.

To open this gate: read the complete CMJ article, record each equation and page, identify waveform versus additive variants, extract parameter units and boundary policy, then implement independent reference tests before adding the public class. Generic Brownian drift, OU motion, or ring smoothing must not be shipped under the DDSS name.
