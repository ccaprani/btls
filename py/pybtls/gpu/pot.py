"""
Host-side peaks-over-threshold (POT) event reconstruction for the GPU engine.

The C++ engine forms an *event* every time the set of vehicles on the bridge
changes: an event window is bounded by vehicle on/off transitions, and its
"peak" is the largest |E| over that window (recorded if it exceeds the
threshold). See cpp/src/Bridge.cpp (event loop) and cpp/src/POTManager.cpp.

Because the GPU engine reconstructs each vehicle's on-bridge interval
``[t_on, t_off)`` exactly as the C++ engine does
(``t_on = get_time()``, ``t_off = t_on + (L + length) / speed``,
cpp/src/Vehicle.cpp:814-830), the same event partition can be rebuilt here:

  * boundaries  ``B`` = sorted unique union of every vehicle's t_on and t_off,
  * window ``w`` = the interval ``[B[w], B[w+1])``; the vehicle set is constant
    over it,
  * ``win_count[w]`` = how many vehicles cover window ``w`` (its "no. trucks"),
    an event iff >= 1.

This module only builds the partition + membership (pure event geometry); the
per-window peak values come from the device reduction in :mod:`engine`, and the
PT_S / PT_C / PT_V files are written by :mod:`runner`.
"""

import numpy as np


def build_partition(t_on, t_off):
    """Build the event-window partition from per-vehicle on/off times.

    Parameters
    ----------
    t_on, t_off : ndarray
        Per (kept) vehicle bridge-entry / bridge-exit times, seconds.

    Returns
    -------
    B : ndarray
        Sorted unique boundary times; windows are ``[B[w], B[w+1])`` for
        ``w`` in ``range(len(B) - 1)``.
    win_count : ndarray[int64]
        Number of vehicles covering each window (its no.-trucks).
    k_start, k_end : ndarray[int64]
        Per vehicle, the half-open window-index range ``[k_start, k_end)`` it
        covers (used to list an event's member vehicles).
    """
    B = np.unique(np.concatenate([t_on, t_off]))
    n_win = len(B) - 1
    if n_win <= 0:
        z = np.zeros(0, dtype=np.int64)
        return B, z, z, z

    # each vehicle's interval starts/ends exactly on a boundary, so its window
    # span is [index-of-t_on, index-of-t_off)
    k_start = np.searchsorted(B, t_on, side="left").astype(np.int64)
    k_end = np.searchsorted(B, t_off, side="left").astype(np.int64)

    # occupancy per window via a +1/-1 difference array + prefix sum
    diff = np.zeros(n_win + 1, dtype=np.int64)
    np.add.at(diff, k_start, 1)
    np.add.at(diff, k_end, -1)
    win_count = np.cumsum(diff)[:n_win]
    return B, win_count, k_start, k_end


def truck_occupancy(k_start, k_end, n_win, is_truck):
    """Per-window count of non-car (truck) vehicles covering it — the stats
    ``No. Trucks`` column. Same +1/-1 difference array as ``build_partition``'s
    ``win_count`` but restricted to the truck members (``is_truck`` masks the
    per-vehicle ``k_start`` / ``k_end`` spans)."""
    diff = np.zeros(n_win + 1, dtype=np.int64)
    np.add.at(diff, k_start[is_truck], 1)
    np.add.at(diff, k_end[is_truck], -1)
    return np.cumsum(diff)[:n_win]


def window_members_csr(k_start, k_end, n_win):
    """CSR map window -> covering vehicles for *every* window in one pass.

    A vehicle covers the contiguous window range ``[k_start, k_end)``; expanding
    those ranges and grouping by window gives, for window ``w``, the member
    (kept-vehicle) indices ``members[indptr[w]:indptr[w+1]]``. Used for PT_V when
    a low threshold makes most events qualify (avoids an O(n_veh) scan per
    event). Total size = sum of per-vehicle window spans = the inherent
    event-vehicle incidence count that PT_V writes anyway."""
    spans = (k_end - k_start).astype(np.int64)
    total = int(spans.sum())
    indptr = np.zeros(n_win + 1, dtype=np.int64)
    if total == 0:
        return indptr, np.zeros(0, dtype=np.int64)
    veh_rep = np.repeat(np.arange(len(k_start), dtype=np.int64), spans)
    starts = np.cumsum(spans) - spans
    win_rep = np.repeat(k_start, spans) + (np.arange(total) - np.repeat(starts, spans))
    order = np.argsort(win_rep, kind="stable")     # group by window
    members = veh_rep[order]
    counts = np.bincount(win_rep, minlength=n_win)
    indptr[1:] = np.cumsum(counts)
    return indptr, members
