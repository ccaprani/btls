"""
Core GPU (torch) load-effect computation via per-vehicle superposition.

Reconstructs each axle's on-bridge trajectory from the recorded vehicles
(kinematics validated against the C++ engine, dev_log/refactor_C0), expands
them into (time-sample, position, weight) pairs, and computes the per-effect
load-effect time history E[t, e] with a per-day-tiled, per-effect scatter-add
so neither the full E(t) nor a [pairs x effects] intermediate is materialized.
Returns per-block block maxima and global maxima per load effect.

Each load effect is described by an ``il_spec`` dict:
  {"kind": "discrete", "pos": ndarray, "ord": ndarray}
  {"kind": "builtin",  "id": int, "length": float}
  {"kind": "surface",  "X": ndarray, "Y": ndarray, "ISords": ndarray[nx, ny],
                       "lane_centre": ndarray[nlane], "lane_width": ndarray[nlane]}
"""

import numpy as np

from .influence import builtin_ordinate, resample_il, uniform_surface_grid

SECONDS_PER_DAY = 86400.0
GRAVITY = 9.80665  # m/s^2, matches GRAVITY_MS2_FOR_LE in cpp/src/InfluenceLine.cpp


class GpuEngineError(RuntimeError):
    """Raised when the GPU engine cannot run (missing torch/CUDA or unsupported config)."""


def _lazy_torch():
    try:
        import torch
    except ImportError as exc:  # pragma: no cover - depends on environment
        raise GpuEngineError(
            "engine='cuda' requires PyTorch. Install a CUDA build of torch, "
            "or use engine='cpu'."
        ) from exc
    return torch


def is_available(device="cuda") -> bool:
    """True if torch is importable and the given accelerator is present
    (``device`` is a torch device name: "cuda" covers NVIDIA + AMD-ROCm)."""
    try:
        import torch

        return _device_present(torch, device)
    except Exception:
        return False


def _device_present(torch, device):
    if device == "cpu":
        return True
    if device == "cuda":
        return bool(torch.cuda.is_available())
    if device == "mps":
        return hasattr(torch.backends, "mps") and torch.backends.mps.is_available()
    if device == "xpu":
        return hasattr(torch, "xpu") and torch.xpu.is_available()
    return False


# The torch backends differ in operator coverage. The engine's non-trivial ops
# are searchsorted (IL interpolation / event-window mapping), index_add_
# (per-axle scatter-add), and scatter_reduce_ amax (POT). CUDA has them all;
# the MPS (Apple) backend historically lacks searchsorted / scatter_reduce amax
# and needs PYTORCH_ENABLE_MPS_FALLBACK=1; XPU (Intel) coverage is broad but
# unverified here. Rather than hard-code a (version-dependent) support matrix,
# probe the actual device once so a gap surfaces as a clear error instead of a
# cryptic mid-compute crash. Cached per device type.
_OP_SUPPORT_CACHE = {}


def _missing_ops(torch, dev):
    """Return the engine-critical ops that fail on ``dev`` (empty list = all OK).
    Probed once per device type with tiny tensors."""
    if dev.type in _OP_SUPPORT_CACHE:
        return _OP_SUPPORT_CACHE[dev.type]
    dt = torch.float32
    idx = torch.zeros(2, dtype=torch.long, device=dev)
    one = torch.ones(2, dtype=dt, device=dev)
    probes = {
        "searchsorted": lambda: torch.searchsorted(
            torch.tensor([0.0, 1.0, 2.0], dtype=dt, device=dev),
            torch.tensor([0.5, 1.5], dtype=dt, device=dev),
            right=True,
        ),
        "index_add": lambda: torch.zeros(2, dtype=dt, device=dev).index_add_(
            0, idx, one
        ),
        "scatter_reduce(amax)": lambda: torch.zeros(
            2, dtype=dt, device=dev
        ).scatter_reduce_(0, idx, one, reduce="amax", include_self=True),
    }
    missing = []
    for name, fn in probes.items():
        try:
            fn()
        except Exception:
            missing.append(name)
    _OP_SUPPORT_CACHE[dev.type] = missing
    return missing


