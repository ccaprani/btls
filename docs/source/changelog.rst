Changelog
=========

1.1.0 (September 2026)
----------------------

The first release carrying the parallel and GPU refactor, together with
traffic-generation and loading fixes from an engine audit. Several
changes alter simulation results, and the output files themselves
changed; read "Output files" below and regenerate any reference results
that depend on the affected paths. The bundled C++ program is versioned
1.3.8.

Added
^^^^^

- ``Simulation.add_sim(no_chunk=...)``: a long simulation is split into
  independent day-chunks that run in parallel and are merged back into a
  single output object. See :doc:`parallel`.
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
- ``OutputConfig.set_fatigue_output(write_residuals=...)``: writes the
  unclosed rainflow residuals that a chunked fatigue run needs in order
  to merge.
- ``pybtls.post_processing``: ``fit_gev`` and ``fit_gpd``, with the
  ``GEVFit`` and ``GPDFit`` result objects, for extreme-value fitting.

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

Changed
^^^^^^^

- **save_output now writes a JSON manifest instead of a binary pickle.**
  ``load_output`` cannot read ``.pkl`` files written by earlier
  versions. Re-save them with the pybtls version that wrote them, or read
  the simulation output text files directly with ``pybtls.output.read`` -
  the manifest only records where those files are.
- Misspelled keyword arguments now raise ``TypeError`` instead of being
  ignored, in ``Simulation.add_sim``, ``TrafficLoader.add_traffic``, the
  vehicle and headway generators, ``LaneFlowComposition.assign_lane_data``,
  ``InfluenceLine.set_IL`` and the garage read/write helpers.

Output files
^^^^^^^^^^^^

The output text files, and the DataFrames the readers build from them,
changed in this release. Existing parsing scripts and any stored results
need to be reviewed against the list below.

- ``read_E_CS`` and ``read_E_IS`` gained "Min" and "Max" columns. The old
  labels were shifted by two positions and were simply wrong, so "Mean",
  "Variance", "Skewness" and "Kurtosis" now return different data than
  before.
- "No. Trucks" is renamed "No. Vehicles" in ``read_TH``, ``read_AE``,
  ``read_FE``, ``read_POT_S`` and the BM and POT event readers, because
  the column has always counted every vehicle, cars included. Scripts
  using the old name raise ``KeyError``. ``read_E_CS`` and ``read_E_IS``
  keep a "No. Trucks" column, but there it means trucks only.
- Every SS_S interval file gains one final interval row, unconditionally.
- BM_S, PT_C and FlowData files gain rows for silent blocks and for the
  tail after the last event.
- A lane whose flow profile has a zero-flow block used to fall silent for
  the rest of the run. It now resumes at the next block that has flow, so
  any diurnal profile with a zero-flow hour generates more traffic than
  before.
- The AllEvents file writes the event start time with fixed 3-decimal
  formatting; the default 6-significant-digit output lost second-level
  accuracy on long runs.

Earlier releases are described on the
`GitHub releases page <https://github.com/ccaprani/btls/releases>`_.
