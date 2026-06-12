# Dev Log — Auto-Chunk Parallel Simulation + Exact Merge + C++ Micro-Optimisation

Branch: `feat/auto-chunk-parallel`
Started: 2026-06-12

## Goal

A single 100-year saturated-flow simulation currently takes 8–12 h single-core
(0.1 s time step). Target: ~25 min via automatic day-chunking across ~30 cores
(statistically equivalent, "meaning A"), then a further ×2–3 from bit-exact C++
micro-optimisation. GPU was evaluated and rejected (marginal gain over 32 cores
does not justify a CUDA rewrite).

## Confirmed decisions (with user)

- Precision contract: **statistical equivalence** (meaning A), not bit-identity
  with a previous serial run. C++ micro-opts however must be bit-exact.
- All existing output types must survive; future outputs must be easy to add
  → merge layer is **metadata-driven**, not per-output bespoke code.
- Outputs the user actively relies on: Block-Max, POT, Stats moments, Fatigue
  Rainflow → all four need **exact** merge (Stats & Rainflow require small C++
  additions to expose boundary state).
- API shape: **auto-hidden chunking** — one `add_sim(...)`, internal expansion
  to N seeded chunks, auto-reduce, returns a single `_OutputManager`-shaped
  result. Existing multi-sim concurrency behaviour unchanged.
- C++ micro-optimisation **in scope** (devirtualise IL dispatch, SIMD lookup,
  build flags). `-march=native` must NOT go into release wheels.

## Constraints

- Machine: 32 cores, ~20 GB usable RAM (local LLM running; GPU1 occupied).
  **Keep test sims small** (days, not years); cap parallel test workers.
- Conda env: `pybtls-dev` (python 3.11). No new tools without permission.
- Respond in Chinese, think in English. Surgical changes only.

## Architecture

Merge layer = 4 primitive categories, declared per-output in a registry next to
`py/pybtls/output/read/`:

1. **concat** — row concatenation + time-offset rebase + index renumbering
   (TH, AllEvents, FatigueEvents, BM_V/S/Mixed, POT_V/S, traffic file)
2. **bin-sum** — histogram/counter addition (POT counter, traffic stats,
   closed rainflow histogram)
3. **moment-merge** — parallel Welford/Chan combination of (N, mean, M2, M3, M4)
   (SS_C cumulative stats; SS_S interval stats if boundaries align)
4. **boundary-state concat** — rainflow residual reversals concatenated in
   order, then re-run rainflow closure (exact)

C++ additions (Phase 2 only): expose raw moment sums from `CEventStatistics`,
residual reversals from `CRainflow` (cpp/include/Rainflow.h:93 m_vReversals).

Validation strategy ("reference accumulator"): chunks additionally dump raw
event streams; feed both chunks' streams sequentially through the *real* C++
accumulators and compare with `merge(a, b)` — exact check that sidesteps the
"different random realisation" problem.

## Phases

- **Phase 0** — foundations: branch, dev log, merge module skeleton, merge
  primitive interfaces + output metadata registry, reference-accumulator test
  harness. → `01_phase0.md`
- **Phase 1** — easy outputs (no C++): concat + bin-sum primitives wired to
  BM / POT / counters / traffic stats; auto-chunk API prototype in
  `Simulation`. → `02_phase1.md`
- **Phase 2** — exact Stats + Rainflow: C++ boundary-state exposure via
  pybind11; Chan moment merge; residual-reversal splice. → `03_phase2.md`
- **Phase 3** — bit-exact C++ micro-opts: profile first; devirtualise
  InfluenceLine dispatch; SIMD axle×IL lookup; portable flags only in wheels.
  → `04_phase3.md`
- **Phase 4** — cross-cutting: comprehensive tests (coverage of failure
  modes, not just happy path), docs rewrite (beginner→advanced guide,
  docstring audit for autodoc API), build/CI checks. → `05_phase4.md`

## Code-quality review track (user request #3)

Beyond performance: collect design concerns in `design_review.md` as they are
encountered (e.g. pickle-based storage choice, Python wrapper duplicating C++
state, `parallel.rst` describing unimplemented features). **Discuss with user
before changing any of these** — they are out of scope until approved.

## Quality bars (user request #4)

- tests/: aim at *failure-mode coverage* (merge correctness vs reference
  accumulator, seed/reproducibility, boundary conditions, format round-trips),
  not just smoke tests.
- docs/: progressive user guide (simple → complex), accurate parallel guide,
  complete & correct docstrings so the API reference autogenerates cleanly.
