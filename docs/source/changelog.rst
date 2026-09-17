Changelog
=========

1.1.0 (September 2026)
----------------------

The first release carrying the GPU engine and the refactored load-effect
calculation, together with traffic-generation and loading fixes from two
engine audits. (Chunked parallel simulation is not new here; it shipped in
1.0.1.) Several
changes alter simulation results, and the output files themselves
changed; read "Output files" below and regenerate any reference results
that depend on the affected paths. The bundled C++ program is versioned
1.3.8.

Added
^^^^^

- ``Simulation.add_sim(engine="cuda"/"mps"/"xpu")``: an experimental GPU
  engine. See :doc:`gpu_engine`.
- ``Simulation.run(show_progress=...)``: progress reporting over the
  queued simulations.
- ``Simulation(overwrite=...)``: replaces a simulation's output directory
  instead of raising ``FileExistsError`` when the tag already exists. The
  default is ``False``, which keeps the existing behaviour — reusing a tag
  would otherwise leave the previous run's files in place to be read back as
  this run's results. All of ``examples/`` now passes ``overwrite=True`` so
  the scripts can be re-run.
- SiWIM CSV traffic input, as ``traffic_format=5`` wherever a recorded
  traffic file is read. Replaying one needs its dates within the BTLS
  calendar, like any recorded traffic (see Fixed).
- ``pybtls.utils.to_btls_calendar`` renumbers a recording dated by the real
  calendar, such as a SiWIM export, into the BTLS calendar so that it can be
  replayed. It keeps working days only, Monday to Friday less any holidays
  given, and numbers them on from 1 January of the first one's year.
- ``pybtls.analyse``: ``fit_gev`` and ``fit_gpd``, with the
  ``GEVFit`` and ``GPDFit`` result objects, for extreme-value fitting, and
  the plots that go with them: ``plot_return_level`` (the fitted curve with
  the observations on the same return period axis), ``plot_qq``, and
  ``plot_mean_residual_life`` and ``plot_parameter_stability``, which are
  how a GPD threshold is chosen.
- ``output.plot.plot_TS`` plots the traffic statistics: the vehicle, truck
  and car counts per hour, and the composition by vehicle class.
- ``output.plot.plot_SV`` plots a single-vehicle simulation, one line per
  lane pass and a subplot per direction, so that the influence line and the
  lane weights the run was made to check can be read off it.
- ``InfluenceLine.set_mode`` and ``InfluenceSurface.set_mode``
  (**experimental**): a load effect can be evaluated in ``"vertical"`` mode
  (the default) or ``"centrifugal"``, which scales the vertical effect by
  each vehicle's own ``v^2/g``. The force is unsigned, because it points to
  the outside of the curve for both directions of travel. The mode is read
  once, by ``Bridge.add_load_effect``, so one influence line can carry a
  different mode for each effect it is added to. This mode has not been
  checked against a reference solution and its sign convention may change
  in a future release; see :doc:`theory`.

Fixed
^^^^^

- ``utils.df_to_vehicle_list`` now matches columns by name. It rebuilds each
  vehicle through a positional C++ call, so a DataFrame whose columns were in
  a different order, or which carried an extra column, was silently read into
  the wrong properties; a frame straight from ``vehicle_list_to_df`` was
  unaffected. Extra columns are now ignored, a missing one is named in the
  error, and the caller's frame is no longer modified in place by the
  GVW / Length refresh.
- **Grave vehicle model, direction-1 lanes (changes results).** The gross
  vehicle weight of trucks in direction-1 lanes was drawn from the
  direction-2 distribution because the lane direction was never handed to
  the generator. Direction-1 lanes now use their own distribution
  (Auxerre 5-axle mean GVW moves from about 426 kN to 387 kN); direction-2
  lanes are unchanged.
- **Congested headway model (changes results).** ``congested_gap_coef_var``
  was used as an absolute standard deviation in seconds. It is again a
  coefficient of variation of the mean congested gap, as documented.
- **Nominal vehicle generator.** ``COV_list=[COV_AS, COV_AW]`` was applied
  crosswise: the axle-spacing COV perturbed axle weights and vice versa.
  Invisible with the default ``[0.05, 0.05]``.
- ``TrafficLoader.add_traffic(use_average_speed=True)`` had no effect; it
  now replaces every vehicle's speed with the file average.
- ``Simulation.add_sim`` reads ``overlap_avoid_distance`` as documented. The
  undocumented ``min_chase_distance`` key it used to read is removed. A
  warning is issued when a bridge is given together with a different
  explicit value, since the bridge length is what gets used.
- ``Simulation.add_sim`` rejects a tag whose output directory is, contains or
  lies inside that of a simulation already queued, including the default
  ``Sim_N`` tags and a chunked simulation's directory. Two such runs used to
  clear or mix each other's files.