def _require_ops(torch, dev, device):
    """Raise a clear GpuEngineError if ``dev`` is missing an engine-critical op."""
    if dev.type == "cuda":
        return  # known-good; skip the probe on the hot path
    missing = _missing_ops(torch, dev)
    if missing:
        hint = ""
        if dev.type == "mps":
            hint = (
                " Set PYTORCH_ENABLE_MPS_FALLBACK=1 before importing torch to "
                "fall back to CPU for these ops (slower, but functional)."
            )
        raise GpuEngineError(
            f"engine='{device}': this PyTorch build lacks required ops on the "
            f"'{dev.type}' backend: {', '.join(missing)}.{hint}"
        )


def _interp(torch, xp, fp, x):
    """Linear interpolation of discrete (xp, fp) at x; 0 outside [xp[0], xp[-1]]."""
    idx = torch.searchsorted(xp, x, right=True).clamp_(1, xp.shape[0] - 1)
    x0 = xp[idx - 1]
    x1 = xp[idx]
    y0 = fp[idx - 1]
    y1 = fp[idx]
    out = y0 + (x - x0) / (x1 - x0) * (y1 - y0)
    return torch.where((x < xp[0]) | (x > xp[-1]), torch.zeros_like(out), out)


def _surface_ordinate(torch, X, Y, ISords, x, y):
    """Bilinear interpolation on an influence surface, mirroring
    CInfluenceSurface::giveOrdinate (cpp/src/InfluenceSurface.cpp:86-131)."""
    nx = X.shape[0]
    ny = Y.shape[0]
    iX = torch.searchsorted(X, x, right=True).clamp_(1, nx - 1)
    iY = torch.searchsorted(Y, y, right=True).clamp_(1, ny - 1)
    dX = X[iX] - X[iX - 1]
    dY = Y[iY] - Y[iY - 1]
    xsi1 = ISords[iX - 1, iY]
    xsi2 = ISords[iX, iY]
    xsi3 = ISords[iX - 1, iY - 1]
    xsi4 = ISords[iX, iY - 1]
    xsiA = xsi1 + (x - X[iX - 1]) / dX * (xsi2 - xsi1)
    xsiB = xsi3 + (x - X[iX - 1]) / dX * (xsi4 - xsi3)
    xsi = xsiB + (y - Y[iY - 1]) / dY * (xsiA - xsiB)
    oob = (x < X[0]) | (x > X[-1]) | (y < Y[0]) | (y > Y[-1])
    return torch.where(oob, torch.zeros_like(xsi), xsi)


