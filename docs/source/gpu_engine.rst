GPU Load-Effect Engine (experimental)
=====================================

PyBTLS can run the load-effect calculation on an NVIDIA GPU instead of the
default C++ engine, by passing ``engine="cuda"`` to
:meth:`Simulation.add_sim`. It is an **opt-in, experimental** backend aimed at
the *compute-dominated* regime; for ordinary runs the default ``engine="cpu"``
is the right choice.

When is the GPU engine worth it?
--------------------------------

Profiling shows the per-timestep load-effect summation is usually only
~15-20 % of wall-clock — the bottleneck is the output writers (time history,
POT, fatigue) and per-event overhead, none of which the GPU accelerates. The
GPU engine pays off only when the *computation* dominates:

* a **long-span or congested bridge** (many axles on the deck at once),
* **many load effects** (tens to hundreds),
* a **fine** ``time_step``, and
* you need **block maxima, peaks-over-threshold, fatigue rainflow and/or flow
  statistics** (all reduced from the same fused E(t) pass). Time history is also
  produced, but — being one row per time step — it is I/O-bound, so the GPU
  speed-up is smaller for time-history-dominated runs.

Measured on an RTX 3090 against **one core** of a Ryzen 9 7950X (float64,
``time_step=0.1``): typical free-flow configurations run **2.5-5x** faster
(6 effects with the full BM/POT/SS/FR output set ~3x, 24 effects ~5x, an
influence surface ~4x), and a compute-dominated case — a 100 m bridge under
bumper-to-bumper congested traffic — reaches **~10x**. The gap grows with the
number of load effects (CPU cost is linear in effects, GPU sublinear) and with
the number of axles simultaneously on the deck. The device working set is
tiled adaptively to the *currently free* VRAM, so congested / long-span
traffic and small or shared GPUs shrink the tile (a few % slower) instead of
running out of memory.

Weigh this against CPU chunk-parallelism before reaching for the GPU: with
``add_sim(..., no_chunk=N)`` the CPU engine scales near-linearly across cores
for *generated* traffic, and on a 16-core machine that matches or beats the
GPU for free-flow runs. The GPU engine's clear wins are therefore:

* **recorded traffic** (``TrafficLoader``) — chunking requires a
  ``TrafficGenerator``, so a long recorded stream otherwise runs on one core;
* runs that must be **exactly sequential** (chunks are only statistically
  equivalent, each with its own RNG stream);
* **many load effects** on a congested / long-span bridge, where the GPU
  overtakes even a fully-chunked CPU.

For short-span bridges with a handful of effects, or any run needing the full
output set, use ``engine="cpu"``.

Running in parallel
-------------------

Run GPU simulations with ``run(no_core=1)``. The CPU engine scales near-linearly
when you split a run with ``add_sim(..., no_chunk=N)`` and ``run(no_core=N)``
(each day-chunk is an independent process — measured ~3x on 4 cores, ~6x on 8,
for a few hundred MB), but the GPU engine does **not**: concurrent tasks share
the one device, so the device compute serialises (no speed-up) while each worker
replicates its window in host RAM and VRAM — multiplying memory for nothing, and
risking OOM if the default ``no_core`` (cpu_count − 2) is applied. The engine
warns when GPU tasks are queued with ``no_core > 1``. Let the GPU parallelise
internally on one process instead.

Other backends (untested)
--------------------------

The engine name passed as ``engine=`` is the torch device: besides ``"cuda"``
(NVIDIA, and AMD via ROCm; float64), ``engine="mps"`` (Apple Silicon, via
Metal; runs in float32 since MPS has no float64) and ``engine="xpu"`` (Intel
GPU; float64) are wired up but **untested** — the engine probes each backend
for the ops it needs and raises a clear error if one is missing. The MPS
backend may lack ``searchsorted``/``scatter_reduce``; set
``PYTORCH_ENABLE_MPS_FALLBACK=1`` to run those ops on the CPU instead (slower).

Installation
------------

The GPU engine needs a CUDA build of PyTorch (the fused kernel uses Triton,
which ships with PyTorch). Install the optional dependency::

   pip install pybtls[gpu]

or install the ``torch`` wheel matching your CUDA toolkit yourself.

Quick start
-----------

