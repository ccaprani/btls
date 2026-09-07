"""
Orchestration for the experimental GPU engine: extract influence-line specs
from a bridge, stream the traffic in RAM-bounded day windows, run the GPU
load-effect computation per window, and write the per-effect block maxima /
peaks-over-threshold so the result reads back through the standard
``_OutputManager``.

Generated traffic is processed one window at a time (never fully materialized),
so peak host memory is bounded by the window size — set from available RAM — and
is independent of the simulated length. This keeps the GPU subsystem
self-contained in ``pybtls.gpu``; Simulation only dispatches to :func:`run`.
"""

import heapq
import os
import sys
import time

import numpy as np

from ..bridge import Bridge
from ..bridge.influence_line import InfluenceLine, InfluenceSurface
from ..traffic import TrafficGenerator, TrafficLoader
from ..output import OutputConfig, _OutputManager
from .._resource import available_host_memory
from . import pot as potmod
from .engine import compute_pot
from .stats import StatsAccumulator
from .flow import FlowStatsAccumulator

SECONDS_PER_DAY = 86400.0


def _il_to_spec(il):
    """Convert one InfluenceLine / InfluenceSurface to an il_spec dict. The
    per-axle force mode (vertical / centrifugal / braking) and braking_factor are
    carried along so the device applies the same per-axle force coefficient the
    C++ engine does (cpp/src/InfluenceLine.cpp:getAxleLoadEffect)."""
    mode = {
        "mode": getattr(il, "_load_effect_mode", "vertical"),
        "braking_factor": float(getattr(il, "_braking_factor", 0.0)),
    }
    if isinstance(il, InfluenceSurface):
        M = np.asarray(il._data_dict["IS_matrix"], dtype=float)
        lp = il._data_dict["lane_position"]
        return {
            "kind": "surface",
            "X": M[1:, 0],
            "Y": M[0, 1:],
            "ISords": M[1:, 1:],
            "lane_centre": np.array([(a + b) / 2.0 for a, b in lp]),
            "lane_width": np.array([abs(b - a) for a, b in lp]),
            **mode,
        }
    if isinstance(il, InfluenceLine) and il._data_dict.get("position") is not None:
        return {
            "kind": "discrete",
            "pos": np.asarray(il._data_dict["position"], dtype=float),
            "ord": np.asarray(il._data_dict["ordinate"], dtype=float),
            **mode,
        }
    if isinstance(il, InfluenceLine) and il._data_dict.get("id") is not None:
        return {
            "kind": "builtin",
            "id": int(il._data_dict["id"]),
            "length": float(il._data_dict["length"]),
            **mode,
        }
    raise NotImplementedError(
        "engine='cuda' (experimental) supports discrete/built-in influence lines "
        "and influence surfaces only."
    )


def _check_il_length(spec, bridge_length, lane, load_case):
    """Reject an influence line whose own span is not the bridge length — the
    same centimetre-level check the CPU path makes in Bridge._get_bridge. The
    GPU path never builds the C++ bridge, so without it a mismatched influence
    line is silently resampled onto the bridge grid (zero-filled or truncated)
    instead of being refused."""
    if spec["kind"] == "discrete":
        il_length = float(spec["pos"][-1])
    elif spec["kind"] == "surface":
        il_length = float(spec["X"][-1] - spec["X"][0])
    else:
        il_length = float(spec["length"])
    if not np.isclose(il_length, bridge_length, atol=1e-2):
        raise RuntimeError(
            f"Influence line or surface for lane {lane} load case {load_case} "
            "is shorter or longer than the bridge."
        )


def _il_specs_from_bridge(bridge):
    """Extract one il_spec + weight per load effect. Effects whose influence line
    or weight differs across lanes become a per-lane spec (matching the C++
    engine's per-lane summation)."""
    il_specs, weights = [], []
    for key in sorted(bridge._inf_file_dict, key=int):
        ils = bridge._inf_file_dict[key]["inf_line"]
        wts = [float(w) for w in bridge._inf_file_dict[key]["weight"]]
        uniform = all(il is ils[0] for il in ils) and all(w == wts[0] for w in wts)
        if uniform:
            spec = _il_to_spec(ils[0])
            _check_il_length(spec, bridge.length, 1, key)
            il_specs.append(spec)
            weights.append(wts[0])
        else:
            lane_specs = [_il_to_spec(il) for il in ils]
            if any(s["kind"] == "surface" for s in lane_specs):
                raise NotImplementedError(
                    "engine='cuda' (experimental) does not support per-lane influence "
                    "surfaces (a single surface already spans all lanes)."
                )
            for i, s in enumerate(lane_specs):
                _check_il_length(s, bridge.length, i + 1, key)
            if len({(s["mode"], s["braking_factor"]) for s in lane_specs}) > 1:
                # the device applies one force coefficient per load effect, so a
                # mixed-mode per-lane effect would silently use lane 1's mode
                raise NotImplementedError(
                    "engine='cuda' (experimental) does not support per-lane "
                    "load-effect modes: every lane's influence line for one load "
                    "effect must use the same mode and braking_factor."
                )
            il_specs.append(
                {
                    "kind": "per_lane",
                    "lane_specs": lane_specs,
                    "lane_weights": wts,
                    "mode": getattr(ils[0], "_load_effect_mode", "vertical"),
                    "braking_factor": float(getattr(ils[0], "_braking_factor", 0.0)),
                }
            )
            weights.append(1.0)  # folded into lane_weights
    return il_specs, weights


