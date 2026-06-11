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
    ``offset_index_cols`` are continuing counters (block / interval / hour
    indices) shifted by the previous chunks' running maximum;
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
"""

from dataclasses import dataclass, field
from fnmatch import fnmatch

import pandas as pd

__all__ = ["MergeSpec", "MERGE_REGISTRY", "merge_concat", "merge_bin_sum"]


@dataclass(frozen=True)
class MergeSpec:
    """Declares how one output type merges across chunks."""

    category: str  # "concat" | "bin_sum" | "moment_merge" | "vehicles_concat"
    time_cols: tuple = ()  # fnmatch patterns; shifted by chunk offset (s)
    offset_index_cols: tuple = ()  # continuing counters; shifted by prior max
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
    # POT_vehicle "Index" is a continuing event counter, repeated on one row
    # per load effect within each event — offsetting by the prior chunk's
    # maximum keeps the repeats intact.
    "POT_vehicle": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "POT_summary": MergeSpec(
        "concat", time_cols=("Time",), renumber_index_cols=("Peak Index",)
    ),
    "POT_counter": MergeSpec("concat", offset_index_cols=("Block",)),
    # FlowData "Hour" increments monotonically over the whole run.
    "traffic_statistics": MergeSpec("concat", offset_index_cols=("Hour",)),
    # Whole-run cumulative moments: needs Welford/Chan combination (Phase 2).
    "E_cumulative_statistics": MergeSpec("moment_merge"),
    # Interval stats are self-contained per interval; SS_S "Time" equals
    # interval_size * ID, so a plain time shift keeps it consistent with the
    # offset ID as long as chunks align with interval boundaries.
    "E_interval_statistics": MergeSpec(
        "concat", time_cols=("Time",), offset_index_cols=("Index",)
    ),
    "fatigue_events": MergeSpec(
        "concat", time_cols=("Start Time", "Effect * Time")
    ),
    # v1: closed-cycle histograms add bin-wise. Residual reversals at chunk
    # boundaries are spliced exactly in Phase 2.
    "fatigue_rainflow": MergeSpec(
        "bin_sum", sum_key_cols=("Amplitude",), sum_value_cols=("No. Cycles",)
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
            if pd.api.types.is_integer_dtype(df[col].dtype) and float(
                offset
            ).is_integer():
                new_col = new_col.astype(df[col].dtype)
            df[col] = new_col
        for col in spec.offset_index_cols:
            if col in df.columns:
                base = index_base[col]
                df[col] = df[col] + base
                index_base[col] = int(df[col].max())
        shifted.append(df)

    if not shifted:
        return chunk_dfs[0].copy() if chunk_dfs else pd.DataFrame()

    merged = pd.concat(shifted, ignore_index=True)

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
