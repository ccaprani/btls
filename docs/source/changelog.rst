Changelog
=========

1.2.0 (September 2026)
----------------------

Traffic-generation and loading fixes from an engine audit. Two of them
change simulation results; regenerate any reference results that depend on
the affected paths. The bundled C++ program is versioned 1.3.8.

Fixed
^^^^^

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

- Misspelled keyword arguments now raise ``TypeError`` instead of being
  ignored, in ``Simulation.add_sim``, ``TrafficLoader.add_traffic``, the
  vehicle and headway generators, ``LaneFlowComposition.assign_lane_data``,
  ``InfluenceLine.set_IL`` and the garage read/write helpers.

Earlier releases are described on the
`GitHub releases page <https://github.com/ccaprani/btls/releases>`_.