def _collect_vehicles(traffic, bridge, no_day, active_lane, seed):
    """Return (vehicles, n_days). Generated traffic is fully materialized."""
    if isinstance(traffic, TrafficLoader):
        lanes = traffic._lanes_vehicles
        if active_lane is not None:
            lanes = [lanes[i - 1] for i in active_lane]
        vehicles = [v for lane in lanes for v in lane]
        n_days = int(no_day) if no_day is not None else traffic.sim_day
        return vehicles, n_days

    if isinstance(traffic, TrafficGenerator):
        if no_day is None:
            raise ValueError("engine='cuda' with a TrafficGenerator requires no_day.")
        if seed is not None:
            from ..lib import libbtls

            libbtls.seed(seed)
        n_days = int(no_day)
        end_time = n_days * SECONDS_PER_DAY
        lane_list = traffic._get_traffic_generator(bridge.length)
        lanes = (
            lane_list
            if active_lane is None
            else [lane_list[i - 1] for i in active_lane]
        )
        vehicles = []
        current_time = 0.0
        while current_time <= end_time:
            lanes = sorted(lanes, key=lambda t: t.getNextArrivalTime())
            v = lanes[0].getNextVehicle()
            if v is None:
                break
            vehicles.append(v)
            current_time = v.get_time()
        return vehicles, n_days

    raise NotImplementedError(
        "engine='cuda' supports TrafficLoader or TrafficGenerator traffic."
    )


def _new_bm_state(total_blocks, n_eff):
    """Run-wide block-maxima accumulator: per (block, effect, event-type) the
    governing value (event type = the event's number of vehicles, as in
    CBlockMaxManager), plus the per-block number of event types seen."""
    return {
        "vals": np.zeros((total_blocks, n_eff, 4)),  # slot axis grows on demand
        "slots": np.zeros(total_blocks, dtype=np.int64),
    }


def _accumulate_bm(bm_state, pot, ev_mask, block_secs, time_offset, total_blocks):
    """Fold one window's owned events into the block-maxima accumulator,
    replicating CBlockMaxManager::Update: each event is credited to the block
    containing its START time (block b covers ((b-1)·size, b·size], strict-`>`
    rollover), into the slot for its number of vehicles, replacing the stored
    value when ``|new| >= |old|`` (ties go to the later event — windows arrive
    in time order, so replacing on ``>=`` preserves that within and across
    windows)."""
    if not ev_mask.any():
        return
    starts = pot["B"][:-1][ev_mask] + time_offset
    vals = pot["peak_value"][:, ev_mask]  # [n_eff, k]
    slot = pot["win_count"][ev_mask].astype(np.int64) - 1  # 0-based event type
    bidx = np.clip(
        np.ceil(starts / block_secs).astype(np.int64) - 1, 0, total_blocks - 1
    )

    while bm_state["vals"].shape[2] <= slot.max():
        bm_state["vals"] = np.concatenate(
            [bm_state["vals"], np.zeros_like(bm_state["vals"])], axis=2
        )
    np.maximum.at(bm_state["slots"], bidx, slot + 1)

    vals_st = bm_state["vals"]
    n_eff = vals.shape[0]
    key = bidx * vals_st.shape[2] + slot  # (block, slot) group id
    for e in range(n_eff):
        # per group keep the |max|, later-start-on-tie candidate: sort by
        # (group, |value|, start) and take each group's last row
        order = np.lexsort((starts, np.abs(vals[e]), key))
        k_sorted = key[order]
        sel = order[np.r_[k_sorted[1:] != k_sorted[:-1], True]]
        cur = vals_st[bidx[sel], e, slot[sel]]
        new = vals[e][sel]
        put = np.abs(new) >= np.abs(cur)  # >= : the (later) window wins ties
        vals_st[bidx[sel][put], e, slot[sel][put]] = new[put]


def _write_bm_summary(sim_dir, length_str, bm_state, n_eff):
    """Write BM_summary files in the C++ engine's format: per block one row with
    the block number then one value column per event type seen in that block
    (CBlockMaxManager::WriteSummaryFiles)."""
    vals, slots = bm_state["vals"], bm_state["slots"]
    for e in range(n_eff):
        with open(sim_dir / f"BM_S_{length_str}_Eff_{e + 1}.txt", "w") as fh:
            for b in range(vals.shape[0]):
                fh.write(
                    f"{b + 1}\t"
                    + "".join(f"{vals[b, e, s]:.1f}\t\t" for s in range(slots[b]))
                    + "\n"
                )


def _write_fatigue(sim_dir, length_str, rainflows, decimal, cutoff, residuals):
    """Write FR_*.txt (the cycle-amplitude histogram) in the C++
    CFatigueManager format. The streamed run carried the residual across all
    windows; normally a single final close (ASTM end-of-data rule) matches a
    sequential run. With ``residuals`` (chunk mode, WRITE_RAINFLOW_RESIDUALS)
    the residual is instead left open and written to an FRR_* sidecar — same
    as CFatigueManager::writeResidualFiles — so chunked runs splice exactly."""
    for e, rf in enumerate(rainflows):
        rf.calcCycles(not residuals)
        if residuals:
            with open(sim_dir / f"FRR_{length_str}_{e + 1}.txt", "w") as fh:
                fh.write(f"{decimal}\t{cutoff:.17g}\n")
                for rev in rf.getResiduals():
                    fh.write(f"{rev:.17g}\n")
        hist = rf.getRainflowOutput()  # dict: rounded range -> cycle count
        with open(sim_dir / f"FR_{length_str}_{e + 1}.txt", "w") as fh:
            fh.write(f"{'Amplitude':>15}{'No. Cycles':>15}\n")
            for rng in sorted(hist):
                fh.write(f"{rng:>15.{decimal}f}{hist[rng]:>10.1f}\n")


def _pot_events_per_effect(pot, thresholds, ev_mask):
    """Per effect, the window indices of above-threshold events, in time order.
    ``ev_mask`` is the runner's event mask (>=1 vehicle, >=1 grid sample, owned
    by this window); on top of it an event needs a signed peak above the
    threshold (matching CPOTManager::Update's ``getValue() > thr``)."""
    pv = pot["peak_value"]
    out = []
    for e in range(pv.shape[0]):
        mask = ev_mask & (pv[e] > thresholds[e])
        out.append(np.nonzero(mask)[0])  # ascending window index == time order
    return out