- ``active_lane`` must hold lane indices from 1 to the number of lanes. A 0
  or negative index was accepted and, by Python's negative indexing, silently
  simulated another lane; an empty list failed with an unrelated error.
- With ``Simulation(overwrite=True)``, a simulation queued with bad
  arguments no longer clears its previous output directory before the error
  is raised.
- When a simulation fails in a multi-core ``Simulation.run``, the queued
  simulations are cancelled instead of all run to completion before the
  error surfaced, and ``get_output()`` keeps every simulation that finished
  (a chunked simulation only when all its chunks did) instead of returning
  nothing. Interrupting the run with Ctrl+C cancels the queue the same way.
- Worker processes started with ``fork`` (the default on Linux before
  Python 3.14) no longer share the C++ random number generator's state: each
  child is reseeded from OS entropy, so unseeded runs in parallel workers no
  longer generate identical traffic. A seeded run is unaffected.
- C++ program: a run with load effects enabled but an empty bridge file no
  longer sets the no-overlap length to 0 m.
- **Influence surfaces on lanes of unequal width (changes results).** A
  bridge lane maps an axle's transverse position onto the surface using its
  own lane width, taken from the ``lane_position`` pairs given to
  ``set_IS``. Every lane used to read the width of the *first* surface lane
  instead, because the lane number it indexed with is never assigned.
  Nothing changes while all ``lane_position`` widths are equal, which is the
  case for every surface in the examples, tests and documentation. On a
  surface alternating 3.0 m and 4.0 m lanes, a single vehicle on a 4.0 m
  lane moves from 946.1 to 864.6, a change of -8.6%. The GPU engine was
  already correct here; the two engines now agree.
- **HeDS truck arrival rate (changes results).** The HeDS model generates
  trucks only, but the exponential tail of its headway distribution was
  drawn at the *total* flow rate, cars included, so a lane carrying cars
  generated far too many trucks. It now uses the truck flow. The other
  headway models do include cars in their mix and are unaffected, as is a
  HeDS lane with no cars. On lanes with 80 trucks and 400 cars per hour,
  five days of generated traffic falls from 80 276 vehicles to 19 295, a
  change of -76%.
- HeDS no longer reads past the end of its headway table when a block's
  truck flow exceeds the highest tabulated band. ``HeDS.csv`` covers bands
  up to 230 trucks/h; a block above that now uses the top band and warns
  once.
- **Block and interval accounting (changes results).** The driver runs the
  bridge past the end of the simulation window on its final iteration, so an
  event starting after the window closed could open a block of its own. The
  end of the window is now authoritative: the output managers are told the
  simulated end time up front, and an event starting after it is credited to
  the final block, in the block-maximum, statistics and POT managers alike,
  whatever the state of the write buffer (an earlier form of this fix folded
  the over-run block back at the end of the run, which silently dropped it
  when the final block had already been flushed to disk, so the block maxima
  and POT counts depended on ``buffer_size``). A chunked run no longer
  shifts every later chunk by the inflated count. On a two-day congested run
  this removes a spurious 49th interval row and a spurious third
  block-maximum row.
- **Event boundaries and the end of the run (changes results).** An event
  is a period over which the set of vehicles on the bridge is constant. The
  driver loop, in ``Simulation`` and in the C++ program alike, advanced the
  bridge to *every* arrival before testing the vehicle against ``min_gvw``,
  so the arrival of a vehicle that never joined the bridge still cut the
  running event: one truck crossing could yield several POT peaks, several
  fatigue events with truncated ranges and several rows of event
  statistics. The bridge is now advanced only to the arrivals of the
  vehicles put on it. With the default ``min_gvw=0`` nothing changes;
  with a threshold, POT peak counts and the event statistics' event and
  vehicle counts fall, fatigue events span whole crossings, block maxima
  keep their values up to the sampling grid, and the time history and
  rainflow are unchanged beyond that grid. The GPU engine, which always
  split events at composition changes only, now agrees with the CPU.
  The run is the vehicles arriving in ``[start, start + no_day * 86400]``,
  where ``start`` is 0 for generated traffic and midnight of the first
  vehicle's day for recorded traffic: the bridge is
  run on until it empties, so the last vehicles' crossings are recorded
  whether the traffic is generated, cut by ``no_day``, or at the end of a
  recorded file (``Simulation`` used to stop at the last arrival of a file
  that ended early, unlike the C++ program), and the first arrival beyond
  the end is neither simulated nor written to the vehicle file or the flow
  statistics (it used to open one more hour row). GPU flow-statistics hours
  are the C++ ``((h-1)*3600, h*3600]``, so a vehicle arriving on the hour
  is counted in the hour that ends there rather than the next one.