def _reconstruct_axles(
    extracted, bridge_length, min_gvw, need_transverse, time_offset=0.0
):
    """Pre-extracted per-vehicle/per-axle arrays -> per-axle (datum, sign, speed,
    weight) and, if a surface effect is present, per-axle (lane, trans, track) for
    the transverse model.

    ``extracted`` is the flat-array tuple from ``libbtls._extract_axle_data``
    (loader) or ``libbtls._generate_and_extract`` (generator) — a vehicle is
    unpacked once, in C++, regardless of source; all per-axle kinematics is then
    vectorized in numpy. ``time_offset`` shifts all vehicle times to a window-local
    origin so a streamed run processes one window at a time on a small
    (window-length) sample grid instead of an absolute one."""
    (
        vtime,
        vspeed,
        vdir,
        vgvw,
        vlane,
        vtrans,
        vlen,
        vacc,
        vcount,
        aw,
        asp,
        at,
        viscar,
        _viscls,
    ) = extracted  # _viscls (flow-stats class bin) used by the runner, not here

    keep = vgvw > min_gvw
    if not keep.any():
        return [np.array([]) for _ in range(4)] + [None, None, None, None, None]

    axle_keep = np.repeat(keep, vcount)  # axle-level mask (full vcount)
    counts = vcount[keep]  # kept vehicles' axle counts
    a_speed = np.repeat(vspeed[keep], counts)
    a_accel = np.repeat(vacc[keep], counts)  # per-axle longitudinal acceleration
    a_sign = np.where(np.repeat(vdir[keep] == 1, counts), 1.0, -1.0)
    a_time = np.repeat(vtime[keep] - time_offset, counts)
    a_dir2 = np.repeat(vdir[keep] == 2, counts)

    # segmented exclusive prefix sum of spacings within each vehicle = cumulative
    # spacing of each axle from the front axle (matches the C++ per-axle datum)
    asp = asp[axle_keep]
    excl = np.cumsum(asp) - asp
    veh_start = np.cumsum(counts) - counts
    cumsp = excl - np.repeat(excl[veh_start], counts)

    datum = a_time + cumsp / a_speed + np.where(a_dir2, bridge_length / a_speed, 0.0)
    weight = aw[axle_keep]

    # per (kept) vehicle on-bridge window, matching the C++ engine exactly:
    #   t_on  = get_time();  t_off = t_on + (L + length) / speed
    # (cpp/src/Vehicle.cpp:814-830). Used for POT event reconstruction.
    v_speed = vspeed[keep]
    v_on = vtime[keep] - time_offset
    v_off = v_on + (bridge_length + vlen[keep]) / v_speed
    veh = {
        "t_on": v_on,
        "t_off": v_off,
        "speed": v_speed,
        "sign": np.where(vdir[keep] == 1, 1.0, -1.0),
        "is_car": viscar[keep].astype(bool),  # per kept vehicle (for stats no.-trucks)
        "kept_idx": np.nonzero(keep)[0],
    }

    out = [datum, a_sign, a_speed, weight, a_accel]
    if need_transverse:
        out += [
            np.repeat(vlane[keep] - 1, counts),  # 0-based bridge lane
            np.repeat(vtrans[keep], counts),
            at[axle_keep],
        ]
    else:
        out += [None, None, None]
    out.append(veh)
    return out


def prepare_axles(
    extracted, il_specs, bridge_length, time_step, min_gvw, time_offset=0.0
):
    """Device-agnostic step: reconstruct per-axle trajectory arrays from the
    pre-extracted vehicle/axle arrays (vectorized kinematics). Returns
    (axles_dict, n_total, veh) or (None, 0, None); ``veh`` holds the per-vehicle
    on-bridge windows used for POT events. The per-time-sample pair expansion is
    deferred to the device (compute_from_axles) so the (much larger) pair arrays
    never materialize on the host. ``time_offset`` is subtracted from all vehicle
    times (window-local origin for streamed runs)."""
    # surfaces need (lane, trans, track); per-lane 1D effects need the lane
    need_lane = any(s["kind"] in ("surface", "per_lane") for s in il_specs)
    datum, sign, speed, weight, accel, lane, trans, track, veh = _reconstruct_axles(
        extracted, bridge_length, min_gvw, need_lane, time_offset
    )
    if len(datum) == 0:
        return None, 0, None
    n_total = int(np.ceil((datum.max() + bridge_length / speed.min()) / time_step)) + 1
    axles = {
        "datum": datum,
        "sign": sign,
        "speed": speed,
        "weight": weight,
        "accel": accel,
    }
    if lane is not None:
        axles.update(lane=lane, trans=trans, track=track)
    return axles, n_total, veh