def _lead_dist(members, t, veh, bridge_length):
    """Best-effort lead-axle position at time ``t`` (the PT_V ``dist`` column):
    the first-axle position of the earliest-departing member vehicle. Grid-noisy,
    forensic only — not used by the EVA statistics."""
    if len(members) == 0:
        return 0.0
    lead = members[np.argmin(veh["t_off"][members])]
    sp, on, sgn = veh["speed"][lead], veh["t_on"][lead], veh["sign"][lead]
    datum0 = on if sgn > 0 else on + bridge_length / sp
    return float(sgn * sp * (t - datum0))


class _PotStream:
    """Streams the POT outputs in the C++ POTManager format, window by window:
    PT_S / PT_V rows are appended to their open files as each window's events
    are folded in (events arrive in time order, so appending preserves the
    sequential layout), and PT_C is a bounded (n_counter_blocks x n_eff) tally
    written on close — host memory for POT stays bounded by one window
    regardless of the simulated length."""

    def __init__(self, sim_dir, length_str, out, n_eff, counter_secs, n_counter_blocks):
        self.n_eff = n_eff
        self.counter_secs = counter_secs
        self.n_counter_blocks = n_counter_blocks
        self.n_events = [0] * n_eff  # per-effect event counter (row numbering)
        self.counts = (
            np.zeros((n_counter_blocks, n_eff), dtype=np.int64)
            if out.POT.WRITE_POT_COUNTER
            else None
        )
        self.s_files = (
            [
                open(sim_dir / f"PT_S_{length_str}_Eff_{e + 1}.txt", "w")
                for e in range(n_eff)
            ]
            if out.POT.WRITE_POT_SUMMARY
            else None
        )
        self.v_files = (
            [
                open(sim_dir / f"PT_V_{length_str}_{e + 1}.txt", "w")
                for e in range(n_eff)
            ]
            if out.POT.WRITE_POT_VEHICLES
            else None
        )
        self._pt_c_path = sim_dir / f"PT_C_{length_str}.txt"

    def update(
        self,
        pot,
        thresholds,
        ev_mask,
        time_step,
        time_offset,
        vehicles,
        file_format,
        bridge_length,
        grid_phase=0.0,
    ):
        """Fold one window's owned above-threshold events in, converting
        window-local times to absolute. PT_V member-vehicle lines are
        serialized now, while the window's vehicles are still in memory.
        ``grid_phase`` is the window's sample-grid phase (see
        :func:`engine.compute_from_axles`): a peak at sample index ``k`` is at
        absolute time ``k*ts + grid_phase + time_offset``."""
        events = _pot_events_per_effect(pot, thresholds, ev_mask)
        pv, pix, win_count, B = (
            pot["peak_value"],
            pot["peak_index"],
            pot["win_count"],
            pot["B"],
        )
        n_eff = self.n_eff
        if self.v_files is not None:
            veh, L = pot["veh"], bridge_length
            kept_idx, t_on = veh["kept_idx"], veh["t_on"]
            indptr, members = potmod.window_members_csr(
                pot["k_start"], pot["k_end"], len(B) - 1
            )
        for e in range(n_eff):
            for w in events[e]:
                self.n_events[e] += 1
                i = self.n_events[e]
                time_abs = float(pix[e, w] * time_step + grid_phase + time_offset)
                value = float(pv[e, w])
                no_trucks = int(win_count[w])
                if self.s_files is not None:
                    self.s_files[e].write(
                        f"{i:>6}{time_abs:>15.1f}{no_trucks:>4}{value:>10.1f}\n"
                    )
                if self.v_files is not None:
                    mem = members[indptr[w] : indptr[w + 1]]
                    mem = mem[np.argsort(t_on[mem], kind="stable")]  # by arrival
                    veh_lines = [
                        vehicles[int(kept_idx[m])].write(file_format) for m in mem
                    ]
                    fh = self.v_files[e]
                    fh.write(f"{i}\n")
                    for k in range(n_eff):  # C++ writes a block for ALL effects
                        # dist uses window-LOCAL time (veh windows are local);
                        # the printed time is absolute (local + offset)
                        t_k = (
                            float(pix[k, w] * time_step + grid_phase + time_offset)
                            if pix[k, w] >= 0
                            else 0.0
                        )
                        d_k = _lead_dist(
                            mem,
                            (
                                float(pix[k, w] * time_step + grid_phase)
                                if pix[k, w] >= 0
                                else 0.0
                            ),
                            veh,
                            L,
                        )
                        fh.write(
                            f"{k + 1:>2}{float(pv[k, w]):>10.1f}{t_k:>15.1f}"
                            f"{d_k:>10.2f}{no_trucks:>4}\n"
                        )
                        for line in veh_lines:
                            fh.write(line + "\n")
            if self.counts is not None:
                for w in events[e]:
                    # counter block b covers ((b-1)·size, b·size] by event start
                    # (CPOTManager::Update's strict-`>` rollover), so bin by ceil
                    cb = int(np.ceil((B[w] + time_offset) / self.counter_secs)) - 1
                    if cb < 0:
                        cb = 0
                    if cb >= self.counts.shape[0]:
                        # an event starting beyond the run end (before the A2
                        # boundary) opens a new counter block, exactly as
                        # CPOTManager::Update's silent-block fill does
                        grow = np.zeros(
                            (cb + 1 - self.counts.shape[0], self.n_eff),
                            dtype=np.int64,
                        )
                        self.counts = np.vstack([self.counts, grow])
                    self.counts[cb, e] += 1

    def close(self):
        for files in (self.s_files, self.v_files):
            if files is not None:
                for fh in files:
                    fh.close()
        if self.counts is not None:
            with open(self._pt_c_path, "w") as fh:
                fh.write(
                    "Block\t"
                    + "".join(f"LE {e + 1}\t" for e in range(self.n_eff))
                    + "\n"
                )
                for b in range(self.counts.shape[0]):
                    fh.write(
                        f"{b + 1}\t"
                        + "".join(f"{self.counts[b, e]}\t" for e in range(self.n_eff))
                        + "\n"
                    )


