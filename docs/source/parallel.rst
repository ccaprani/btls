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

.. code-block:: python

   import multiprocessing
   import pybtls

   MASTER_SEED = 12345
   TOTAL_DAYS  = 25000          # e.g. 100 years × 250 days
   WARMUP_SECS = 60.0           # >> max bridge crossing time
   N_WORKERS   = 30             # match your core count

   def run_chunk(args):
       chunk_id, day_start, day_end = args

       # 1. Seed — each worker gets a unique, deterministic stream
       pybtls.lib.libbtls.seed(MASTER_SEED + chunk_id)

       # 2. Build sim as usual (bridge, traffic config, etc.)
       sim = pybtls.Simulation()
       # ... configure bridge, influence lines, traffic model ...

       # 3. Set the time window with a warmup buffer
       #    Start slightly before the chunk so vehicles are already
       #    on the bridge at the chunk boundary.
       sim_start = max(0.0, day_start * 86400.0 - WARMUP_SECS)
       sim_end   = day_end * 86400.0

       # 4. Run
       sim.run(start_time=sim_start, end_time=sim_end)

       # 5. Discard warmup results and return the chunk output
       #    (POT events, block-max events, etc.)
       return sim.get_results(after=day_start * 86400.0)

   if __name__ == "__main__":
       # Divide the timeline into chunks
       days_per_chunk = TOTAL_DAYS // N_WORKERS
       chunks = [
           (i, i * days_per_chunk, (i + 1) * days_per_chunk)
           for i in range(N_WORKERS)
       ]

       # Map across workers (PyBTLS already uses 'spawn')
       with multiprocessing.Pool(N_WORKERS) as pool:
           results = pool.map(run_chunk, chunks)

       # 6. Merge results
       #    - POT: concatenate event lists across chunks
       #    - Block-max: take the maximum across chunks per block
       #    - Stats: combine online-moment accumulators
       merged = merge_results(results)


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
