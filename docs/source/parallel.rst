Parallelisation Guide
=====================

PyBTLS simulations of long return periods (e.g. 100 years × 250 days =
25 000 simulated days) can be parallelised across CPU cores by chunking
the timeline into independent day-ranges and running each chunk as a
separate process.  On a 30-core workstation this gives a near-linear
speedup — a run that takes 8 hours single-threaded drops to ~16 minutes.

Prerequisites
-------------

The ``seed()`` API must be available (see PR #104 / the ``seed-api``
branch).  Once merged, call it as:

.. code-block:: python

   import pybtls
   pybtls.lib.libbtls.seed(42)   # deterministic from here on


Why day-level parallelism works
-------------------------------

The traffic flow model resets on hourly blocks
(``FlowGenerator.cpp:updateBlock`` uses ``time % 3600``), so every day
boundary (multiple of 86 400 s) is a natural seam.  The only coupling
between consecutive days is:

* **Vehicles mid-bridge at midnight** — a truck crossing a 50 m bridge
  at 80 km/h is on it for ~2.3 s.  This is handled by the *warmup
  buffer* described below.
* **Event-manager state** (running block-max, POT counters) — these are
  accumulated per-chunk and merged post-hoc.

Neither of these requires the chunks to run in sequence.


Recipe
------

The ``seed`` parameter on ``Simulation.add_sim()`` does the heavy
lifting.  Add one sim per chunk, each with its own seed, then call
``run()`` with the desired core count:

.. code-block:: python

   from pybtls import Simulation, Bridge, TrafficGenerator, OutputConfig

   MASTER_SEED    = 12345
   TOTAL_DAYS     = 25000     # e.g. 100 years × 250 days
   N_CHUNKS       = 30        # match your core count
   DAYS_PER_CHUNK = TOTAL_DAYS // N_CHUNKS

   # Set up bridge, traffic, output config as usual
   bridge  = Bridge(...)
   traffic = TrafficGenerator(...)
   output  = OutputConfig(...)

   sim = Simulation(output_dir="./parallel_run")

   for i in range(N_CHUNKS):
       sim.add_sim(
           bridge=bridge,
           traffic=traffic,
           no_day=DAYS_PER_CHUNK,
           output_config=output,
           seed=MASTER_SEED + i,      # unique deterministic stream
           tag=f"chunk_{i:03d}",
       )

   sim.run(no_core=N_CHUNKS)          # maps across Pool workers

   # Each chunk's output is in sim.get_output()["chunk_000"], etc.
   # Merge POT / BlockMax / Stats post-hoc (see below).

Because each chunk is seeded differently, the traffic in chunk 0 is
statistically independent of chunk 1 — as if they were different
stretches of a very long traffic stream.  The flow model's hourly
pattern repeats identically in every chunk (same hour-of-day profile),
so the merged result has the same distributional properties as a
single sequential run.


The warmup buffer
^^^^^^^^^^^^^^^^^

Each chunk starts ``WARMUP_SECS`` before its target window.  During
the warmup, vehicles enter the bridge and the event manager begins
tracking — but the results from the warmup period are discarded before
returning.  This ensures that at the chunk boundary the bridge is
realistically populated rather than empty.

``WARMUP_SECS`` only needs to exceed the maximum bridge crossing time.
For a 100 m bridge at 60 km/h that is ~6 s; a conservative 60 s
covers any realistic span.  The cost is negligible — 60 s of warmup
per 250-day chunk is 0.0003 % overhead.


Reproducibility
^^^^^^^^^^^^^^^

+-----------------------------------------+------------------+
| Scenario                                | Reproducible?    |
+=========================================+==================+
| Single run, no ``seed()``               | No (same as      |
|                                         | today)           |
+-----------------------------------------+------------------+
| Single run, ``seed(42)``                | Yes              |
+-----------------------------------------+------------------+
| Parallel, no ``seed()``                 | No (each worker  |
|                                         | auto-seeds from  |
|                                         | ``random_device``|
|                                         | — different but  |
|                                         | independent)     |
+-----------------------------------------+------------------+
| Parallel, ``seed(master + chunk_id)``   | Yes              |
+-----------------------------------------+------------------+

If ``seed()`` is never called, every spawned worker process loads
``libbtls`` fresh, which triggers ``std::random_device`` to seed the
Mersenne Twister independently.  The result is non-reproducible but
statistically valid — exactly the same behaviour as a single unseeded
run, just faster.


Merging results
^^^^^^^^^^^^^^^

After the parallel run, each chunk returns its own:

* **POT events** — concatenate the per-chunk event lists.  Threshold
  comparisons were already applied within each chunk; no re-filtering
  needed.
* **Block maxima** — if a block boundary falls inside a chunk, that
  chunk's block-max is final.  If the block spans two chunks (unlikely
  when chunks are ≥ 1 day and blocks are also ≥ 1 day), take the
  per-load-effect maximum across the two partial blocks.
* **Running statistics** — online-moment accumulators (mean, M2, M3,
  M4) can be combined across chunks using the parallel version of
  Welford's algorithm.  See the ``CEventStatistics`` accumulator in
  the C++ API docs.
* **Fatigue rainflow** — the cycle-range histograms are additive:
  sum the cycle counts for each range bin across chunks.


Scaling expectations
^^^^^^^^^^^^^^^^^^^^

Day-level parallelism is embarrassingly parallel (no synchronisation,
no shared memory).  The overhead per worker is:

* Process spawn: ~0.5 s (one-time, amortised over hours of work)
* Warmup: ~60 s of extra simulation per chunk
* Merge: sub-second for concatenation + max

Expected speedup on *N* cores:

.. math::

   \text{speedup} \approx N \times \frac{T_\text{chunk}}{T_\text{chunk} + T_\text{warmup}}

For a 250-day chunk, ``T_chunk`` is minutes to hours and
``T_warmup`` is ~60 s — the fraction is effectively 1.  So 30 cores
→ ~30× speedup.