def _occupancy(veh, n_total, time_step):
    """Per-sample on-bridge vehicle count (the TH 'No. Trucks' column): a
    +1/-1 difference array over each vehicle's [t_on, t_off) sample range."""
    nlo = np.clip(np.ceil(veh["t_on"] / time_step).astype(np.int64), 0, n_total)
    nhi = np.clip(np.ceil(veh["t_off"] / time_step).astype(np.int64), 0, n_total)
    diff = np.zeros(n_total + 1, dtype=np.int64)
    np.add.at(diff, nlo, 1)
    np.add.at(diff, nhi, -1)
    return np.cumsum(diff)[:n_total]


def compute_from_axles(
    axles,
    n_total,
    il_specs,
    weights,
    bridge_length,
    time_step,
    block_size_days=1,
    device="cuda",
    dtype="float64",
    pot_boundaries=None,
    rainflows=None,
    th=None,
):
    """Device-specific step: move per-axle arrays to the device, expand them into
    per-time-sample pairs ON the device (no host pair materialization), and
    compute per-effect block maxima.

    If ``pot_boundaries`` (the POT event-window boundary times) is given, the
    same per-block E(t) is also reduced per event window: the result gains
    ``pot_peak_value`` / ``pot_peak_index`` arrays ([n_eff, n_window]) holding,
    per window, the signed largest-|E| peak and the global sample index at which
    it occurs (-1 if the window contains no sample).

    If ``rainflows`` (a list of one ``libbtls._Rainflow`` per effect) is given,
    each block's E(t) is reduced to its turning points on the device and fed to
    the rainflow counters (ASTM E1049-85, residual carried across blocks/windows)
    for fatigue. The counters are finalized and written by the caller."""
    torch = _lazy_torch()
    if not _device_present(torch, device):
        raise GpuEngineError(
            f"engine='{device}' selected but no '{device}' device is available."
        )
    if device == "mps":
        dtype = "float32"  # Metal (MPS) has no float64
    dev = torch.device(device)
    _require_ops(torch, dev, device)  # surface backend op gaps as a clear error
    dt = torch.float64 if dtype == "float64" else torch.float32
    L, ts = bridge_length, time_step

    n_eff = len(il_specs)
    need_transverse = "lane" in axles

    datum = torch.as_tensor(axles["datum"], dtype=dt, device=dev)
    sign = torch.as_tensor(axles["sign"], dtype=dt, device=dev)
    speed = torch.as_tensor(axles["speed"], dtype=dt, device=dev)
    weight = torch.as_tensor(axles["weight"], dtype=dt, device=dev)

    # per-axle on-bridge window in sample indices (kept resident, axle-scale);
    # the per-time-sample pairs are expanded PER BLOCK below so peak memory is
    # only one block's pairs (no global pair array) — bounds VRAM for long runs.
    tlo = torch.where(sign > 0, datum, datum - L / speed)
    thi = torch.where(sign > 0, datum + L / speed, datum)
    nlo = torch.clamp(torch.ceil(tlo / ts).long(), 0, n_total - 1)
    nhi = torch.clamp(torch.floor(thi / ts).long(), 0, n_total - 1)
    if need_transverse:
        lane_ax = torch.as_tensor(axles["lane"], device=dev)
        trans_ax = torch.as_tensor(axles["trans"], dtype=dt, device=dev)
        track_ax = torch.as_tensor(axles["track"], dtype=dt, device=dev)

    wts = torch.as_tensor(weights, dtype=dt, device=dev)

    # prepare per-effect compute method. 1D ILs (discrete / built-in) are
    # resampled to a uniform grid and run through a fused Triton kernel when
    # available (one pass, ~2.4x over the torch multi-pass); surfaces and the
    # no-Triton fallback use torch.
    from .kernels import triton_available, scatter_interp_1d, scatter_interp_2d

    use_triton = triton_available() and dev.type == "cuda"
    n_grid = min(max(4096, int(L / 0.005) + 1), 262144)
    x_grid = np.linspace(0.0, L, n_grid)

    prepared = []
    for spec in il_specs:
        if spec["kind"] == "surface":
            lc = torch.as_tensor(spec["lane_centre"], dtype=dt, device=dev)
            lw = torch.as_tensor(spec["lane_width"], dtype=dt, device=dev)
            uni = (
                uniform_surface_grid(spec["X"], spec["Y"], spec["ISords"])
                if use_triton
                else None
            )
            if (
                uni is not None
            ):  # fused two-track bilinear + scatter on the uniform grid
                Zu, x0s, dxs, y0s, dys = uni
                nxs, nys = Zu.shape
                Zt = torch.as_tensor(
                    np.ascontiguousarray(Zu).ravel(), dtype=dt, device=dev
                )
                prepared.append(
                    ("surface_triton", (Zt, nxs, nys, x0s, dxs, y0s, dys, lc, lw))
                )
            else:  # non-uniform grid (or no Triton): exact torch searchsorted path
                prepared.append(
                    (
                        "surface",
                        (
                            torch.as_tensor(spec["X"], dtype=dt, device=dev),
                            torch.as_tensor(spec["Y"], dtype=dt, device=dev),
                            torch.as_tensor(spec["ISords"], dtype=dt, device=dev),
                            lc,
                            lw,
                        ),
                    )
                )
        elif spec["kind"] == "per_lane":
            lane_prepared = []
            for ls in spec["lane_specs"]:
                if ls["kind"] == "discrete":
                    lane_prepared.append(
                        (
                            "torch_discrete",
                            (
                                torch.as_tensor(ls["pos"], dtype=dt, device=dev),
                                torch.as_tensor(ls["ord"], dtype=dt, device=dev),
                            ),
                        )
                    )
                else:  # built-in
                    lane_prepared.append(("torch_builtin", ls))
            prepared.append(
                (
                    "per_lane",
                    (
                        lane_prepared,
                        torch.as_tensor(spec["lane_weights"], dtype=dt, device=dev),
                    ),
                )
            )
        elif use_triton:
            g = resample_il(spec, x_grid)
            prepared.append(
                (
                    "triton1d",
                    (torch.as_tensor(g, dtype=dt, device=dev), L / (n_grid - 1)),
                )
            )
        elif spec["kind"] == "discrete":
            prepared.append(
                (
                    "torch_discrete",
                    (
                        torch.as_tensor(spec["pos"], dtype=dt, device=dev),
                        torch.as_tensor(spec["ord"], dtype=dt, device=dev),
                    ),
                )
            )
        else:
            prepared.append(("torch_builtin", spec))

    # per-effect force mode: vertical (default), centrifugal (x v^2/g), braking
    # (x |a|/g, or braking_factor when a==0). The factor scales each axle's weight
    # before the influence-ordinate multiply (cpp/src/InfluenceLine.cpp).
    modes = [
        (s.get("mode", "vertical"), float(s.get("braking_factor", 0.0)))
        for s in il_specs
    ]
    any_mode = any(mo != "vertical" for mo, _ in modes)
    accel = torch.as_tensor(axles["accel"], dtype=dt, device=dev) if any_mode else None

    block = int(round(block_size_days * SECONDS_PER_DAY / time_step))
    n_blocks = (n_total + block - 1) // block
    bm = torch.full((n_blocks, n_eff), -float("inf"), dtype=dt, device=dev)

    # POT: per-event-window signed peak (largest |E|) and its global sample index
    do_pot = pot_boundaries is not None
    if do_pot:
        Bt = torch.as_tensor(pot_boundaries, dtype=dt, device=dev)
        n_win = Bt.shape[0] - 1
        peakmag = torch.full((n_eff, n_win), -1.0, dtype=dt, device=dev)
        peakval = torch.zeros((n_eff, n_win), dtype=dt, device=dev)
        peakidx = torch.full((n_eff, n_win), -1, dtype=torch.long, device=dev)

    for b in range(n_blocks):
        a, z = b * block, min((b + 1) * block, n_total)
        # expand only this block's pairs: clamp each axle's window to [a, z-1]
        blo = torch.clamp(nlo, min=a)
        bhi = torch.clamp(nhi, max=z - 1)
        cnt = torch.clamp(bhi - blo + 1, min=0)
        Pb = int(cnt.sum().item())
        if Pb == 0:
            bm[b] = 0.0
            continue
        off = torch.arange(Pb, device=dev) - torch.repeat_interleave(
            torch.cumsum(cnt, 0) - cnt, cnt
        )
        gsidx = torch.repeat_interleave(blo, cnt) + off  # global sample index
        si = gsidx - a  # block-local index
        pp = (
            torch.repeat_interleave(sign, cnt)
            * torch.repeat_interleave(speed, cnt)
            * (gsidx.to(dt) * ts - torch.repeat_interleave(datum, cnt))
        )
        ww = torch.repeat_interleave(weight, cnt)
        if need_transverse:
            lane_b = torch.repeat_interleave(lane_ax, cnt)
            trans_b = torch.repeat_interleave(trans_ax, cnt)
            track_b = torch.repeat_interleave(track_ax, cnt)
        m = (pp >= 0.0) & (pp <= L)
        si, pp, ww = si[m].contiguous(), pp[m].contiguous(), ww[m].contiguous()
        if need_transverse:
            lane_b, trans_b, track_b = lane_b[m], trans_b[m], track_b[m]
        if any_mode:  # per-pair speed / acceleration for the force-mode coefficient
            speed_b = torch.repeat_interleave(speed, cnt)[m].contiguous()
            accel_b = torch.repeat_interleave(accel, cnt)[m].contiguous()
        E = torch.zeros((n_eff, z - a), dtype=dt, device=dev)  # effect-major rows
        for e, (method, data) in enumerate(prepared):
            mode, bf = modes[e]
            if mode == "vertical":
                ww_e = ww
            elif mode == "centrifugal":
                ww_e = ww * (speed_b * speed_b / GRAVITY)
            else:  # braking: |a|/g per axle, falling back to braking_factor if a==0
                ww_e = ww * torch.where(
                    accel_b != 0.0,
                    accel_b.abs() / GRAVITY,
                    torch.full_like(accel_b, bf),
                )
            if method == "triton1d":
                grid, dx = data
                scatter_interp_1d(si, pp, ww_e, grid, dx, float(weights[e]), E[e])
            elif method == "surface_triton":
                Zt, nxs, nys, x0s, dxs, y0s, dys, lc, lw = data
                y_centre = lc[lane_b] + trans_b - lw[lane_b] / 2.0
                yl = (y_centre - track_b / 2.0).contiguous()
                yr = (y_centre + track_b / 2.0).contiguous()
                scatter_interp_2d(
                    si,
                    pp,
                    yl,
                    yr,
                    ww_e.contiguous(),
                    Zt,
                    nxs,
                    nys,
                    x0s,
                    dxs,
                    y0s,
                    dys,
                    float(wts[e]),
                    E[e],
                )
            elif method == "surface":
                X, Y, ISords, lc, lw = data
                y_centre = lc[lane_b] + trans_b - lw[lane_b] / 2.0
                ol = _surface_ordinate(
                    torch, X, Y, ISords, pp, y_centre - track_b / 2.0
                )
                orr = _surface_ordinate(
                    torch, X, Y, ISords, pp, y_centre + track_b / 2.0
                )
                E[e].index_add_(0, si, ww_e * 0.5 * (ol + orr) * wts[e])
            elif method == "torch_discrete":
                xp, fp = data
                E[e].index_add_(0, si, ww_e * _interp(torch, xp, fp, pp) * wts[e])
            elif method == "torch_builtin":
                ordn = builtin_ordinate(torch, data["id"], data["length"], pp)
                E[e].index_add_(0, si, ww_e * ordn * wts[e])
            else:  # per_lane: each lane uses its own IL + weight (matches the
                # C++ engine's per-lane summation)
                lane_prepared, lane_w = data
                for lane in range(len(lane_prepared)):
                    lm = lane_b == lane
                    if not bool(lm.any()):
                        continue
                    lmethod, ldata = lane_prepared[lane]
                    pl, sl, wl = pp[lm], si[lm], ww_e[lm]
                    if lmethod == "torch_discrete":
                        xp, fp = ldata
                        ordn = _interp(torch, xp, fp, pl)
                    else:  # torch_builtin
                        ordn = builtin_ordinate(torch, ldata["id"], ldata["length"], pl)
                    E[e].index_add_(0, sl, wl * ordn * lane_w[lane])
        # governing extreme per effect = value with the largest magnitude (the
        # C++ engine tracks fabs peaks, so hogging/negative ILs are captured too)
        bmax = E.max(dim=1).values
        bmin = E.min(dim=1).values
        bm[b] = torch.where(bmax.abs() >= bmin.abs(), bmax, bmin)

        if do_pot:
            # map each block sample to its event window, then per effect keep the
            # signed largest-|E| sample per window (merging across blocks for the
            # rare event that straddles a block boundary)
            gidx = torch.arange(a, z, device=dev)
            win_s = (torch.searchsorted(Bt, gidx.to(dt) * ts, right=True) - 1).clamp_(
                0, n_win - 1
            )
            zeros_win = torch.zeros(n_win, dtype=dt, device=dev)
            for e in range(n_eff):
                mag = E[e].abs()
                bmag = torch.full((n_win,), -1.0, dtype=dt, device=dev)
                bmag.scatter_reduce_(0, win_s, mag, reduce="amax", include_self=True)
                # latest sample (max global index) achieving the window's |peak|
                is_pk = mag == bmag.gather(0, win_s)
                cand = torch.where(is_pk, gidx, torch.full_like(gidx, -1))
                bidx = torch.full((n_win,), -1, dtype=torch.long, device=dev)
                bidx.scatter_reduce_(0, win_s, cand, reduce="amax", include_self=True)
                bval = torch.where(
                    bidx >= 0, E[e].gather(0, (bidx - a).clamp_(min=0)), zeros_win
                )
                better = bmag > peakmag[e]
                peakmag[e] = torch.where(better, bmag, peakmag[e])
                peakval[e] = torch.where(better, bval, peakval[e])
                peakidx[e] = torch.where(better, bidx, peakidx[e])

        if rainflows is not None:
            # reduce each effect's block E(t) to turning points on the device
            # (drop strictly-monotonic and flat interiors — a superset of the
            # true reversals, which the C++ extractReversals re-cleans exactly),
            # keep the block endpoints so consecutive blocks join, then feed the
            # small reversal stream to the per-effect rainflow counters.
            for e in range(n_eff):
                row = E[e]
                if row.shape[0] >= 3:
                    dl = row[1:-1] - row[:-2]
                    dr = row[2:] - row[1:-1]
                    keep = ~(
                        (((dl > 0) & (dr > 0)) | ((dl < 0) & (dr < 0)))
                        | ((dl == 0) & (dr == 0))
                    )
                    idx = torch.nonzero(keep, as_tuple=False).flatten() + 1
                    ends = torch.tensor([0, row.shape[0] - 1], device=dev)
                    tp_idx = torch.cat([ends[:1], idx, ends[1:]])
                    tp = row[tp_idx]
                else:
                    tp = row
                rainflows[e].processData(tp.detach().cpu().numpy())
                rainflows[e].calcCycles(False)

        if th is not None:  # time history: write this block's E(t) per sample
            fh, occ, th_offset = th
            E_h = E.detach().cpu().numpy()  # [n_eff, z-a]
            times = np.arange(a, z) * ts + th_offset
            rows = np.column_stack([times, occ[a:z], E_h.T])  # [z-a, 2+n_eff]
            np.savetxt(fh, rows, fmt=["%.3f", "%d"] + ["%.3f"] * n_eff, delimiter="\t")
        del E

    bm_np = bm.cpu().numpy()
    gidx = np.abs(bm_np).argmax(axis=0)
    global_maxima = bm_np[gidx, np.arange(bm_np.shape[1])]
    result = {"block_maxima": bm_np, "global_maxima": global_maxima}
    if do_pot:
        result["pot_peak_value"] = peakval.cpu().numpy()
        result["pot_peak_index"] = peakidx.cpu().numpy()
    return result