def _free_device_bytes(device):
    """Free VRAM on the compute device now, or None if it is not a CUDA device
    (the per-day device tiling means MPS/XPU are bounded by RAM in practice)."""
    try:
        import torch

        if torch.device(device).type == "cuda" and torch.cuda.is_available():
            free, _total = torch.cuda.mem_get_info(torch.device(device))
            return int(free)
    except Exception:
        pass
    return None


# Output flags the GPU engine does not produce (it targets the aggregate
# analysis outputs; the per-event / per-vehicle detail outputs are exactly the
# I/O-bound ones for which the GPU does not pay off — use engine="cpu" for them).
_UNSUPPORTED_OUTPUTS = (
    ("WRITE_EACH_EVENT", "every-event output (set_event_output write_each_event)"),
    ("VehicleFile.WRITE_VEHICLE_FILE", "vehicle file (set_vehicle_file_output)"),
    ("BlockMax.WRITE_BM_VEHICLES", "block-max vehicles (set_BM_output write_vehicle)"),
    ("BlockMax.WRITE_BM_MIXED", "block-max mixed (set_BM_output write_mixed)"),
    ("WRITE_FATIGUE_EVENT", "fatigue events (set_fatigue_output write_fatigue_event)"),
)


def _warn_unsupported_outputs(out):
    """Warn (once, to stderr) about configured outputs the GPU engine silently
    skips, so the missing files are not a surprise."""
    missing = []
    for path, label in _UNSUPPORTED_OUTPUTS:
        obj = out
        for attr in path.split("."):
            obj = getattr(obj, attr, False)
        if obj:
            missing.append(label)
    if missing:
        print(
            "Warning: engine='cuda' does not produce these requested outputs, "
            "they will be skipped (use engine='cpu' for them):\n  - "
            + "\n  - ".join(missing),
            file=sys.stderr,
            flush=True,
        )


