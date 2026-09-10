Parallel Simulation Guide
=========================

Long simulations (e.g. a 100-year return period) can be split into
independent day-chunks that run in parallel across CPU cores. PyBTLS
does the splitting, seeding and result merging for you: pass
``no_chunk`` to :meth:`Simulation.add_sim` and read the merged result
as if it were a single run. On a 30-core workstation a simulation that
takes ~10 hours single-threaded completes in ~20 minutes.

Quick start
-----------

.. code-block:: python

   from pybtls import Simulation

   sim = Simulation(output_dir="./out_100yr")
   sim.add_sim(
       bridge=bridge,
       traffic=traffic_gen,          # a TrafficGenerator
       no_day=25000,                 # e.g. 100 years x 250 days
       output_config=output_config,
       time_step=0.1,
       seed=42,                      # master seed (optional)
       no_chunk=25,                  # split into 25 chunks of 1000 days
       tag="100yr",
   )
   sim.run(no_core=25)

   out = sim.get_output()["100yr"]   # one merged result
   out.read_data("BM_summary")       # same shape as a sequential run

Everything else - per-chunk seeding, output directories, and the merge
of every output type - is automatic. The merged object mirrors the
usual output-manager interface (``get_summary``, ``read_data``,
``relocate``); per-chunk results remain on disk under
``<tag>/chunk_000``, ``<tag>/chunk_001``, ... and can be inspected with
``out.read_chunk_data(key)`` or ``out.chunks``.

Chunked simulations coexist freely with ordinary ones: you can add
several chunked and unchunked simulations to the same ``Simulation``
and they share one process pool.

The ``__main__`` guard
----------------------

Workers are started with the ``spawn`` start method, which re-imports
the main module in every child process. The set-up and the ``run()``
call must therefore live inside a function that is called from a guard:

.. code-block:: python

   def main():
       sim = Simulation(output_dir="./out_100yr")
       ...
       sim.run(no_core=25)


   if __name__ == "__main__":
       main()

Without the guard each child re-executes the module body, tries to
start a pool of its own and dies during bootstrap; the pool replaces
every dead worker, so the script neither raises nor terminates - it
just keeps spawning processes.

Why day-chunking is statistically valid
---------------------------------------

The traffic flow model is periodic over one day (hourly blocks,
``FlowGenerator``); there is no weekly or seasonal pattern. Each chunk
runs with an independent, deterministic RNG stream derived by mixing
``(master_seed, chunk_index)``, so the chunks behave like different
stretches of one long traffic history. The merged outputs are therefore
*statistically equivalent* to a sequential run - they are not the same
random realisation a particular sequential seed would have produced.

The only physical seam is the empty bridge at each chunk start: a
crossing event that would have straddled the boundary is split. At one
boundary per several hundred simulated days this bias is negligible
(well below the Monte Carlo noise).

What "merged exactly" means per output
--------------------------------------

PyBTLS validates this with a *split-replay* test: a recorded traffic
stream is replayed once continuously and once as two chunks; the merged
chunk outputs must equal the continuous outputs. The merge rules are:

* **Block maxima, POT, events, time history, fatigue events** -
  concatenated with time/index continuation. Exact.
* **Interval statistics (SS_S)** - intervals are self-contained;
  concatenated with index continuation. Exact.
* **Cumulative statistics (SS_C)** - each chunk's statistics are
  inverted back to raw moment sums and combined with the parallel
  (Chan) formulas - the exact counterpart of the C++ accumulator. The
  only error is the 0.01 text quantisation of the inputs.
* **Fatigue rainflow** - chunk runs keep their unclosed residual
  reversals (``FRR_*`` sidecar files) instead of closing them; the
  merge concatenates the residual sequences in order and closes them
  with the same C++ algorithm (residue splicing). Exact.
* **Flow statistics / vehicle files** - concatenated with hour /
  calendar continuation. Exact.

Chunk-size validation
---------------------

For the merged indices to align, the chunk length must be a whole
number of days that is also a multiple of the configured block-maximum
block size, the POT counter block size, and the statistics interval.
:meth:`Simulation.add_sim` validates this and raises ``ValueError``
with a specific message if a setting is incompatible - adjust
``no_chunk`` (or the block sizes) accordingly. With the default
day-based blocks and the default 3600 s interval, any whole-day chunk
length is valid.

Chunking requires a :class:`TrafficGenerator` (recorded traffic cannot
be re-seeded) and ``no_day`` divisible by ``no_chunk``.

Reproducibility
---------------

+--------------------------------------------+----------------------+
| Scenario                                   | Reproducible?        |
+============================================+======================+
| ``seed=None`` (default)                    | No - a random master |
|                                            | seed is drawn; read  |
|                                            | it back from         |
|                                            | ``out.master_seed``  |
+--------------------------------------------+----------------------+
| ``seed=42, no_chunk=N``                    | Yes - chunk i is     |
|                                            | seeded from (42, i)  |
+--------------------------------------------+----------------------+
| Same seed, different ``no_chunk``          | No - different chunk |
|                                            | boundaries and seeds |
+--------------------------------------------+----------------------+

A chunked run is a different statistical realisation, not a
reproduction of the serial run with the same seed: chunk *i* is seeded
from ``(master_seed, i)`` and starts with an empty bridge. The mixing
means two runs whose master seeds are close (a ``seed=100+k`` replicate
study, say) do not silently share chunk streams. Reproducible here
means that repeating the *same* chunked configuration gives the same
numbers, not that the numbers match ``no_chunk=None``.

Performance expectations
------------------------

Day-chunking is embarrassingly parallel; the only overheads are the
process spawn (~0.5 s per worker, one-off) and the merge (sub-second).
Measured on a 32-core workstation (2 lanes, 500 trucks/h/lane, 0.1 s
step, BM+POT+Stats+rainflow outputs):

==========  ===========  ================  ========
Days        single core  8 chunks/8 cores  speedup
==========  ===========  ================  ========
16          2.5 s        0.8 s             3.1x
96          15.0 s       2.1 s             7.1x
==========  ===========  ================  ========

The shortfall from 8x is the fixed spawn cost; for real workloads
(minutes to hours per chunk) the speedup approaches the core count.

Choose ``no_chunk`` roughly equal to the cores you will give
``run(no_core=...)``; more chunks than cores also works (they queue).
