"""Vehicle flow statistics (FlowData_{dir}_{lane}.txt) for the GPU engine.

The C++ engine (``CVehicleBuffer::writeFlowData``) writes, per global lane, an
hourly table of vehicle / truck / car counts plus a per-class histogram. Those
are pure traffic counts — no load effect, no device compute — so the GPU
reproduces them from the per-vehicle scalars already extracted with each window
(time, global lane, is-car, and the classifier bin from ``_extract_axle_data`` /
``_generate_and_extract``). Counts are exact (no grid sampling).

The headers below reproduce the two C++ classifiers verbatim so the files read
back through the same ``read_TS`` reader as ``engine="cpu"``.
"""

import numpy as np

__all__ = ["FlowStatsAccumulator"]

# class-histogram bins, matching CVehClassPattern / CVehClassAxle (m_Desc per class)
_PATTERN_BINS = [
    "0: Default",
    "1: Car",
    "2: Pattern 11",
    "3: Pattern 123",
    "4: Pattern 12",
    "5: Pattern 1233",
    "6: Pattern 122",
    "7: Pattern 112",
    "8: Pattern 113",
]
_AXLE_BINS = [
    "0: Default",
    "1: Car",
    "2: 2-axle",
    "3: 3-axle",
    "4: 4-axle",
    "5: 5-axle",
]


def _header(bins):
    h = f"{'Hour':>12}{'#Vehicles':>12}{'#Trucks':>12}{'#Cars':>12}"
    return h + "".join(f"{b:>20}" for b in bins) + "\n"


class FlowStatsAccumulator:
    """Per (hour, global lane) vehicle / truck / car + class-histogram counts,
    streamed window by window. ``classifier_type`` is 0 (axle) or 1 (pattern);
    ``no_lane_dir1`` splits the global lanes into directions for the file names.
    ``hour_origin`` is hour 1's start time — 0 for both traffic kinds, matching
    the C++ ``m_FirstHour`` with the ``start_time=0.0`` the CPU path passes."""

    def __init__(
        self, no_lane, classifier_type, no_lane_dir1, total_hours, hour_origin=0.0
    ):
        self.no_lane = int(no_lane)
        self.no_lane_dir1 = int(no_lane_dir1)
        self.bins = _PATTERN_BINS if classifier_type else _AXLE_BINS
        self.n_bins = len(self.bins)
        self.total_hours = int(total_hours)
        self.hour_origin = float(hour_origin)
        H, L = self.total_hours, self.no_lane
        self.n_veh = np.zeros((H, L), dtype=np.int64)
        self.n_car = np.zeros((H, L), dtype=np.int64)
        self.n_trk = np.zeros((H, L), dtype=np.int64)
        self.hist = np.zeros((H, L, self.n_bins), dtype=np.int64)

    def update(self, vtime, vlane, viscar, viscls):
        """Fold one window's raw per-vehicle arrays (absolute times) into the
        hourly per-lane tallies."""
        hour = ((np.asarray(vtime) - self.hour_origin) / 3600.0).astype(np.int64)
        lane = np.asarray(vlane).astype(np.int64) - 1  # 0-based global lane
        cls = np.asarray(viscls).astype(np.int64)
        ok = (
            (hour >= 0)
            & (hour < self.total_hours)
            & (lane >= 0)
            & (lane < self.no_lane)
            & (cls >= 0)
            & (cls < self.n_bins)
        )
        hour, lane, cls = hour[ok], lane[ok], cls[ok]
        car = np.asarray(viscar).astype(bool)[ok]
        np.add.at(self.n_veh, (hour, lane), 1)
        np.add.at(self.n_car, (hour, lane), car)
        np.add.at(self.n_trk, (hour, lane), ~car)
        np.add.at(self.hist, (hour, lane, cls), 1)

    def write(self, sim_dir):
        head = _header(self.bins)
        for lane in range(self.no_lane):
            d = 1 if lane < self.no_lane_dir1 else 2
            with open(sim_dir / f"FlowData_{d}_{lane + 1}.txt", "w") as fh:
                fh.write(head)
                for h in range(self.total_hours):
                    row = (
                        f"{h + 1:>12}{self.n_veh[h, lane]:>12}"
                        f"{self.n_trk[h, lane]:>12}{self.n_car[h, lane]:>12}"
                    )
                    row += "".join(
                        f"{self.hist[h, lane, b]:>20}" for b in range(self.n_bins)
                    )
                    fh.write(row + "\n")