def compute_load_effect_maxima(
    extracted,
    il_specs,
    weights,
    bridge_length,
    time_step,
    n_days,
    min_gvw=0,
    block_size_days=1,
    device="cuda",
    dtype="float64",
    time_offset=0.0,
    rainflows=None,
    th_file=None,
):
    """Per-effect block maxima and global maxima via superposition.

    Returns dict with "block_maxima" (n_blocks x n_eff) and "global_maxima" (n_eff,).
    ``extracted`` is the pre-extracted vehicle/axle array tuple.
    ``time_offset`` shifts vehicle times to a window-local origin (streamed runs).
    ``rainflows`` (one ``_Rainflow`` per effect) accumulates fatigue cycles;
    ``th_file`` (an open file) receives the per-sample time history.
    """
    n_eff = len(il_specs)
    axles, n_total, veh = prepare_axles(
        extracted, il_specs, bridge_length, time_step, min_gvw, time_offset
    )
    if axles is None:
        return {"block_maxima": np.zeros((0, n_eff)), "global_maxima": np.zeros(n_eff)}
    th = (
        (th_file, _occupancy(veh, n_total, time_step), time_offset)
        if th_file is not None
        else None
    )
    return compute_from_axles(
        axles,
        n_total,
        il_specs,
        weights,
        bridge_length,
        time_step,
        block_size_days,
        device,
        dtype,
        rainflows=rainflows,
        th=th,
    )


