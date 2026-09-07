"""Flow load-effect statistics (SS_C / SS_S) for the GPU engine.

The C++ engine accumulates, per load effect, the distribution of *per-event
governing values* — ``CEventStatistics`` updates with
``Event.getMaxEffect(iLE).getValue()`` (the signed largest-``|E|`` over the event,
cpp/src/EventManager.cpp:105) for every event with at least one vehicle on the
bridge. That governing value is exactly the GPU engine's POT ``peak_value`` for
the event window, so the statistics reuse the existing per-window reduction —
no extra device pass is needed.

For each effect the accumulator keeps raw power sums (Σx, Σx², Σx³, Σx⁴) about
zero plus min/max and the event/vehicle/truck tallies. Power sums are additive,
so a streamed run merges windows by simple addition (no Welford/Chan merge),
and the central moments are recovered at the end exactly like
``CEventStatistics::finalize()``. ``No. Vehicles`` is the event's on-bridge
count (``win_count``); ``No. Trucks`` excludes class-0 cars (``win_truck_count``).
"""

import numpy as np

__all__ = ["StatsAccumulator"]


def _moments(n, S1, S2, S3, S4):
    """Central moments (mean, M2, M3, M4) from raw power sums about zero."""
    nf = n.astype(float)
    with np.errstate(invalid="ignore", divide="ignore"):
        mean = np.where(nf > 0, S1 / nf, 0.0)
        M2 = S2 - S1 * mean
        M3 = S3 - 3.0 * mean * S2 + 2.0 * nf * mean**3
        M4 = S4 - 4.0 * mean * S3 + 6.0 * mean**2 * S2 - 3.0 * nf * mean**4
    return mean, np.nan_to_num(M2), np.nan_to_num(M3), np.nan_to_num(M4)


def _finalize(n, S1, S2, S3, S4, vmin, vmax):
    """Vectorized CEventStatistics::finalize() over an axis of accumulators."""
    nf = n.astype(float)
    mean, M2, M3, M4 = _moments(n, S1, S2, S3, S4)
    ok = (nf >= 2) & (M2 > 0.0)
    with np.errstate(invalid="ignore", divide="ignore"):
        variance = np.where(ok, M2 / (nf - 1.0), 0.0)
        std = np.sqrt(variance)
        skew = np.where(ok, np.sqrt(nf) * M3 / np.sqrt(M2**3), 0.0)
        kurt = np.where(ok, nf * M4 / M2**2 - 3.0, 0.0)
    mean = np.where(nf >= 1, mean, 0.0)
    lo = np.where(n >= 1, vmin, 0.0)  # CEventStatistics inits min/max to 0 for empty
    hi = np.where(n >= 1, vmax, 0.0)
    return lo, hi, mean, std, variance, skew, kurt


