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
  traffic file is read.
- ``pybtls.post_processing``: ``fit_gev`` and ``fit_gpd``, with the
  ``GEVFit`` and ``GPDFit`` result objects, for extreme-value fitting.
- ``InfluenceLine.set_mode`` and ``InfluenceSurface.set_mode``: a load
  effect can be evaluated in ``"vertical"`` mode (the default),
  ``"centrifugal"`` or ``"braking"``. Braking scales the vertical effect by
  each vehicle's own deceleration where it has one and by the given
  ``braking_factor`` otherwise, and carries the sign of travel, so vehicles
  braking in opposite directions partially cancel. Centrifugal scales by
  ``v^2/g`` and deliberately does not carry that sign, because the force
  points to the outside of the curve for both directions of travel.

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
  end of the window is now authoritative and an over-run block is folded
  back into the final one, in the block-maximum, statistics and POT managers
  alike. A chunked run no longer shifts every later chunk by the inflated
  count. On a two-day congested run this removes a spurious 49th interval
  row and a spurious third block-maximum row.
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
- Writing a vehicle out no longer changes it. ``CVehicle::Write`` assigned
  the normalised transverse position back to the vehicle, and that position
  feeds influence-surface eccentricity, so a load effect could depend on
  whether the vehicle had been serialised first. No output changes in
  practice: the call order was safe.

Changed
^^^^^^^

- Misspelled keyword arguments now raise ``TypeError`` instead of being
  ignored, in ``Simulation.add_sim``, ``TrafficLoader.add_traffic``, the
  vehicle and headway generators, ``LaneFlowComposition.assign_lane_data``,
  ``InfluenceLine.set_IL`` and the garage read/write helpers.

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
- In the AllEvents file, the fixed 3-decimal formatting now applies to the
  event start time only, which is what needs it: at long simulation lengths
  the default formatting loses second-level accuracy. The effect values go
  back to the default 6 significant digits, because the fixed format wrote
  every small effect as "0.000". Effect values therefore change in their
  last digit or two — 293.790 is written as 293.79, and 83.898 as 83.8981 —
  while the number itself is unchanged.

Earlier releases are described on the
`GitHub releases page <https://github.com/ccaprani/btls/releases>`_.
