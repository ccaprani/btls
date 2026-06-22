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
from .engine import compute_load_effect_maxima, compute_pot
from .stats import StatsAccumulator

SECONDS_PER_DAY = 86400.0


def _il_to_spec(il):
    """Convert one InfluenceLine / InfluenceSurface to an il_spec dict. The
    per-axle force mode (vertical / centrifugal / braking) and braking_factor are
    carried along so the device applies the same per-axle force coefficient the
    C++ engine does (cpp/src/InfluenceLine.cpp:getAxleLoadEffect)."""
    mode = {"mode": getattr(il, "_load_effect_mode", "vertical"),
            "braking_factor": float(getattr(il, "_braking_factor", 0.0))}
    if isinstance(il, InfluenceSurface):
        M = np.asarray(il._data_dict["IS_matrix"], dtype=float)
        lp = il._data_dict["lane_position"]
        return {
            "kind": "surface",
            "X": M[1:, 0], "Y": M[0, 1:], "ISords": M[1:, 1:],
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
            il_specs.append(_il_to_spec(ils[0]))
            weights.append(wts[0])
        else:
            lane_specs = [_il_to_spec(il) for il in ils]
            if any(s["kind"] == "surface" for s in lane_specs):
                raise NotImplementedError(
                    "engine='cuda' (experimental) does not support per-lane influence "
                    "surfaces (a single surface already spans all lanes)."
                )
            il_specs.append({"kind": "per_lane", "lane_specs": lane_specs, "lane_weights": wts,
                             "mode": getattr(ils[0], "_load_effect_mode", "vertical"),
                             "braking_factor": float(getattr(ils[0], "_braking_factor", 0.0))})
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
        lanes = lane_list if active_lane is None else [lane_list[i - 1] for i in active_lane]
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


def _write_bm_summary(sim_dir, length_str, block_maxima, n_eff):
    """Write BM_summary files in the C++ engine's format (single value column,
    read back by read_BM_S as the 1-Truck-Event column)."""
    for e in range(n_eff):
        with open(sim_dir / f"BM_S_{length_str}_Eff_{e + 1}.txt", "w") as fh:
            for b in range(block_maxima.shape[0]):
                fh.write(f"{b + 1}\t{block_maxima[b, e]:.1f}\t\t\n")


def _write_fatigue(sim_dir, length_str, rainflows, decimal):
    """Close each rainflow counter and write FR_*.txt (the cycle-amplitude
    histogram) in the C++ CFatigueManager format. The streamed run carried the
    residual across all windows, so a single final close (ASTM end-of-data rule)
    matches a sequential run."""
    for e, rf in enumerate(rainflows):
        rf.calcCycles(True)  # close the residual as half-cycles at end of data
        hist = rf.getRainflowOutput()  # dict: rounded range -> cycle count
        with open(sim_dir / f"FR_{length_str}_{e + 1}.txt", "w") as fh:
            fh.write(f"{'Amplitude':>15}{'No. Cycles':>15}\n")
            for rng in sorted(hist):
                fh.write(f"{rng:>15.{decimal}f}{hist[rng]:>10.1f}\n")


def _pot_events_per_effect(pot, thresholds):
    """Per effect, the window indices of above-threshold events, in time order.
    An event needs >=1 vehicle, at least one grid sample, and a signed peak
    above the threshold (matching CPOTManager::Update's ``getValue() > thr``)."""
    pv, pix, win_count = pot["peak_value"], pot["peak_index"], pot["win_count"]
    out = []
    for e in range(pv.shape[0]):
        mask = (win_count >= 1) & (pix[e] >= 0) & (pv[e] > thresholds[e])
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


def _accumulate_pot(accum, pot, thresholds, time_step, time_offset, vehicles,
                    out, file_format, counter_secs, n_counter_blocks):
    """Fold one window's above-threshold events into the run-wide accumulator,
    converting window-local times to absolute. PT_V member-vehicle lines are
    serialized now, while the window's vehicles are still in memory."""
    events = _pot_events_per_effect(pot, thresholds)
    pv, pix, win_count, B = pot["peak_value"], pot["peak_index"], pot["win_count"], pot["B"]
    n_eff = pv.shape[0]
    write_v = out.POT.WRITE_POT_VEHICLES
    if write_v:
        veh, L = pot["veh"], pot["_bridge_length"]
        kept_idx, t_on = veh["kept_idx"], veh["t_on"]
        indptr, members = potmod.window_members_csr(pot["k_start"], pot["k_end"], len(B) - 1)
    for e in range(n_eff):
        for w in events[e]:
            rec = {"time": float(pix[e, w] * time_step + time_offset),
                   "value": float(pv[e, w]), "no_trucks": int(win_count[w])}
            if write_v:
                mem = members[indptr[w]:indptr[w + 1]]
                mem = mem[np.argsort(t_on[mem], kind="stable")]  # by arrival time
                # dist uses window-LOCAL time (veh windows are local); the printed
                # time is absolute (local + offset)
                rec["all_val"] = [float(pv[k, w]) for k in range(n_eff)]
                rec["all_time"] = [float(pix[k, w] * time_step + time_offset) if pix[k, w] >= 0 else 0.0
                                   for k in range(n_eff)]
                rec["all_dist"] = [_lead_dist(mem, float(pix[k, w] * time_step) if pix[k, w] >= 0 else 0.0, veh, L)
                                   for k in range(n_eff)]
                rec["veh_lines"] = [vehicles[int(kept_idx[m])].write(file_format) for m in mem]
            accum["events"][e].append(rec)
        if accum["counts"] is not None:
            for w in events[e]:
                cb = int((B[w] + time_offset) // counter_secs)
                if 0 <= cb < n_counter_blocks:
                    accum["counts"][cb, e] += 1


def _write_pot_files(sim_dir, length_str, accum, out, n_eff):
    """Write the accumulated PT_S / PT_C / PT_V in the C++ POTManager format."""
    events = accum["events"]
    if out.POT.WRITE_POT_SUMMARY:
        for e in range(n_eff):
            with open(sim_dir / f"PT_S_{length_str}_Eff_{e + 1}.txt", "w") as fh:
                for i, rec in enumerate(events[e]):
                    fh.write(f"{i + 1:>6}{rec['time']:>15.1f}{rec['no_trucks']:>4}{rec['value']:>10.1f}\n")

    if out.POT.WRITE_POT_COUNTER and accum["counts"] is not None:
        counts = accum["counts"]
        with open(sim_dir / f"PT_C_{length_str}.txt", "w") as fh:
            fh.write("Block\t" + "".join(f"LE {e + 1}\t" for e in range(n_eff)) + "\n")
            for b in range(counts.shape[0]):
                fh.write(f"{b + 1}\t" + "".join(f"{counts[b, e]}\t" for e in range(n_eff)) + "\n")

    if out.POT.WRITE_POT_VEHICLES:
        for e in range(n_eff):
            with open(sim_dir / f"PT_V_{length_str}_{e + 1}.txt", "w") as fh:
                for i, rec in enumerate(events[e]):
                    fh.write(f"{i + 1}\n")
                    for k in range(n_eff):  # C++ writes a block for ALL effects
                        fh.write(f"{k + 1:>2}{rec['all_val'][k]:>10.1f}{rec['all_time'][k]:>15.1f}"
                                 f"{rec['all_dist'][k]:>10.2f}{rec['no_trucks']:>4}\n")
                        for line in rec["veh_lines"]:
                            fh.write(line + "\n")


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
    ("WRITE_EACH_EVENT",                 "every-event output (set_event_output write_each_event)"),
    ("VehicleFile.WRITE_VEHICLE_FILE",   "vehicle file (set_vehicle_file_output)"),
    ("VehicleFile.WRITE_FLOW_STATS",     "flow statistics (set_stats_output write_flow_stats)"),
    ("BlockMax.WRITE_BM_VEHICLES",       "block-max vehicles (set_BM_output write_vehicle)"),
    ("BlockMax.WRITE_BM_MIXED",          "block-max mixed (set_BM_output write_mixed)"),
    ("WRITE_FATIGUE_EVENT",              "fatigue events (set_fatigue_output write_fatigue_event)"),
    ("Fatigue.WRITE_RAINFLOW_RESIDUALS", "rainflow residuals (set_fatigue_output write_residuals)"),
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
        print("Warning: engine='cuda' does not produce these requested outputs, "
              "they will be skipped (use engine='cpu' for them):\n  - "
              + "\n  - ".join(missing), file=sys.stderr, flush=True)


def _window_target_vehicles(n_eff, device, want_pot):
    """Per-window vehicle budget = the smaller of a host-RAM and a VRAM budget,
    each a fraction of *free* memory (so it adapts to the machine and to whatever
    else is running). Biased toward safety: an over-large window OOMs (fatal),
    an over-small one only costs a little speed, so the fraction is well under 1
    and the per-vehicle estimates are generous."""
    FRACTION = 0.5
    HOST_BYTES = 2500                                  # Python vehicle + numpy axle arrays / veh
    VRAM_BYTES = 400 + (60 * n_eff if want_pot else 0)  # device axle arrays + POT accumulators / veh

    targets = [int(FRACTION * available_host_memory()) // HOST_BYTES]
    free_vram = _free_device_bytes(device)
    if free_vram is not None:
        targets.append(int(FRACTION * free_vram) // VRAM_BYTES)
    return max(100_000, min(targets))


def _vehicle_stream(traffic, bridge, active_lane, seed, end_time):
    """Yield vehicles in arrival-time order, lazily, up to ``end_time`` — for both
    recorded (merge the pre-loaded per-lane lists) and generated (pull the
    generator one vehicle at a time) traffic. Generated traffic is never fully
    materialized: the caller holds only the window being processed."""
    if isinstance(traffic, TrafficLoader):
        lanes = traffic._lanes_vehicles
        if active_lane is not None:
            lanes = [lanes[i - 1] for i in active_lane]
        for v in heapq.merge(*lanes, key=lambda x: x.get_time()):
            if v.get_time() >= end_time:
                break
            yield v
    elif isinstance(traffic, TrafficGenerator):
        from ..lib import libbtls
        if seed is not None:
            libbtls.seed(seed)
        lane_list = traffic._get_traffic_generator(bridge.length)
        lanes = lane_list if active_lane is None else [lane_list[i - 1] for i in active_lane]
        # generate in day-sized bulk passes: the C++ pulls the globally-earliest
        # lane each step, so the stream (and RNG draw order) is identical to a
        # per-vehicle Python loop, but without the per-vehicle Python<->C++ cost.
        chunk = 0.0
        while chunk < end_time:
            chunk_end = min(chunk + SECONDS_PER_DAY, end_time)
            for v in libbtls._generate_traffic_stream(lanes, chunk_end):
                yield v
            chunk = chunk_end
    else:
        raise NotImplementedError(
            "engine='cuda' supports TrafficLoader or TrafficGenerator traffic."
        )


def _traffic_windows(traffic, bridge, n_days, active_lane, seed, target, align_days):
    """Yield (vehicles, day_offset, window_days): contiguous, block-aligned day
    windows each holding about ``target`` vehicles (a vehicle goes to the window
    containing its arrival day). Bounds host memory to one window for generated
    traffic, independent of ``n_days``."""
    end_time = n_days * SECONDS_PER_DAY
    stream = _vehicle_stream(traffic, bridge, active_lane, seed, end_time)
    day0 = 0
    carry = None
    exhausted = False
    while day0 < n_days and not exhausted:
        win_end_day = min(day0 + align_days, n_days)
        vehicles = []
        if carry is not None:
            vehicles.append(carry)
            carry = None
        for v in stream:
            vday = int(v.get_time() // SECONDS_PER_DAY)
            if vday < win_end_day:
                vehicles.append(v)
                continue
            # vehicle is past the current window end: grow the window (block-aligned)
            # while still under the vehicle budget, else carry it to the next window
            if len(vehicles) < target and win_end_day < n_days:
                while win_end_day < n_days and vday >= win_end_day:
                    win_end_day = min(win_end_day + align_days, n_days)
                if vday < win_end_day:
                    vehicles.append(v)
                    continue
            carry = v
            break
        else:
            exhausted = True
        yield vehicles, day0, win_end_day - day0
        day0 = win_end_day


def run(bridge, traffic, no_day, time_step, min_gvw, active_lane,
        sim_tag, overlap_avoid_distance, output_root, seed, device="cuda",
        output_config=None):
    """Run the GPU load-effect engine and return an _OutputManager.

    ``device`` is the torch device name (``"cuda"`` covers NVIDIA and AMD-ROCm).
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
            "engine='cuda' supports TrafficLoader or TrafficGenerator traffic.")

    os.makedirs(output_root / str(sim_tag), exist_ok=True)
    sim_dir = output_root / str(sim_tag)
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
        out.POT.WRITE_POT_SUMMARY or out.POT.WRITE_POT_COUNTER or out.POT.WRITE_POT_VEHICLES
    )
    want_bm = out.BlockMax.WRITE_BM and out.BlockMax.WRITE_BM_SUMMARY
    want_fatigue = out.Fatigue.DO_FATIGUE_RAINFLOW
    want_th = out.WRITE_TIME_HISTORY
    want_stats = out.Stats.WRITE_STATS
    if not want_pot and not want_bm and not want_fatigue and not want_th and not want_stats:
        want_bm = True  # default standalone behavior
    _warn_unsupported_outputs(out)

    bm_block_days = (out.BlockMax.BLOCK_SIZE_DAYS
                     + out.BlockMax.BLOCK_SIZE_SECS / SECONDS_PER_DAY) or 1
    align_days = max(1, int(round(bm_block_days)))  # windows never split a block

    total_blocks = max(0, int(np.ceil(n_days / bm_block_days)))
    global_bm = np.zeros((total_blocks, n_eff))

    thresholds = list(bridge._threshold_list) if want_pot else None
    counter_secs = 0.0
    n_counter_blocks = 0
    accum = None
    if want_pot:
        counter_secs = out.POT.POT_COUNT_SIZE_DAYS * SECONDS_PER_DAY + out.POT.POT_COUNT_SIZE_SECS
        if counter_secs <= 0:
            counter_secs = SECONDS_PER_DAY
        n_counter_blocks = max(1, int(np.ceil(n_days * SECONDS_PER_DAY / counter_secs)))
        accum = {"events": [[] for _ in range(n_eff)],
                 "counts": (np.zeros((n_counter_blocks, n_eff), dtype=np.int64)
                            if out.POT.WRITE_POT_COUNTER else None)}

    rainflows = None
    if want_fatigue:
        from ..lib import libbtls
        rainflows = [libbtls._Rainflow(int(out.Fatigue.RAINFLOW_DECIMAL),
                                       float(out.Fatigue.RAINFLOW_CUTOFF))
                     for _ in range(n_eff)]

    th_file = None
    if want_th:
        th_file = open(sim_dir / f"TH_{length_str}.txt", "w")
        th_file.write("Time\tNo. Trucks\t" + "\t".join(f"Effect {e + 1}" for e in range(n_eff)) + "\n")

    stats = None
    if want_stats:
        interval_size = float(out.Stats.WRITE_SS_INTERVAL_SIZE)
        want_intervals = out.Stats.WRITE_SS_INTERVALS
        total_intervals = (int(np.ceil(n_days * SECONDS_PER_DAY / interval_size))
                           if want_intervals else 0)
        stats = StatsAccumulator(n_eff, want_intervals, interval_size, total_intervals)

    target = _window_target_vehicles(n_eff, device, want_pot)
    file_format = out.VehicleFile.FILE_FORMAT

    show_progress = n_days > 730  # multi-year streamed runs: report window progress
    t_start = time.perf_counter()
    for vehicles, day0, win_days in _traffic_windows(
            traffic, bridge, n_days, active_lane, seed, target, align_days):
        time_offset = day0 * SECONDS_PER_DAY
        if want_pot or want_stats:  # both need the per-event partition + peak_value
            pot = compute_pot(
                vehicles=vehicles, il_specs=il_specs, weights=weights,
                bridge_length=bridge.length, time_step=time_step,
                no_lane=bridge.no_lane, min_gvw=min_gvw,
                block_size_days=bm_block_days, device=device, time_offset=time_offset,
                rainflows=rainflows, th_file=th_file,
            )
            if pot is None:  # empty window (no vehicle above min_gvw)
                continue
            if want_pot:
                pot["_bridge_length"] = bridge.length
                _accumulate_pot(accum, pot, thresholds, time_step, time_offset,
                                vehicles, out, file_format, counter_secs, n_counter_blocks)
            if want_stats:
                n_win = len(pot["B"]) - 1
                wtrk = potmod.truck_occupancy(
                    pot["k_start"], pot["k_end"], n_win, ~pot["veh"]["is_car"])
                stats.update(pot["peak_value"], pot["win_count"], wtrk,
                             pot["B"], time_offset)
            win_bm = pot["block_maxima"]
        else:
            result = compute_load_effect_maxima(
                vehicles=vehicles, il_specs=il_specs, weights=weights,
                bridge_length=bridge.length, time_step=time_step, n_days=win_days,
                no_lane=bridge.no_lane, min_gvw=min_gvw,
                block_size_days=bm_block_days, device=device, time_offset=time_offset,
                rainflows=rainflows, th_file=th_file,
            )
            win_bm = result["block_maxima"]

        if want_bm and len(win_bm):  # place this window's day-blocks (drop spillover)
            start = int(round(day0 / bm_block_days))
            expected = max(1, int(round(win_days / bm_block_days)))
            wb = win_bm[:expected]
            global_bm[start:start + len(wb)] = wb

        if show_progress:
            done = day0 + win_days
            el = time.perf_counter() - t_start
            eta = el / done * (n_days - done)
            print(f"  GPU streaming: day {done}/{n_days} ({100 * done / n_days:.0f}%), "
                  f"elapsed {el:.0f}s, ETA ~{eta:.0f}s", file=sys.stderr, flush=True)

    if want_bm:
        _write_bm_summary(sim_dir, length_str, global_bm, n_eff)
    if want_pot:
        _write_pot_files(sim_dir, length_str, accum, out, n_eff)
    if want_fatigue:
        _write_fatigue(sim_dir, length_str, rainflows, int(out.Fatigue.RAINFLOW_DECIMAL))
    if want_stats:
        if out.Stats.WRITE_SS_CUMULATIVE:
            stats.write_cumulative(sim_dir / f"SS_C_{length_str}.txt")
        if out.Stats.WRITE_SS_INTERVALS:
            stats.write_intervals(sim_dir, length_str)
    if th_file is not None:
        th_file.close()

    return _OutputManager(output_root, sim_tag, output_config)