.. code-block:: python

   from pybtls import Simulation, Bridge, InfluenceLine, TrafficLoader, OutputConfig

   il = InfluenceLine(IL_type="built-in")
   il.set_IL(id=1, length=40.0)
   bridge = Bridge(length=40.0, no_lane=4)
   bridge.add_load_effect(inf_line_surf=il, threshold=0.0)

   traffic = TrafficLoader(no_lane=4)
   traffic.add_traffic(traffic="recorded.txt", traffic_format=4)

   sim = Simulation(output_dir="./out_gpu")
   sim.add_sim(bridge=bridge, traffic=traffic, engine="cuda")   # <-- the switch
   sim.run(no_core=1)

   bm = sim.get_output()["Sim_1"].read_data("BM_summary")       # per-effect block maxima

The result is read back through the same ``read_data("BM_summary")`` path as the
CPU engine, so switching engines does not change how you consume the output.

Scope and limitations
---------------------

The GPU engine is experimental and intentionally narrower than the CPU engine:

* **Traffic:** recorded (``TrafficLoader``) or generated (``TrafficGenerator``,
  deterministic under ``seed``). Generated traffic is streamed in RAM-bounded day
  windows, so peak host memory is constant regardless of the simulated length
  (a 1000-year run uses the same memory as a 1-year run, only more wall-clock).
* **Influence lines:** discrete, built-in (ids 1-9) and influence surfaces,
  including hogging / negative ILs and a distinct influence line / weight per
  lane (matching the C++ engine's per-lane summation).
* **Load-effect mode:** vertical, centrifugal (``W·v²/g``) and braking
  (``W·|a|/g``, or ``W·braking_factor`` when the acceleration is zero) — the same
  per-axle force coefficient the C++ engine applies.
* **Output:** block maxima (``BM_summary``), peaks-over-threshold
  (``PT_S`` / ``PT_C`` / ``PT_V``), fatigue rainflow (``FR_*``, via
  ``set_fatigue_output``; each block's E(t) is reduced to turning points on the
  device and fed to the same ASTM E1049-85 counter the CPU uses, with the
  residual carried across blocks), load-effect statistics (``SS_C`` / ``SS_S``),
  vehicle flow statistics (``FlowData_*``, via ``set_stats_output``
  ``write_flow_stats``) and time history (``TH``), selected through the same
  :class:`OutputConfig` flags as the CPU engine.
* **Not produced** (configure these and they are skipped with a warning — use
  ``engine="cpu"``): every-event output (``write_each_event``), the vehicle file
  (``set_vehicle_file_output``) and per-block-max / fatigue-event vehicle detail
  (``set_BM_output`` ``write_vehicle`` / ``write_mixed``, ``write_fatigue_event``).
  These are per-event / per-vehicle detail outputs — the I/O-bound ones the GPU
  does not accelerate anyway. Rainflow residual sidecars (``write_residuals``,
  the exact chunk-splice mechanism) ARE produced, so chunked GPU runs merge
  their fatigue histograms exactly.

The POT and statistics paths rebuild the C++ engine's *event* partition — an
event is a window of constant on-bridge vehicle composition — from each
vehicle's on-bridge interval, so the events, vehicle counts and truck counts
track ``engine="cpu"`` to within ~1 % (the uniform sampling grid merges
composition changes that fall inside one ``time_step``, which the CPU resolves
exactly); the peak *values/times* carry the same grid-sampling noise as the
block maxima, and borderline events near a POT threshold may flicker in or out.
Flow statistics (``SS_C`` / ``SS_S``) are the distribution of each event's
governing value, so they inherit the same tolerance. The very end of a run
differs slightly by design: the CPU loop never simulates the final vehicle of a
recorded stream and lets events that started before the end time run past it,
while the GPU computes every vehicle's full crossing and keeps events starting
up to (and including) the end time. To enable POT, configure it
before adding the simulation::

   cfg = OutputConfig()
   cfg.set_POT_output(write_summary=True, write_counter=True, write_vehicle=True)
   sim.add_sim(bridge=bridge, traffic=traffic, output_config=cfg, engine="cuda")

Numerical agreement
-------------------

The GPU engine is **not bit-identical** to the CPU engine, by design. The two
sample the continuous load-effect history on different time grids (the CPU
engine steps from each event boundary; the GPU uses a uniform grid), so the
captured extreme differs slightly — under ~1 % for smooth influence lines and a
few percent for discontinuous shear influence lines. The same applies to POT
peak values; the sorted peak-value distribution (what extreme-value analysis
uses) agrees to well under 1 % in the bulk. The GPU engine targets this
*statistical-tolerance* regime in exchange for parallel efficiency; see the
project notes for the underlying sampling design.
