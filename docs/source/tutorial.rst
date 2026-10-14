.. _chap_tutorial:

*********
Tutorials
*********

The notebooks below build up from the simplest possible run to a full
parallel study, each introducing one new concept on top of the previous
one. If you are new to PyBTLS, work through them in order; each is
self-contained, so you can also jump straight to the topic you need.

**1. Generate traffic** — :doc:`notebooks/minimum_example`
   The smallest complete run: define one lane of traffic, generate a
   month of vehicles, and read them back. No bridge, no load effects -
   just the traffic-generation workflow.

**2. One vehicle, one bridge** — :doc:`notebooks/single_vehicle_example`
   Before simulating thousands of vehicles, see what a single truck
   crossing does: influence lines and surfaces, load effects, and the
   time-history output, with nothing else going on.

**3. The full generation toolkit** — :doc:`notebooks/gen_traffic_example`
   Traffic generation in depth: every vehicle generator (Grave, Garage,
   Nominal), every headway model (free-flow, congested, constant, HeDS),
   multiple lanes and directions, and load-effect calculation with
   several outputs.

**4. Replay recorded traffic** — :doc:`notebooks/read_traffic_example`
   Run the load-effect calculation over a recorded (e.g. WIM) traffic
   file instead of generated traffic.

**5. Long simulations in parallel** — :doc:`notebooks/parallel_sim_example`
   Split a long simulation into day-chunks with ``no_chunk``, run them
   across CPU cores, read the merged result, and save/reload outputs.
   Background reading: :doc:`parallel`.

**6. Extreme value analysis** — :doc:`notebooks/BM` and :doc:`notebooks/POT`
   Statistical post-processing of the simulation outputs:
   block-maxima/GEV fitting and peaks-over-threshold/GP fitting for
   characteristic load effect estimation.

.. toctree::
    :maxdepth: 1
    :hidden:

    notebooks/minimum_example
    notebooks/single_vehicle_example
    notebooks/gen_traffic_example
    notebooks/read_traffic_example
    notebooks/parallel_sim_example
    notebooks/BM
    notebooks/POT
