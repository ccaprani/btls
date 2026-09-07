"""
Merge primitives for combining per-chunk simulation outputs into a single
logical result (used by the auto-chunked parallel simulation).

Each output type declares *what kind* of merge it needs (a category) plus
which columns carry time / index semantics — the merge functions are generic
and driven entirely by these declarations. To make a future output type
mergeable, add one ``MergeSpec`` entry to ``MERGE_REGISTRY``; no new merge
code should be needed unless it introduces a genuinely new category.

Categories
----------
concat
    Rows from consecutive chunks are concatenated. ``time_cols`` (fnmatch
    patterns) are shifted by each chunk's start offset in seconds;
    ``offset_index_cols`` are continuing counters (block / interval
    indices) shifted by the previous chunks' running maximum;
    ``hour_index_cols`` are indices on the fixed 3600 s grid, shifted by
    the chunk's start hour and summed where chunks overlap;
    ``renumber_index_cols`` are re-sequenced 1..N after concatenation.
    Exact, provided chunk boundaries align with the relevant block /
    interval size (validated at chunking time).
bin_sum
    Histogram-style outputs: group by ``sum_key_cols`` and sum
    ``sum_value_cols`` across chunks.
moment_merge
    Whole-run statistical moments (SS_C): combined with the parallel
    Welford/Chan formulas. Implemented in Phase 2.
vehicles_concat
    Recorded vehicle files (Vehicle objects with embedded timestamps).
    Implemented in Phase 1/2.
rainflow_splice
    Fatigue rainflow histograms (FR_*): closed-cycle bins are summed like
    ``bin_sum``, and each chunk's unclosed residual reversal sequence
    (FRR_* sidecar) is concatenated in chunk order and closed with the
    same C++ rainflow algorithm, falling back to plain bin summing when
    no sidecars exist. See ``merge_rainflow``.
"""

from dataclasses import dataclass, field
from fnmatch import fnmatch

import numpy as np
import pandas as pd

__all__ = [
    "MergeSpec",
    "MERGE_REGISTRY",
    "merge_concat",
    "merge_bin_sum",
    "merge_cumulative_stats",
    "merge_vehicle_traffic",
    "merge_rainflow",
]

# Simulation calendar (CConfigData::Time defaults): 25-day "months",
# 10-month "years" - used by the vehicle-record day arithmetic.
_DAYS_PER_MT = 25
_MTS_PER_YR = 10
_DAYS_PER_YR = _DAYS_PER_MT * _MTS_PER_YR


@dataclass(frozen=True)
class MergeSpec:
    """Declares how one output type merges across chunks."""

    category: str  # "concat" | "bin_sum" | "moment_merge" | "vehicles_concat" | "rainflow_splice"
    time_cols: tuple = ()  # fnmatch patterns; shifted by chunk offset (s)
    offset_index_cols: tuple = ()  # continuing counters; shifted by prior max
    hour_index_cols: tuple = ()  # 1-h grid indices; shifted by chunk start hour
    renumber_index_cols: tuple = ()  # re-sequenced 1..N after concat
    sum_key_cols: tuple = ()  # bin_sum: group-by keys
    sum_value_cols: tuple = ()  # bin_sum: summed values


# Keys match _OutputManager._summary keys (= read_data keys).
MERGE_REGISTRY: dict[str, MergeSpec] = {
    "time_history": MergeSpec("concat", time_cols=("Time",)),
    "all_events": MergeSpec("concat", time_cols=("Start Time",)),
    "traffic": MergeSpec("vehicles_concat"),
    # BM "Index" is the block index — a continuing counter.
    "BM_by_no_trucks": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "BM_by_mixed": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "BM_summary": MergeSpec("concat", offset_index_cols=("Block Index",)),
    # POT_vehicle "Index" is the event's ordinal within its output buffer
    # flush (CPOTManager::WriteVehicleFiles restarts it at 1 after every
    # flush), repeated on one row per load effect within each event —
    # offsetting by the prior chunk's maximum keeps the repeats intact and
    # keeps chunks from colliding.
    "POT_vehicle": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "POT_summary": MergeSpec(
        "concat", time_cols=("Time",), renumber_index_cols=("Peak Index",)
    ),
    "POT_counter": MergeSpec("concat", offset_index_cols=("Block",)),
    # FlowData "Hour" indexes a fixed 1-hour grid from the run start. A
    # chunk also opens a partial hour for the first vehicle generated past
    # its end time, so consecutive chunks overlap on that hour: rebase to
    # absolute hours and sum the counts of coincident rows.
    "traffic_statistics": MergeSpec("concat", hour_index_cols=("Hour",)),
    # Whole-run cumulative moments: needs Welford/Chan combination (Phase 2).
    "E_cumulative_statistics": MergeSpec("moment_merge"),
    # Interval stats are self-contained per interval; SS_S "Time" equals
    # interval_size * ID, so a plain time shift keeps it consistent with the
    # offset ID as long as chunks align with interval boundaries.
    "E_interval_statistics": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "fatigue_events": MergeSpec("concat", time_cols=("Start Time", "Effect * Time")),
    # Closed-cycle histograms add bin-wise; the unclosed residual reversal
    # sequences (FRR_* sidecars, written in chunk mode) are concatenated and
    # closed with the same C++ algorithm — exact residue splicing. Falls
    # back to plain bin_sum when no sidecars exist.
    "fatigue_rainflow": MergeSpec(
        "rainflow_splice", sum_key_cols=("Amplitude",), sum_value_cols=("No. Cycles",)
    ),
}