- **Recorded traffic dated after day 0.** ``Simulation`` replayed recorded
  traffic from t = 0 for the file's number of days, on both engines, so a
  file whose first vehicle is not on BTLS day 0 - MON records dated 2019,
  say - simulated nothing, without an error. The replay now starts at
  midnight of the first vehicle's day, as the C++ program always did, and
  keeps the traffic's dates: the outputs carry its absolute times, and the
  block, counter, interval and flow-hour rows count from that day. Traffic
  starting on day 0 is unaffected. Times that large are rounded more
  coarsely, so such a replay can differ in the last printed digit from the
  same traffic dated on day 0, as in the C++ program.
- ``TrafficLoader.sim_day``, the default length of a replay, counts days from
  midnight of the first vehicle's day. Counted from the first arrival, a file
  that starts later in its first day than it ends in its last got a day too
  few, and the replay closed before its last vehicles; such a file now
  replays one more day.
- Replaying recorded traffic dated outside the BTLS calendar raises
  ``ValueError``, and the C++ program stops with an error. BTLS counts 25
  days to a month and 10 months to a year, so a real calendar date after the
  25th or in November or December took the time of a day in the following
  month or year: the vehicle replayed out of order, and the replay could stop
  at it and drop the rest of the file without a warning. Renumber such dates
  into the BTLS calendar first, for instance with ``utils.to_btls_calendar``.
  Reading a garage file does not use the dates and is unaffected.
- ``Simulation.run`` with several cores sends each worker only its own
  simulation. Each task used to pickle the whole ``Simulation``, the bridge and
  traffic of every queued simulation included, so the transfer grew with the
  square of the number of simulations: with eight simulations each replaying
  9 427 recorded vehicles, every task pickled 12.4 MB where its own share was
  1.55 MB. Results are unchanged.
- ``OutputConfig``, and the configuration the generators carry, keep every
  field Python can set when they are pickled, whether sent to a worker process
  or written into a ``save_output`` manifest. The pickle state was a hand-kept
  list that missed six writable fields (``_Road.LANES_FILE``,
  ``_Gen.GEN_TRAFFIC``, ``_Gen.NO_DAYS``, ``_Gen.NO_OVERLAP_LENGTH``,
  ``_Traffic.VEHICLE_MODEL`` and ``_Traffic.HEADWAY_MODEL``), which reset to
  their defaults on the way. pybtls itself never sets them, so no results
  change. The state now comes from the same field table as the bindings, and
  a manifest or pickle written before still loads, a field it lacks keeping
  its default.
- The chunk merge shifts the arrival time of the ``Vehicle`` objects in the
  "Trucks" column of the BM and POT event files by the chunk offset, on
  copies, so they sit on the same timeline as the row's shifted "Time";
  they used to keep their chunk-local time. A chunk whose traffic produced
  no file of an output (a day without a qualifying truck writes no BM_V_*)
  no longer makes the merged output unreadable, and ``get_summary()`` of a
  chunked run lists the union of the chunks' outputs rather than the first
  chunk's.
- The POT counter writes one row per block. It used to split a block across
  an event-buffer flush and emit the same block index more than once; the
  per-block totals were already correct, only the row layout was not. A
  two-day run with a one-day POT block goes from 29 counter rows to 2.
- ``merge_concat`` takes each chunk's index span from the chunk geometry
  rather than from the largest index it happens to observe. ``BM_V`` only
  records blocks that had an event, so a chunk ending in silent blocks used
  to under-shift every later chunk. Only the "Index" column of
  ``BM_by_no_trucks`` frames moves.
- ``read_FE`` pairs each event's two extremes by absolute magnitude, which
  is how the engine selects them. It used to take them in the order the two
  lines appear in the file, which is chronological, so whenever the
  larger-magnitude extreme happened to be written second - routinely, for a
  hogging influence line - the reader labelled the smaller one "Max".
- Comparing two vehicles with ``==`` no longer changes them. The comparison
  serialised both vehicles, serialising writes the normalised transverse
  position back to the vehicle, and that position feeds influence-surface
  eccentricity, so a load effect could depend on whether the vehicle had
  been compared first. The comparison now serialises copies. No output
  changes in practice: the call order was safe.

- ``min_gvw`` is normalised to a whole number of kN by ``add_sim``. The C++
  engine truncated it while the GPU engine compared it as a float, so a
  fractional threshold meant different things on the two engines; it is now
  rejected outright.
- ``torch`` is no longer part of the ``test`` extra. ``cibuildwheel``
  installs that extra into every wheel's test environment, which would pull
  a multi-gigabyte torch into each of them and buy nothing: no release
  runner has a GPU, so the CUDA tests skip there whatever is installed. CI
  installs torch explicitly on the one leg that requires it, so the GPU
  engine keeps its coverage.
- ``pybtls.analyse`` is listed in the API documentation.

Changed
^^^^^^^