def _window_target_vehicles(n_eff, device, want_pot):
    """Per-window vehicle budget = the smaller of a host-RAM and a VRAM budget,
    each a fraction of *free* memory (so it adapts to the machine and to whatever
    else is running). Biased toward safety: an over-large window OOMs (fatal),
    an over-small one only costs a little speed, so the fraction is well under 1
    and the per-vehicle estimates are generous. A budget whose free memory the
    platform will not report is left out; with neither, the floor applies."""
    FRACTION = 0.5
    HOST_BYTES = 2500  # Python vehicle + numpy axle arrays / veh
    VRAM_BYTES = 400 + (
        60 * n_eff if want_pot else 0
    )  # device axle arrays + POT accumulators / veh

    targets = []
    free_host = available_host_memory()  # None where the OS will not report it
    if free_host is not None:
        targets.append(int(FRACTION * free_host) // HOST_BYTES)
    free_vram = _free_device_bytes(device)
    if free_vram is not None:
        targets.append(int(FRACTION * free_vram) // VRAM_BYTES)
    return max(100_000, min(targets)) if targets else 100_000


def _pull_beyond_end(lanes, end_time):
    """Pull generated vehicles until the first arrival STRICTLY after
    ``end_time`` (the CPU loop's final ``while current_time <= end_time``
    iteration fetches exactly that vehicle) and return it. The earliest lane is
    pulled first each step; ``min()`` resolves ties to the lowest lane, like the
    CPU's stable sort and the C++ ``_earliest_lane``. An arrival exactly at
    ``end_time`` (in-sim on the CPU; measure-zero for continuous headways) is
    consumed but not simulated."""
    while True:
        v = min(lanes, key=lambda t: t.getNextArrivalTime()).getNextVehicle()
        if v.get_time() > end_time:
            return v


def _vehicle_stream(traffic, bridge, active_lane, seed, end_time, tail=None):
    """Yield vehicles in arrival-time order, lazily, up to ``end_time`` — for both
    recorded (merge the pre-loaded per-lane lists) and generated (pull the
    generator one vehicle at a time) traffic. Generated traffic is never fully
    materialized: the caller holds only the window being processed.

    For generated traffic, ``tail`` (optional dict) receives, once the stream is
    exhausted, the first vehicle arriving after ``end_time`` (``tail["vehicle"]``)
    — the CPU end-of-run boundary A2 (see :func:`_pull_beyond_end`)."""
    if isinstance(traffic, TrafficLoader):
        lanes = traffic._lanes_vehicles
        if active_lane is not None:
            lanes = [lanes[i - 1] for i in active_lane]
        for v in heapq.merge(*lanes, key=lambda x: x.get_time()):
            # the CPU loop (`while current_time <= end_time`) still processes a
            # vehicle arriving at exactly end_time, so use a strict `>` cut-off
            if v.get_time() > end_time:
                break
            yield v
    elif isinstance(traffic, TrafficGenerator):
        from ..lib import libbtls

        if seed is not None:
            libbtls.seed(seed)
        lane_list = traffic._get_traffic_generator(bridge.length)
        lanes = (
            lane_list
            if active_lane is None
            else [lane_list[i - 1] for i in active_lane]
        )
        # generate in day-sized bulk passes: the C++ pulls the globally-earliest
        # lane each step, so the stream (and RNG draw order) is identical to a
        # per-vehicle Python loop, but without the per-vehicle Python<->C++ cost.
        chunk = 0.0
        while chunk < end_time:
            chunk_end = min(chunk + SECONDS_PER_DAY, end_time)
            for v in libbtls._generate_traffic_stream(lanes, chunk_end):
                yield v
            chunk = chunk_end
        if tail is not None and lanes:
            tail["vehicle"] = _pull_beyond_end(lanes, end_time)
    else:
        raise NotImplementedError(
            "engine='cuda' supports TrafficLoader or TrafficGenerator traffic."
        )


def _vehicle_windows(traffic, bridge, n_days, active_lane, seed, target, tail=None):
    """Yield (vehicles, day_offset, window_days): contiguous whole-day windows
    each holding about ``target`` vehicles (a vehicle goes to the window
    containing its arrival day). Bounds host memory to one window for generated
    traffic, independent of ``n_days``. ``tail`` is passed through to
    :func:`_vehicle_stream` (generated end-of-run boundary)."""
    end_time = n_days * SECONDS_PER_DAY
    stream = _vehicle_stream(traffic, bridge, active_lane, seed, end_time, tail)
    day0 = 0
    carry = None
    exhausted = False
    while day0 < n_days and not exhausted:
        win_end_day = min(day0 + 1, n_days)
        vehicles = []
        if carry is not None:
            vehicles.append(carry)
            carry = None
        for v in stream:
            vday = int(v.get_time() // SECONDS_PER_DAY)
            if vday < win_end_day:
                vehicles.append(v)
                continue
            # vehicle is past the current window end: grow the window in whole
            # days while still under the vehicle budget, else carry it over
            if len(vehicles) < target and win_end_day < n_days:
                while win_end_day < n_days and vday >= win_end_day:
                    win_end_day = min(win_end_day + 1, n_days)
                if vday < win_end_day:
                    vehicles.append(v)
                    continue
            carry = v
            break
        else:
            exhausted = True
        yield vehicles, day0, win_end_day - day0
        day0 = win_end_day


def _array_windows(
    traffic, bridge, n_days, active_lane, seed, target, classifier, tail=None
):
    """Fast generated-traffic path: fuse generate+extract per day (no Python
    Vehicle objects) and accumulate whole days into a window of about ``target``
    vehicles. Yields (extracted, day_offset, window_days) where ``extracted`` is
    the per-window vehicle/axle array tuple. Day boundaries match
    :func:`_vehicle_windows` (a vehicle goes to the window containing its arrival
    day), so the result is identical to the per-vehicle path.

    ``tail`` (optional dict) receives, after the last window, the first arrival
    beyond the run end — the CPU end-of-run boundary A2 — as ``tail["arrival"]``
    (absolute time) and ``tail["extracted"]`` (its single-vehicle extraction
    tuple, for the flow-statistics count)."""
    from ..lib import libbtls

    if seed is not None:
        libbtls.seed(seed)
    lane_list = traffic._get_traffic_generator(bridge.length)
    lanes = (
        lane_list if active_lane is None else [lane_list[i - 1] for i in active_lane]
    )
    no_lane = bridge.no_lane
    day0 = 0
    while day0 < n_days:
        win_end_day = min(day0 + 1, n_days)
        chunks = []
        count = 0
        while True:  # grow the window in whole-day steps up to ~target
            chunk = libbtls._generate_and_extract(
                lanes, win_end_day * SECONDS_PER_DAY, no_lane, classifier
            )
            chunks.append(chunk)
            count += len(chunk[0])
            if count >= target or win_end_day >= n_days:
                break
            win_end_day = min(win_end_day + 1, n_days)
        extracted = (
            chunks[0]
            if len(chunks) == 1
            else tuple(
                np.concatenate([c[k] for c in chunks]) for k in range(len(chunks[0]))
            )
        )
        yield extracted, day0, win_end_day - day0
        day0 = win_end_day
    if tail is not None and lanes:
        v = _pull_beyond_end(lanes, n_days * SECONDS_PER_DAY)
        tail["arrival"] = float(v.get_time())
        tail["extracted"] = libbtls._extract_axle_data([v], no_lane, classifier)


_PER_AXLE_IDX = (9, 10, 11)  # aw, asp, at in the extraction tuple


def _subset_extracted(extracted, vmask):
    """Row-subset an extraction tuple by a per-vehicle mask (per-axle arrays
    are masked via each vehicle's axle count)."""
    amask = np.repeat(vmask, extracted[8])
    return tuple(
        arr[amask] if i in _PER_AXLE_IDX else arr[vmask]
        for i, arr in enumerate(extracted)
    )


def _concat_extracted(a, b):
    return tuple(np.concatenate([x, y]) for x, y in zip(a, b))


def _seam_context(raw_windows, first_time, carry_of, concat, count, end_arrival=None):
    """Attach window-seam context to a raw (data, day0, win_days) window stream.

    Yields (data, day0, win_days, n_carried, next_arrival, is_last) where
    ``data`` gains the previous window's tail vehicles that are still crossing
    at the seam (``n_carried`` of them, prepended — load context only, their
    events belong to the previous window) and ``next_arrival`` is the following
    window's first arrival time (absolute), the boundary that ends the event
    straddling the seam. Windows whose first arrival is still unknown (empty
    successors) are held back until one arrives. ``is_last`` marks the run's
    final window.

    ``end_arrival`` (optional callable, evaluated once the raw stream is
    exhausted) supplies the first arrival beyond the run end — the generated
    traffic's A2 boundary — used as the trailing windows' ``next_arrival``;
    without it (or when it returns None) the trailing windows get ``inf``."""
    pending = []
    carry = None
    for data, day0, win_days in raw_windows:
        t0 = first_time(data)
        if t0 is not None:
            for p in pending:
                yield (*p, t0, False)
            pending = []
        n_carried = 0
        if carry is not None and count(carry):
            n_carried = count(carry)
            data = concat(carry, data)
        carry = carry_of(data, (day0 + win_days) * SECONDS_PER_DAY)
        pending.append((data, day0, win_days, n_carried))
    t_end = end_arrival() if end_arrival is not None else None
    if t_end is None:
        t_end = np.inf
    for i, p in enumerate(pending):
        yield (*p, t_end, i == len(pending) - 1)


def _traffic_windows(
    traffic,
    bridge,
    n_days,
    active_lane,
    seed,
    target,
    want_vehicles,
    classifier=None,
    end_tail=None,
):
    """Yield (extracted, vehicles, day_offset, window_days, n_carried,
    next_arrival, is_last) day-windows. ``extracted`` (the per-window
    vehicle/axle array tuple) is always present; ``vehicles`` is the Vehicle
    list — kept only when a per-vehicle output (PT_V) needs it, else ``None``.
    Generated traffic without such an output takes the fused generate+extract
    fast path (no Python Vehicle objects). ``classifier`` (when flow statistics
    are wanted) makes the extraction also return each vehicle's class bin.
    ``n_carried`` / ``next_arrival`` / ``is_last`` are the seam context (see
    :func:`_seam_context`) so load effects and events at window seams match a
    continuous run.

    For generated traffic, ``end_tail`` (optional dict) receives the first
    arrival beyond the run end — the CPU end-of-run boundary A2 — as
    ``end_tail["arrival"]`` / ``end_tail["extracted"]``; that arrival is also
    threaded to the trailing windows as their ``next_arrival``."""
    no_lane = bridge.no_lane
    L = bridge.length
    tail = end_tail if isinstance(traffic, TrafficGenerator) else None
    if isinstance(traffic, TrafficGenerator) and not want_vehicles:
        raw = _array_windows(
            traffic, bridge, n_days, active_lane, seed, target, classifier, tail
        )
        for (
            extracted,
            day0,
            win_days,
            n_carried,
            next_arrival,
            is_last,
        ) in _seam_context(
            raw,
            first_time=lambda ex: float(ex[0][0]) if len(ex[0]) else None,
            carry_of=lambda ex, seam: _subset_extracted(
                ex, ex[0] + (L + ex[6]) / ex[1] > seam
            ),
            concat=_concat_extracted,
            count=lambda ex: len(ex[0]),
            end_arrival=(None if tail is None else (lambda: tail.get("arrival"))),
        ):
            yield extracted, None, day0, win_days, n_carried, next_arrival, is_last
    else:
        from ..lib import libbtls

        raw = _vehicle_windows(traffic, bridge, n_days, active_lane, seed, target, tail)

        def _end_arrival():
            # A2 was stashed by _vehicle_stream when the stream ran out; extract
            # its flow-count row here (same normalized form as _array_windows)
            v = None if tail is None else tail.get("vehicle")
            if v is None:
                return None
            tail["arrival"] = float(v.get_time())
            tail["extracted"] = libbtls._extract_axle_data([v], no_lane, classifier)
            return tail["arrival"]

        t_off = lambda v: v.get_time() + (L + v.get_length()) / v.get_velocity()
        for vehicles, day0, win_days, n_carried, next_arrival, is_last in _seam_context(
            raw,
            first_time=lambda vs: vs[0].get_time() if vs else None,
            carry_of=lambda vs, seam: [v for v in vs if t_off(v) > seam],
            concat=lambda ca, vs: ca + vs,
            count=len,
            end_arrival=None if tail is None else _end_arrival,
        ):
            yield libbtls._extract_axle_data(
                vehicles, no_lane, classifier
            ), vehicles, day0, win_days, n_carried, next_arrival, is_last


def run(
    bridge,
    traffic,
    no_day,
    time_step,
    min_gvw,
    active_lane,
    sim_tag,
    overlap_avoid_distance,
    output_root,
    seed,
    device="cuda",
    output_config=None,
    overwrite=False,
):
    """Run the GPU load-effect engine and return an _OutputManager.

    ``device`` is the torch device name (``"cuda"`` covers NVIDIA and AMD-ROCm).
``overwrite`` replaces an existing output directory for ``sim_tag`` instead of
raising, matching ``Simulation(overwrite=...)``.
    Honors ``output_config``: block-maxima summaries (BM_S), peaks-over-threshold
    (PT_S / PT_C / PT_V), fatigue rainflow (FR), flow statistics (SS_C / SS_S) and
    time history (TH). When ``output_config`` is None, defaults to BM_summary
    (the engine's standalone behavior).

    Generated traffic is streamed in RAM-bounded day windows (see
    :func:`_traffic_windows`), so peak host memory is independent of the
    simulated length — a 1000-year run uses the same memory as a 1-year run.
    Recorded traffic is bounded by its file (the loader holds it).

    Scope: recorded or generated traffic; discrete / built-in / surface influence
    lines; vertical / centrifugal / braking load-effect modes. The event
    partition is rebuilt from the per-vehicle on/off times, so event and
    vehicle/truck counts match the C++ engine to ~1% (uniform-grid sampling
    merges composition changes that fall within one time step); peak values,
    times and the derived statistics carry the same grid-sampling tolerance.
    """
    if not isinstance(bridge, Bridge):
        raise TypeError("engine='cuda' requires a Bridge.")
    if isinstance(traffic, TrafficGenerator) and no_day is None:
        raise ValueError("engine='cuda' with a TrafficGenerator requires no_day.")
    if not isinstance(traffic, (TrafficGenerator, TrafficLoader)):
        raise NotImplementedError(
            "engine='cuda' supports TrafficLoader or TrafficGenerator traffic."
        )
    if bridge.no_lane != traffic.no_lane:  # as the CPU path checks in simulation.py
        raise RuntimeError(
            "The number of lanes in the bridge and traffic generator are not equal."
        )

    # Same rule as the CPU path: reusing a tag would leave the previous run's
    # files in place for _OutputManager to glob back as this run's, so an
    # existing directory is an error unless the caller asked to overwrite.
    from ..simulation import _make_sim_dir

    sim_dir = output_root / str(sim_tag)
    _make_sim_dir(sim_dir, output_root, overwrite)
    length_str = f"{bridge.length:g}"

    il_specs, weights = _il_specs_from_bridge(bridge)
    n_eff = len(il_specs)
    if isinstance(traffic, TrafficLoader):
        n_days = int(no_day) if no_day is not None else int(traffic.sim_day)
    else:
        n_days = int(no_day)

    if output_config is None:
        output_config = OutputConfig()
        output_config.set_BM_output(write_summary=True)
    out = output_config._Output

    want_pot = out.POT.WRITE_POT and (
        out.POT.WRITE_POT_SUMMARY
        or out.POT.WRITE_POT_COUNTER
        or out.POT.WRITE_POT_VEHICLES
    )
    want_bm = out.BlockMax.WRITE_BM and out.BlockMax.WRITE_BM_SUMMARY
    want_fatigue = out.Fatigue.DO_FATIGUE_RAINFLOW
    want_th = out.WRITE_TIME_HISTORY
    want_stats = out.Stats.WRITE_STATS
    want_flow = out.VehicleFile.WRITE_FLOW_STATS
    if not (want_pot or want_bm or want_fatigue or want_th or want_stats or want_flow):
        want_bm = True  # default standalone behavior
    _warn_unsupported_outputs(out)

    bm_block_days = (
        out.BlockMax.BLOCK_SIZE_DAYS + out.BlockMax.BLOCK_SIZE_SECS / SECONDS_PER_DAY
    ) or 1
    bm_block_secs = bm_block_days * SECONDS_PER_DAY

    total_blocks = max(0, int(np.ceil(n_days / bm_block_days)))
    bm_state = _new_bm_state(total_blocks, n_eff)

    thresholds = list(bridge._threshold_list) if want_pot else None
    pot_stream = None
    if want_pot:
        counter_secs = (
            out.POT.POT_COUNT_SIZE_DAYS * SECONDS_PER_DAY + out.POT.POT_COUNT_SIZE_SECS
        )
        if counter_secs <= 0:
            counter_secs = SECONDS_PER_DAY
        n_counter_blocks = max(1, int(np.ceil(n_days * SECONDS_PER_DAY / counter_secs)))
        pot_stream = _PotStream(
            sim_dir, length_str, out, n_eff, counter_secs, n_counter_blocks
        )

    rainflows = None
    if want_fatigue:
        from ..lib import libbtls

        rainflows = [
            libbtls._Rainflow(
                int(out.Fatigue.RAINFLOW_DECIMAL), float(out.Fatigue.RAINFLOW_CUTOFF)
            )
            for _ in range(n_eff)
        ]

    th_file = None
    if want_th:
        th_file = open(sim_dir / f"TH_{length_str}.txt", "w")
        th_file.write(
            "Time\tNo. Trucks\t"
            + "\t".join(f"Effect {e + 1}" for e in range(n_eff))
            + "\n"
        )

    stats = None
    if want_stats:
        interval_size = float(out.Stats.WRITE_SS_INTERVAL_SIZE)
        want_intervals = out.Stats.WRITE_SS_INTERVALS
        total_intervals = (
            int(np.ceil(n_days * SECONDS_PER_DAY / interval_size))
            if want_intervals
            else 0
        )
        stats = StatsAccumulator(n_eff, want_intervals, interval_size, total_intervals)

    flow = None
    classifier = None
    if want_flow:
        from ..lib import libbtls

        ctype = traffic.vehicle_classifier  # 0 = axle, 1 = pattern
        classifier = (
            libbtls._VehClassAxle() if ctype == 0 else libbtls._VehClassPattern()
        )
        # hour 1 starts at t=0 for both traffic kinds: the CPU engine constructs
        # its _VehicleBuffer with start_time=0.0 (simulation.py), so recorded
        # traffic starting later gets leading zero rows, not a shifted grid
        flow = FlowStatsAccumulator(
            bridge.no_lane,
            ctype,
            traffic._no_lane_dir_1,
            n_days * 24,
        )

    target = _window_target_vehicles(n_eff, device, want_pot)
    file_format = out.VehicleFile.FILE_FORMAT
    # PT_V serialises the member Vehicle objects, so it needs the vehicle list;
    # otherwise generated traffic takes the fused generate+extract fast path.
    want_vehicles = bool(want_pot and out.POT.WRITE_POT_VEHICLES)

    # generated traffic: receives the first arrival beyond end_time (A2), the
    # CPU loop's end-of-run boundary — its arrival ends the last owned event
    # and it is counted in the flow statistics (but never simulated)
    end_tail = {} if isinstance(traffic, TrafficGenerator) else None

    show_progress = n_days > 730  # multi-year streamed runs: report window progress
    t_start = time.perf_counter()
    for (
        extracted,
        vehicles,
        day0,
        win_days,
        n_carried,
        next_arrival,
        is_last,
    ) in _traffic_windows(
        traffic,
        bridge,
        n_days,
        active_lane,
        seed,
        target,
        want_vehicles,
        classifier,
        end_tail,
    ):
        time_offset = day0 * SECONDS_PER_DAY
        # keep every window's sample grid on the global k*ts lattice: shift the
        # window-local grid by the phase that lands its first sample on a global
        # lattice point. 0 when time_offset is a multiple of ts (the default
        # ts | 86400); otherwise a memory-driven window split would phase-shift
        # later windows' grids and make results depend on the split.
        grid_phase = (-time_offset) % time_step
        if grid_phase < 1e-6 or grid_phase > time_step - 1e-6:
            grid_phase = 0.0
        if want_flow:  # raw per-vehicle counts (time, global lane, is-car, class
            # bin), excluding the carried seam-context rows the previous window
            # already counted
            flow.update(
                extracted[0][n_carried:],
                extracted[4][n_carried:],
                extracted[12][n_carried:],
                extracted[13][n_carried:],
            )
        has_next = np.isfinite(next_arrival)
        win_secs = win_days * SECONDS_PER_DAY
        # sample/event ownership ends at the seam for interior windows; for the
        # last generated window it ends at A2 (the first beyond-end arrival,
        # threaded in as next_arrival) — the CPU records through A2; the loader
        # last window ends at its last read arrival (the CPU read-and-sim loop
        # breaks on the end-of-stream vehicle before simulating past it)
        if not is_last:
            own_end = win_secs
        elif isinstance(traffic, TrafficLoader):
            own_end = (
                float(extracted[0].max()) - time_offset if len(extracted[0]) else None
            )
        elif has_next:
            own_end = next_arrival - time_offset
        else:
            own_end = None
        # BM / POT / stats are all reductions over the C++ event partition,
        # rebuilt here per window (rainflow / TH ride along on the same pass)
        pot = compute_pot(
            extracted,
            il_specs=il_specs,
            weights=weights,
            bridge_length=bridge.length,
            time_step=time_step,
            min_gvw=min_gvw,
            device=device,
            time_offset=time_offset,
            rainflows=rainflows,
            th_file=th_file,
            next_arrival=next_arrival - time_offset if has_next else None,
            own_samples=(
                None
                if own_end is None
                else int(np.ceil((own_end - grid_phase) / time_step - 1e-9))
            ),
            grid_phase=grid_phase,
        )
        if pot is None:  # empty window (no vehicle above min_gvw)
            continue

        # this window owns the events STARTING in it: carried seam-context
        # vehicles produce (negative-start) events owned by the previous
        # window, and post-seam events reappear in the next window with the
        # full vehicle set.
        starts = pot["B"][:-1]
        if not is_last:
            owned = (starts >= 0.0) & (starts < win_secs)
        elif isinstance(traffic, TrafficLoader):
            # loader end-of-run: the CPU read-and-sim loop advances on EVERY
            # vehicle read (regardless of GVW) and breaks on the end-of-stream
            # vehicle *before* processing the tail (simulation.py:687-690), so
            # only events at/after the LAST READ arrival are lost. Match it:
            # own events starting before the full stream's last arrival (not
            # the min_gvw-filtered one — trailing light vehicles still advance
            # the CPU loop past earlier trucks' crossings). ``own_end`` is that
            # last arrival, so the rainflow / TH streams are cut at the same
            # point.
            owned = (starts >= 0.0) & (starts < own_end)
        elif has_next:
            # generated end-of-run: the CPU's final `while current_time <=
            # end_time` iteration pulls the first arrival strictly after
            # end_time (A2) and its update() records every composition event
            # starting BEFORE A2 — including events past end_time
            # (simulation.py:691-708, Bridge.cpp::Update). A2 arrives here as
            # this last window's next_arrival (see _seam_context end_arrival),
            # so it is already a partition boundary ending the straddling event.
            owned = (starts >= 0.0) & (starts < next_arrival - time_offset)
        else:
            # generated without a beyond-end arrival (no active lane): keep the
            # cut at end_time (the strict-`>` block rollover includes it)
            owned = (starts >= 0.0) & (starts <= win_secs)
        # >=1 vehicle and >=1 grid sample (sub-time-step composition windows
        # merge into their neighbours — the documented grid tolerance)
        ev_mask = owned & (pot["win_count"] >= 1) & (pot["peak_index"][0] >= 0)

        if want_bm:
            _accumulate_bm(
                bm_state, pot, ev_mask, bm_block_secs, time_offset, total_blocks
            )
        if want_pot:
            pot_stream.update(
                pot,
                thresholds,
                ev_mask,
                time_step,
                time_offset,
                vehicles,
                file_format,
                bridge.length,
                grid_phase,
            )
        if want_stats:
            n_win = len(pot["B"]) - 1
            wtrk = potmod.truck_occupancy(
                pot["k_start"], pot["k_end"], n_win, ~pot["veh"]["is_car"]
            )
            stats.update(
                pot["peak_value"],
                pot["win_count"],
                wtrk,
                pot["B"],
                time_offset,
                ev_mask,
            )

        if show_progress:
            done = day0 + win_days
            el = time.perf_counter() - t_start
            eta = el / done * (n_days - done)
            print(
                f"  GPU streaming: day {done}/{n_days} ({100 * done / n_days:.0f}%), "
                f"elapsed {el:.0f}s, ETA ~{eta:.0f}s",
                file=sys.stderr,
                flush=True,
            )

    if want_bm:
        _write_bm_summary(sim_dir, length_str, bm_state, n_eff)
    if want_pot:
        pot_stream.close()
    if want_fatigue:
        _write_fatigue(
            sim_dir,
            length_str,
            rainflows,
            int(out.Fatigue.RAINFLOW_DECIMAL),
            float(out.Fatigue.RAINFLOW_CUTOFF),
            bool(out.Fatigue.WRITE_RAINFLOW_RESIDUALS),
        )
    if want_stats:
        if out.Stats.WRITE_SS_CUMULATIVE:
            stats.write_cumulative(sim_dir / f"SS_C_{length_str}.txt")
        if out.Stats.WRITE_SS_INTERVALS:
            stats.write_intervals(sim_dir, length_str)
    if want_flow:
        if end_tail and "extracted" in end_tail:
            # the CPU flow buffer also counts the beyond-end vehicle A2 (its
            # AddVehicle happens on the loop's final iteration, before the
            # break), opening the hour row containing it
            # (CVehicleBuffer::updateFlowData's silent-hour fill)
            ex2 = end_tail["extracted"]
            flow.extend_hours(int(ex2[0][0] // 3600.0) + 1)
            flow.update(ex2[0], ex2[4], ex2[12], ex2[13])
        flow.write(sim_dir)
    if th_file is not None:
        th_file.close()

    return _OutputManager(output_root, sim_tag, output_config)