def _match_cols(df: pd.DataFrame, patterns: tuple) -> list[str]:
    return [c for c in df.columns if any(fnmatch(c, p) for p in patterns)]


def merge_concat(
    chunk_dfs: list[pd.DataFrame], spec: MergeSpec, time_offsets: list[float]
) -> pd.DataFrame:
    """
    Concatenate per-chunk frames in chunk order.

    Parameters
    ----------
    chunk_dfs : list[pd.DataFrame]\n
        One frame per chunk, in chunk (time) order. Empty frames allowed.
    spec : MergeSpec\n
        The declaration for this output type (category "concat").
    time_offsets : list[float]\n
        Start time of each chunk in seconds, relative to the merged timeline
        (chunk 0 is usually 0.0).

    Returns
    -------
    pd.DataFrame\n
        The merged frame, indistinguishable in shape from a single long run.
    """

    if len(chunk_dfs) != len(time_offsets):
        raise ValueError("chunk_dfs and time_offsets must have equal length.")

    shifted = []
    index_base = {col: 0 for col in spec.offset_index_cols}

    for df, offset in zip(chunk_dfs, time_offsets):
        if df.empty:
            continue
        df = df.copy()
        for col in _match_cols(df, spec.time_cols):
            new_col = df[col] + offset
            # Keep integer time columns integer (e.g. SS_S "Time").
            if (
                pd.api.types.is_integer_dtype(df[col].dtype)
                and float(offset).is_integer()
            ):
                new_col = new_col.astype(df[col].dtype)
            df[col] = new_col
        for col in spec.offset_index_cols:
            if col in df.columns:
                base = index_base[col]
                df[col] = df[col] + base
                index_base[col] = int(df[col].max())
        for col in spec.hour_index_cols:
            if col in df.columns:
                df[col] = df[col] + int(offset // 3600)
        shifted.append(df)

    if not shifted:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    merged = pd.concat(shifted, ignore_index=True)

    grid_cols = [col for col in spec.hour_index_cols if col in merged.columns]
    if grid_cols:
        # Chunks overlap on the boundary hour - add their counts together.
        merged = merged.groupby(grid_cols, as_index=False).sum()

    for col in spec.renumber_index_cols:
        if col in merged.columns:
            merged[col] = range(1, len(merged) + 1)

    return merged


def merge_bin_sum(chunk_dfs: list[pd.DataFrame], spec: MergeSpec) -> pd.DataFrame:
    """
    Merge histogram-style frames by summing values over the key columns.

    Parameters
    ----------
    chunk_dfs : list[pd.DataFrame]\n
        One frame per chunk. Empty frames allowed.
    spec : MergeSpec\n
        The declaration for this output type (category "bin_sum").

    Returns
    -------
    pd.DataFrame\n
        One row per key, values summed across chunks, sorted by key.
    """

    frames = [df for df in chunk_dfs if not df.empty]
    if not frames:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    merged = pd.concat(frames, ignore_index=True)
    keys = list(spec.sum_key_cols)
    values = list(spec.sum_value_cols)
    merged = merged.groupby(keys, as_index=False)[values].sum()
    return merged.sort_values(keys, ignore_index=True)


def merge_cumulative_stats(chunk_dfs: list[pd.DataFrame]) -> pd.DataFrame:
    """
    Merge SS_C cumulative-statistics frames (one row per load effect) by
    reconstructing the raw moment sums from each chunk's reported
    statistics and combining them with the parallel (Chan et al.) update
    formulas - the exact counterpart of the C++ online accumulator
    (CEventStatistics::accumulator/finalize).

    The inversion (M2 from Variance, M3 from Skewness, M4 from Kurtosis)
    is mathematically exact; the only error source is the fixed-precision
    text formatting of the input files.

    Parameters
    ----------
    chunk_dfs : list[pd.DataFrame]\n
        One frame per chunk, as returned by ``read_E_CS`` (columns:
        Effect, No. Events, No. Vehicles, No. Trucks, Min, Max, Mean,
        Std Dev, Variance, Skewness, Kurtosis).

    Returns
    -------
    pd.DataFrame\n
        A single frame with the same columns, equal to what one
        continuous run would have reported.
    """

    frames = [df for df in chunk_dfs if not df.empty]
    if not frames:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    # Combined integer tallies are exact.
    out = frames[0].copy()
    base = frames[0].set_index("Effect")
    effects = base.index

    n = base["No. Events"].to_numpy(dtype=float)
    mean = base["Mean"].to_numpy(dtype=float)
    m2, m3, m4 = _invert_moments(base, n)
    lo = np.where(n > 0, base["Min"].to_numpy(dtype=float), np.inf)
    hi = np.where(n > 0, base["Max"].to_numpy(dtype=float), -np.inf)
    vehs = base["No. Vehicles"].to_numpy(dtype=np.int64)
    trks = base["No. Trucks"].to_numpy(dtype=np.int64)

    for df in frames[1:]:
        other = df.set_index("Effect").loc[effects]
        nb = other["No. Events"].to_numpy(dtype=float)
        mean_b = other["Mean"].to_numpy(dtype=float)
        m2b, m3b, m4b = _invert_moments(other, nb)

        na = n
        ntot = na + nb
        delta = mean_b - mean
        with np.errstate(invalid="ignore", divide="ignore"):
            mean = np.where(ntot > 0, mean + delta * nb / ntot, 0.0)
            m4 = (
                m4
                + m4b
                + delta**4 * na * nb * (na**2 - na * nb + nb**2) / ntot**3
                + 6.0 * delta**2 * (na**2 * m2b + nb**2 * m2) / ntot**2
                + 4.0 * delta * (na * m3b - nb * m3) / ntot
            )
            m3 = (
                m3
                + m3b
                + delta**3 * na * nb * (na - nb) / ntot**2
                + 3.0 * delta * (na * m2b - nb * m2) / ntot
            )
            m2 = m2 + m2b + delta**2 * na * nb / ntot
        m4 = np.nan_to_num(m4)
        m3 = np.nan_to_num(m3)
        m2 = np.nan_to_num(m2)
        n = ntot

        lo = np.where(nb > 0, np.minimum(lo, other["Min"].to_numpy(dtype=float)), lo)
        hi = np.where(nb > 0, np.maximum(hi, other["Max"].to_numpy(dtype=float)), hi)
        vehs = vehs + other["No. Vehicles"].to_numpy(dtype=np.int64)
        trks = trks + other["No. Trucks"].to_numpy(dtype=np.int64)

    # A zero-event effect reports 0.0 extremes, as CEventStatistics does.
    lo = np.where(n > 0, lo, 0.0)
    hi = np.where(n > 0, hi, 0.0)

    # Finalize exactly like CEventStatistics::finalize().
    ok = (n >= 2) & (m2 > 0.0)
    with np.errstate(invalid="ignore", divide="ignore"):
        variance = np.where(ok, m2 / (n - 1), 0.0)
        std_dev = np.sqrt(variance)
        skewness = np.where(ok, np.sqrt(n) * m3 / np.sqrt(m2**3), 0.0)
        kurtosis = np.where(ok, n * m4 / m2**2 - 3.0, 0.0)

    out["No. Events"] = n.astype(np.int64)
    out["No. Vehicles"] = vehs
    out["No. Trucks"] = trks
    out["Min"] = lo
    out["Max"] = hi
    out["Mean"] = mean
    out["Std Dev"] = std_dev
    out["Variance"] = variance
    out["Skewness"] = skewness
    out["Kurtosis"] = kurtosis
    return out


def _invert_moments(
    indexed_df: pd.DataFrame, n: "np.ndarray"
) -> tuple["np.ndarray", "np.ndarray", "np.ndarray"]:
    """Recover (M2, M3, M4) from finalized statistics, inverting
    CEventStatistics::finalize()."""

    variance = indexed_df["Variance"].to_numpy(dtype=float)
    skewness = indexed_df["Skewness"].to_numpy(dtype=float)
    kurtosis = indexed_df["Kurtosis"].to_numpy(dtype=float)

    ok = n >= 2
    with np.errstate(invalid="ignore", divide="ignore"):
        m2 = np.where(ok, variance * (n - 1), 0.0)
        m3 = np.where(ok, skewness * np.sqrt(m2**3) / np.sqrt(n), 0.0)
        m4 = np.where(ok, (kurtosis + 3.0) * m2**2 / n, 0.0)
    return np.nan_to_num(m2), np.nan_to_num(m3), np.nan_to_num(m4)


def merge_rainflow(
    chunk_dfs: list[pd.DataFrame],
    residual_seqs: list[list[float]],
    decimal: int,
    cutoff: float,
) -> pd.DataFrame:
    """
    Merge rainflow histograms exactly: sum the per-chunk closed-cycle
    counts, then concatenate the chunks' residual reversal sequences in
    order and close them with the same C++ rainflow algorithm (residue
    splicing). The result equals what one continuous run would report.

    Parameters
    ----------
    chunk_dfs : list[pd.DataFrame]\n
        Per-chunk frames as returned by ``read_FR`` (closed cycles only,
        i.e. the chunks were run with ``write_residuals=True``).
    residual_seqs : list[list[float]]\n
        Per-chunk residual reversal sequences, in chunk (time) order.
    decimal : int\n
        Rainflow binning precision (RAINFLOW_DECIMAL).
    cutoff : float\n
        Amplitude cut-off (RAINFLOW_CUTOFF).

    Returns
    -------
    pd.DataFrame\n
        Columns Amplitude / No. Cycles, sorted by amplitude.
    """

    from ..lib import libbtls

    frames = [df for df in chunk_dfs if not df.empty]

    spliced = []
    for seq in residual_seqs:
        spliced.extend(seq)
    if spliced:
        counter = libbtls._Rainflow(decimal, cutoff)
        counter.processData(spliced)
        counter.calcCycles(True)
        closure = counter.getRainflowOutput()
        if closure:
            frames.append(
                pd.DataFrame(
                    {
                        "Amplitude": list(closure.keys()),
                        "No. Cycles": list(closure.values()),
                    }
                )
            )

    if not frames:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    merged = pd.concat(frames, ignore_index=True)
    # Re-round so text-parsed amplitudes and C++-computed bin keys collapse
    # into the same groups despite last-ulp float differences.
    if decimal >= 0:
        merged["Amplitude"] = merged["Amplitude"].round(decimal)
    merged = merged.groupby("Amplitude", as_index=False)["No. Cycles"].sum()
    return merged.sort_values("Amplitude", ignore_index=True)


def merge_vehicle_traffic(
    chunk_dfs: list[pd.DataFrame], day_offsets: list[int]
) -> pd.DataFrame:
    """
    Merge recorded-vehicle frames (``read_traffic``) by advancing each
    chunk's calendar fields by its start offset in days. The simulation
    calendar uses 25-day months and 10-month years (CConfigData::Time).

    Parameters
    ----------
    chunk_dfs : list[pd.DataFrame]\n
        One frame per chunk, in chunk order.
    day_offsets : list[int]\n
        Start day of each chunk on the merged timeline (chunk 0 = 0).

    Returns
    -------
    pd.DataFrame\n
        The merged vehicle frame with continuous calendar fields. The
        vehicle id ("Head") is a source-record identifier and is left
        exactly as each chunk wrote it.
    """

    if len(chunk_dfs) != len(day_offsets):
        raise ValueError("chunk_dfs and day_offsets must have equal length.")

    shifted = []
    for df, offset in zip(chunk_dfs, day_offsets):
        if df.empty:
            continue
        df = df.copy()
        abs_day = (
            (df["Year"].astype(np.int64)) * _DAYS_PER_YR
            + (df["Month"].astype(np.int64) - 1) * _DAYS_PER_MT
            + (df["Day"].astype(np.int64) - 1)
            + int(offset)
        )
        df["Year"] = abs_day // _DAYS_PER_YR
        df["Month"] = (abs_day % _DAYS_PER_YR) // _DAYS_PER_MT + 1
        df["Day"] = abs_day % _DAYS_PER_MT + 1
        shifted.append(df)

    if not shifted:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    return pd.concat(shifted, ignore_index=True)