- Misspelled keyword arguments now raise ``TypeError`` instead of being
  ignored, in ``Simulation.add_sim``, ``TrafficLoader.add_traffic``, the
  vehicle and headway generators, ``LaneFlowComposition.assign_lane_data``,
  ``InfluenceLine.set_IL`` and the garage read/write helpers.
- ``Simulation.add_sim`` validates its arguments when the simulation is
  queued, so a bad one raises from ``add_sim`` rather than from ``run``. A
  ``bridge`` that is not a ``Bridge`` now raises ``TypeError``; it used to
  run the simulation without load effects.
- The package metadata declares the licence as the SPDX expression
  ``GPL-3.0-only`` (PEP 639), in place of the free text "GNU GPL v3", and
  names ``LICENSE`` as the licence file.
- ``Simulation`` no longer sets the process-wide ``multiprocessing`` start
  method to ``spawn``. ``run`` already starts its workers with an explicit
  spawn context, and the global setting changed how the calling program's
  own process pools started.
- ``save_output`` writes a JSON manifest, holding each output's directory and
  configuration, instead of a pickle file; the data stays in the output text
  files, and a manifest stays readable across pybtls versions. ``save_output``
  writes to a temporary file first, so a failed save leaves an existing
  manifest intact. ``load_output`` reads manifests only and raises
  ``RuntimeError`` for any other file, and for a manifest of a format version
  it does not know. A
  ``.pkl`` file saved by 1.0.1 or earlier is read with the new
  ``load_legacy_output``, which is deprecated: it issues a ``FutureWarning``
  and may be removed in a future release, so load such a file once and
  re-save it with ``save_output``. Only load a pickle file you trust, because
  unpickling can run arbitrary code.
- **Linux wheels need glibc 2.27 or newer.** They are built in the
  ``manylinux_2_28`` image rather than cibuildwheel's ``manylinux2014``
  default; auditwheel tags the result
  ``manylinux_2_27_x86_64.manylinux_2_28_x86_64``, so the floor rises from
  glibc 2.17 in 1.0.1 to 2.27. The 2.17 baseline can no longer be built or
  tested: pillow, which matplotlib pulls in, ships no ``manylinux_2_17``
  wheel from 12.3.0 on, and neither does numpy or contourpy for CPython
  3.11 and later, so pip falls back to their source distributions inside
  the build image and that build fails. CentOS and RHEL 7 (glibc 2.17),
  Debian 9 (2.24) and Amazon Linux 2 (2.26) fall below the new floor and
  build the PyBTLS source distribution instead, which needs GCC 9 or
  newer. Ubuntu 18.04 (2.27), Debian 10 and RHEL 8 (2.28) still get a
  wheel. See :doc:`startup`.
- **macOS wheels are arm64 only.** 1.0.1 also published x86_64 wheels, but
  the GitHub runner that built them stopped being available, so Intel Macs
  install from the source distribution now.

Output files
^^^^^^^^^^^^

The output text files, and the DataFrames the readers build from them,
changed in this release. Existing parsing scripts and any stored results
need to be reviewed against the list below.

- "No. Trucks" is renamed "No. Vehicles" in ``read_TH``, ``read_AE``,
  ``read_FE``, ``read_POT_S`` and the BM and POT event readers, because
  the column has always counted every vehicle, cars included. Scripts
  using the old name raise ``KeyError``. ``read_E_CS`` and ``read_E_IS``
  keep a "No. Trucks" column, but there it means trucks only.
- The POT counter file (PT_C) has one row per block. Earlier versions wrote
  a row per event-buffer flush, so a block could appear several times: a
  two-day congested run wrote 29 rows where it now writes 2.
- The vehicle file and the flow statistics (FlowData) end with the last
  vehicle arriving within the simulated window: the first arrival beyond it
  is no longer written or counted, so a one-day run has 24 FlowData rows.
- ``read_BM_S`` pads a block whose row stops short of a later block's
  buckets with 0.0, the value the engine writes for a bucket it opened but
  never filled, instead of NaN: the block was simulated and had no event
  of that size. ``fit_gev`` counts such blocks as observed, so a
  ``read_data("BM_summary")`` column whose higher truck-count buckets are
  sparse now yields the return level of the full simulated length; it used
  to drop those blocks from the count and overstate the exceedance rate.
  The merged BM_summary of a chunked run fills a bucket a chunk never
  opened the same way.
- In the AllEvents file, the fixed 3-decimal formatting now applies to the
  event start time only, which is what needs it: at long simulation lengths
  the default formatting loses second-level accuracy. The effect values go
  back to the default 6 significant digits, because the fixed format wrote
  every small effect as "0.000". Effect values therefore change in their
  last digit or two — 293.790 is written as 293.79, and 83.898 as 83.8981 —
  while the number itself is unchanged.

Earlier releases are described on the
`GitHub releases page <https://github.com/ccaprani/btls/releases>`_.
