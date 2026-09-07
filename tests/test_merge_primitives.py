"""
Unit tests for the chunk-merge primitives (pybtls.output._merge) on synthetic
data. These cover the generic merge mechanics; end-to-end equivalence against
real simulation outputs is tested separately (test_parallel_equivalence.py).
"""

import pandas as pd
import pytest

from pybtls.output._merge import (
    MERGE_REGISTRY,
    MergeSpec,
    merge_bin_sum,
    merge_concat,
    merge_vehicle_traffic,
)


def test_registry_covers_all_output_keys():
    # Must stay in sync with _OutputManager._summary.
    expected = {
        "time_history",
        "all_events",
        "traffic",
        "BM_by_no_trucks",
        "BM_by_mixed",
        "BM_summary",
        "POT_vehicle",
        "POT_summary",
        "POT_counter",
        "traffic_statistics",
        "E_cumulative_statistics",
        "E_interval_statistics",
        "fatigue_events",
        "fatigue_rainflow",
    }
    assert set(MERGE_REGISTRY.keys()) == expected


def test_concat_shifts_time_columns():
    spec = MERGE_REGISTRY["all_events"]
    a = pd.DataFrame({"Start Time": [10.0, 50.0], "No. Trucks": [1, 2]})
    b = pd.DataFrame({"Start Time": [5.0, 20.0], "No. Trucks": [1, 3]})

    merged = merge_concat([a, b], spec, [0.0, 86400.0])

    assert merged["Start Time"].tolist() == [10.0, 50.0, 86405.0, 86420.0]
    assert merged["No. Trucks"].tolist() == [1, 2, 1, 3]


def test_concat_offsets_continuing_counters():
    spec = MERGE_REGISTRY["BM_summary"]
    a = pd.DataFrame({"Block Index": [1, 2, 3], "1-Truck Event": [5.0, 7.0, 6.0]})
    b = pd.DataFrame({"Block Index": [1, 2], "1-Truck Event": [8.0, 4.0]})

    merged = merge_concat([a, b], spec, [0.0, 3 * 86400.0])

    assert merged["Block Index"].tolist() == [1, 2, 3, 4, 5]


def test_concat_offsets_accumulate_over_three_chunks():
    spec = MERGE_REGISTRY["POT_counter"]
    chunks = [
        pd.DataFrame({"Block": [1, 2], "Effect 1": [3, 1]}),
        pd.DataFrame({"Block": [1], "Effect 1": [9]}),
        pd.DataFrame({"Block": [1, 2], "Effect 1": [0, 2]}),
    ]

    merged = merge_concat(chunks, spec, [0.0, 2.0, 3.0])

    assert merged["Block"].tolist() == [1, 2, 3, 4, 5]
    assert merged["Effect 1"].tolist() == [3, 1, 9, 0, 2]


def test_concat_renumbers_event_counters():
    spec = MERGE_REGISTRY["POT_summary"]
    a = pd.DataFrame(
        {"Peak Index": [1, 2], "Time": [9.0, 90.0], "Peak Value": [1.0, 2.0]}
    )
    b = pd.DataFrame({"Peak Index": [1], "Time": [9.0], "Peak Value": [3.0]})

    merged = merge_concat([a, b], spec, [0.0, 100.0])

    assert merged["Peak Index"].tolist() == [1, 2, 3]
    assert merged["Time"].tolist() == [9.0, 90.0, 109.0]


def test_concat_matches_wildcard_time_columns():
    spec = MERGE_REGISTRY["fatigue_events"]
    cols = {
        "Start Time": [1.0],
        "No. Trucks": [2],
        "Effect 1 Max Time": [3.0],
        "Effect 1 Max Amplitude": [40.0],
        "Effect 1 Min Time": [5.0],
        "Effect 1 Min Amplitude": [-6.0],
    }
    a = pd.DataFrame(cols)
    b = pd.DataFrame(cols)

    merged = merge_concat([a, b], spec, [0.0, 1000.0])

    row = merged.iloc[1]
    assert row["Start Time"] == 1001.0
    assert row["Effect 1 Max Time"] == 1003.0
    assert row["Effect 1 Min Time"] == 1005.0
    # Amplitudes are values, not times — must be untouched.
    assert row["Effect 1 Max Amplitude"] == 40.0
    assert row["Effect 1 Min Amplitude"] == -6.0