def compute_pot(
    extracted,
    il_specs,
    weights,
    bridge_length,
    time_step,
    min_gvw=0,
    block_size_days=1,
    device="cuda",
    dtype="float64",
    time_offset=0.0,
    rainflows=None,
    th_file=None,
):
    """Peaks-over-threshold event reduction (shares one E(t) pass with BM).

    Rebuilds the C++ event partition from the per-vehicle on-bridge windows and
    reduces E(t) per event window. Returns a dict with the per-window signed
    peak value + sample index ([n_eff, n_window]), the partition (boundaries,
    no.-trucks per window, per-vehicle window span), the per-vehicle window
    arrays, and block maxima — or ``None`` if no vehicle is above ``min_gvw``.
    ``time_offset`` shifts vehicle times to a window-local origin (streamed runs);
    add it back to peak_index*ts / B to recover absolute times.
    """
    from . import pot as potmod

    axles, n_total, veh = prepare_axles(
        extracted, il_specs, bridge_length, time_step, min_gvw, time_offset
    )
    if axles is None:
        return None

    B, win_count, k_start, k_end = potmod.build_partition(veh["t_on"], veh["t_off"])
    th = (
        (th_file, _occupancy(veh, n_total, time_step), time_offset)
        if th_file is not None
        else None
    )
    out = compute_from_axles(
        axles,
        n_total,
        il_specs,
        weights,
        bridge_length,
        time_step,
        block_size_days,
        device,
        dtype,
        pot_boundaries=B,
        rainflows=rainflows,
        th=th,
    )
    return {
        "peak_value": out["pot_peak_value"],  # [n_eff, n_window], signed
        "peak_index": out["pot_peak_index"],  # [n_eff, n_window], -1 if empty
        "block_maxima": out["block_maxima"],
        "B": B,
        "win_count": win_count,
        "k_start": k_start,
        "k_end": k_end,
        "veh": veh,
    }