class StatsAccumulator:
    """Per-effect flow statistics over events, streamed window by window.

    ``interval_size`` and ``total_intervals`` are only used when
    ``want_intervals`` is set (SS_S). Interval ``i`` (1-based) covers events with
    start time in ``((i-1)·size, i·size]``, matching CStatsManager::Update's
    strict-`>` interval rollover; ``sim_start`` is the time origin (0, as the
    CPU path anchors both traffic kinds at t=0)."""

    def __init__(
        self,
        n_eff,
        want_intervals=False,
        interval_size=3600.0,
        total_intervals=0,
        sim_start=0.0,
    ):
        self.n_eff = n_eff
        self.sim_start = sim_start
        self.n_events = 0
        self.n_veh = 0
        self.n_trk = 0
        z = lambda: np.zeros(n_eff)
        self.S1, self.S2, self.S3, self.S4 = z(), z(), z(), z()
        self.vmin = np.full(n_eff, np.inf)
        self.vmax = np.full(n_eff, -np.inf)

        self.want_intervals = bool(want_intervals) and total_intervals > 0
        if self.want_intervals:
            self.interval_size = float(interval_size)
            ni = int(total_intervals)
            self.ni = ni
            self.iN = np.zeros(ni, dtype=np.int64)
            self.inveh = np.zeros(ni, dtype=np.int64)
            self.intrk = np.zeros(ni, dtype=np.int64)
            self.iS1 = np.zeros((n_eff, ni))
            self.iS2 = np.zeros((n_eff, ni))
            self.iS3 = np.zeros((n_eff, ni))
            self.iS4 = np.zeros((n_eff, ni))
            self.ivmin = np.full((n_eff, ni), np.inf)
            self.ivmax = np.full((n_eff, ni), -np.inf)

    def update(
        self, peak_value, win_count, win_truck_count, B, time_offset, ev_mask=None
    ):
        """Fold one traffic window's events into the accumulators.

        ``peak_value`` is ``[n_eff, n_win]`` (the per-event governing value),
        ``win_count`` / ``win_truck_count`` are ``[n_win]``, and ``B`` is the
        ``[n_win+1]`` window-boundary times (window-local; ``time_offset`` shifts
        them back to absolute time for interval binning). ``ev_mask`` (optional)
        overrides the default event mask — a streamed runner passes ownership +
        has-a-grid-sample there so seam duplicates and never-sampled windows
        (whose ``peak_value`` is the 0.0 initializer, not a real value) stay out."""
        ev = win_count >= 1 if ev_mask is None else ev_mask
        if not ev.any():
            return
        vals = peak_value[:, ev]  # [n_eff, n_ev]
        wc = win_count[ev].astype(np.int64)
        wt = win_truck_count[ev].astype(np.int64)

        self.n_events += int(ev.sum())
        self.n_veh += int(wc.sum())
        self.n_trk += int(wt.sum())
        self.S1 += vals.sum(axis=1)
        self.S2 += (vals**2).sum(axis=1)
        self.S3 += (vals**3).sum(axis=1)
        self.S4 += (vals**4).sum(axis=1)
        self.vmin = np.minimum(self.vmin, vals.min(axis=1))
        self.vmax = np.maximum(self.vmax, vals.max(axis=1))

        if not self.want_intervals:
            return
        starts = B[:-1][ev] + time_offset - self.sim_start
        # interval i (1-based) covers event starts in ((i-1)·size, i·size] —
        # CStatsManager::Update advances on strict `>` — so bin by ceil. An event
        # starting past the run end (before the A2 boundary) rolls the C++
        # counter into an extra interval, which CStatsManager::FinishAt then
        # folds back into the last interval of the window: clipping here is that
        # same fold, so both engines write exactly one row per interval.
        idx = np.clip(
            np.ceil(starts / self.interval_size).astype(np.int64) - 1, 0, self.ni - 1
        )
        np.add.at(self.iN, idx, 1)
        np.add.at(self.inveh, idx, wc)
        np.add.at(self.intrk, idx, wt)
        for e in range(self.n_eff):
            ve = vals[e]
            np.add.at(self.iS1[e], idx, ve)
            np.add.at(self.iS2[e], idx, ve**2)
            np.add.at(self.iS3[e], idx, ve**3)
            np.add.at(self.iS4[e], idx, ve**4)
            np.minimum.at(self.ivmin[e], idx, ve)
            np.maximum.at(self.ivmax[e], idx, ve)

    # ---- output (matches CStatsManager file layout) -----------------------

    def write_cumulative(self, path):
        n = np.full(self.n_eff, self.n_events, dtype=np.int64)
        lo, hi, mean, std, var, skew, kurt = _finalize(
            n, self.S1, self.S2, self.S3, self.S4, self.vmin, self.vmax
        )
        with open(path, "w") as fh:
            fh.write(
                "    LE   #Events   #Ev Vehs #Ev Trucks    Min      Max"
                "        Mean     StdDev       Variance   Skewness  Kurtosis\n"
            )
            for e in range(self.n_eff):
                fh.write(
                    f"{e + 1:>6}{self.n_events:>10}{self.n_veh:>11}{self.n_trk:>11}"
                    f"{lo[e]:>10.2f}{hi[e]:>10.2f}{mean[e]:>10.2f}{std[e]:>10.2f}"
                    f"{var[e]:>15.2f}{skew[e]:>10.2f}{kurt[e]:>10.2f}\n"
                )

    def write_intervals(self, sim_dir, length_str):
        header = (
            "    ID        Time(s)   #Events   #Ev Vehs #Ev Trucks    Min"
            "      Max        Mean     StdDev       Variance   Skewness  Kurtosis\n"
        )
        for e in range(self.n_eff):
            lo_e, hi_e, mean_e, std_e, var_e, skew_e, kurt_e = _finalize(
                self.iN,
                self.iS1[e],
                self.iS2[e],
                self.iS3[e],
                self.iS4[e],
                self.ivmin[e],
                self.ivmax[e],
            )
            with open(sim_dir / f"SS_S_{length_str}_Eff_{e + 1}.txt", "w") as fh:
                fh.write(header)
                for i in range(self.ni):
                    t = int(round(self.interval_size * (i + 1)))
                    fh.write(
                        f"{i + 1:>6}{t:>15}{self.iN[i]:>10}{self.inveh[i]:>11}"
                        f"{self.intrk[i]:>11}{lo_e[i]:>10.2f}{hi_e[i]:>10.2f}"
                        f"{mean_e[i]:>10.2f}{std_e[i]:>10.2f}{var_e[i]:>15.2f}"
                        f"{skew_e[i]:>10.2f}{kurt_e[i]:>10.2f}\n"
                    )