def test_concat_skips_empty_chunks():
    spec = MERGE_REGISTRY["all_events"]
    a = pd.DataFrame({"Start Time": [], "No. Trucks": []})
    b = pd.DataFrame({"Start Time": [7.0], "No. Trucks": [1]})

    merged = merge_concat([a, b], spec, [0.0, 50.0])

    assert merged["Start Time"].tolist() == [57.0]


def test_concat_all_empty_returns_empty():
    spec = MERGE_REGISTRY["all_events"]
    a = pd.DataFrame({"Start Time": [], "No. Trucks": []})

    merged = merge_concat([a, a], spec, [0.0, 50.0])

    assert merged.empty


def test_concat_length_mismatch_raises():
    spec = MERGE_REGISTRY["all_events"]
    a = pd.DataFrame({"Start Time": [1.0], "No. Trucks": [1]})

    with pytest.raises(ValueError):
        merge_concat([a, a], spec, [0.0])


def test_bin_sum_adds_overlapping_bins():
    spec = MERGE_REGISTRY["fatigue_rainflow"]
    a = pd.DataFrame({"Amplitude": [1.0, 2.0], "No. Cycles": [10.0, 5.0]})
    b = pd.DataFrame({"Amplitude": [2.0, 3.0], "No. Cycles": [1.0, 4.0]})

    merged = merge_bin_sum([a, b], spec)

    assert merged["Amplitude"].tolist() == [1.0, 2.0, 3.0]
    assert merged["No. Cycles"].tolist() == [10.0, 6.0, 4.0]


def test_bin_sum_single_chunk_is_identity():
    spec = MERGE_REGISTRY["fatigue_rainflow"]
    a = pd.DataFrame({"Amplitude": [2.0, 1.0], "No. Cycles": [5.0, 10.0]})

    merged = merge_bin_sum([a], spec)

    # Sorted by key, values preserved.
    assert merged["Amplitude"].tolist() == [1.0, 2.0]
    assert merged["No. Cycles"].tolist() == [10.0, 5.0]


def test_vehicle_traffic_shifts_calendar_across_month_and_year():
    # The simulation calendar has 25-day months and 10-month years; Year is
    # 0-based while Month/Day are 1-based (CVehicle::getTime/setTime).
    chunk_0 = pd.DataFrame(
        {"Head": [1, 2], "Year": [0, 0], "Month": [1, 1], "Day": [1, 25]}
    )
    chunk_1 = pd.DataFrame(
        {"Head": [1, 2], "Year": [0, 0], "Month": [1, 1], "Day": [1, 2]}
    )
    chunk_2 = pd.DataFrame({"Head": [1], "Year": [0], "Month": [1], "Day": [2]})

    merged = merge_vehicle_traffic([chunk_0, chunk_1, chunk_2], [0, 25, 249])

    # Chunk 1 starts on absolute day 25 (month boundary), chunk 2 on day 249
    # so its second day is the first day of the next year.
    assert merged["Year"].tolist() == [0, 0, 0, 0, 1]
    assert merged["Month"].tolist() == [1, 1, 2, 2, 1]
    assert merged["Day"].tolist() == [1, 25, 1, 2, 1]


def test_vehicle_traffic_preserves_head_verbatim():
    # "Head" is the record identifier of the source data row
    # (CVehicle::m_Head), not a running vehicle counter: recorded traffic
    # takes it from the file row number, and every generated vehicle gets
    # the constant 1001 (CVehicleGenerator::GenerateVehicle). Chunking only
    # accepts generated traffic, so offsetting it by the previous chunk's
    # maximum would turn 1001 into 2002, 3003, ... and overflow the 4-digit
    # Head field of the CASTOR/BeDIT/DITIS writers. It must pass through.
    chunk_0 = pd.DataFrame(
        {"Head": [1001, 1001], "Year": [0, 0], "Month": [1, 1], "Day": [1, 1]}
    )
    chunk_1 = pd.DataFrame(
        {"Head": [1001, 1001], "Year": [0, 0], "Month": [1, 1], "Day": [1, 1]}
    )

    merged = merge_vehicle_traffic([chunk_0, chunk_1], [0, 1])

    assert merged["Head"].tolist() == [1001, 1001, 1001, 1001]
