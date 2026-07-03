"""
Regression tests for the output/read and output/plot bugs found in
dev_log/audit_20260704/bugs_output.md and docs_audit_read_part1.md.
"""

import matplotlib

matplotlib.use("Agg")

import pandas as pd
import pytest

from pybtls.output.plot import plot_TH
from pybtls.output.read import (
    read_AE,
    read_BM_All,
    read_BM_S,
    read_BM_V,
    read_E_CS,
    read_E_IS,
    read_FE,
    read_FR,
    read_POT_C,
    read_POT_S,
    read_POT_V,
    read_TH,
    read_TS,
)
from pybtls.output.read.event_file import read_event_file


# --- FIX 1: read_TS header sniff / axle-classifier columns ------------------


def test_read_TS_detects_axle_classifier_header(tmp_path):
    path = tmp_path / "FlowData_1_1.txt"
    path.write_text(
        "        Hour   #Vehicles     #Trucks       #Cars     0: Default"
        "        1: Car     2: 2-axle     3: 3-axle     4: 4-axle     5: 5-axle\n"
        "           1          10           5           5              0"
        "             5             2             1             1             1\n"
    )

    df = read_TS(path)

    assert list(df.columns) == [
        "Hour",
        "No. Vehicles",
        "No. Trucks",
        "No. Cars",
        "0: Default",
        "1: Car",
        "2: 2-axle",
        "3: 3-axle",
        "4: 4-axle",
        "5: 5-axle",
    ]
    assert df["2: 2-axle"].iloc[0] == 2
    assert df["5: 5-axle"].iloc[0] == 1
    assert not df.isna().any().any()


def test_read_TS_detects_pattern_classifier_header(tmp_path):
    path = tmp_path / "FlowData_1_1.txt"
    path.write_text(
        "        Hour   #Vehicles     #Trucks       #Cars     0: Default"
        "        1: Car 2: Pattern 11 3: Pattern 123 4: Pattern 12"
        " 5: Pattern 1233 6: Pattern 122 7: Pattern 112 8: Pattern 113\n"
        "           1          10           5           5              0"
        "             5             1             1             1"
        "             1             0             0             1\n"
    )

    df = read_TS(path)

    assert list(df.columns) == [
        "Hour",
        "No. Vehicles",
        "No. Trucks",
        "No. Cars",
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
    assert not df.isna().any().any()


def test_read_TS_empty_file(tmp_path):
    path = tmp_path / "FlowData_1_1.txt"
    path.write_text("")

    df = read_TS(path)

    assert len(df) == 0
    # empty file cannot be classified: pin the fallback (pattern) schema so a
    # fallback change is a conscious decision
    assert list(df.columns[:4]) == ["Hour", "No. Vehicles", "No. Trucks", "No. Cars"]
    assert len(df.columns) == 13


# --- FIX 2: read_FE Max/Min chronological-order swap ------------------------


def test_read_FE_max_min_not_swapped_regardless_of_chronological_order(tmp_path):
    path = tmp_path / "BL_20.0_Fatigue.txt"
    # Event 1: the minimum occurs before the maximum (min written on line 1).
    # Event 2: the maximum occurs before the minimum (max written on line 1).
    path.write_text(
        "        1000.00\t         100.00\t     10.00\n"
        "              3\t         150.00\t     50.00\n"
        "        2000.00\t         200.00\t     60.00\n"
        "              4\t         250.00\t      5.00\n"
    )

    df = read_FE(path)

    assert len(df) == 2
    assert df["No. Vehicles"].tolist() == [3, 4]

    assert df["Effect 1 Max Time"].iloc[0] == 150.0
    assert df["Effect 1 Max Amplitude"].iloc[0] == 50.0
    assert df["Effect 1 Min Time"].iloc[0] == 100.0
    assert df["Effect 1 Min Amplitude"].iloc[0] == 10.0

    assert df["Effect 1 Max Time"].iloc[1] == 200.0
    assert df["Effect 1 Max Amplitude"].iloc[1] == 60.0
    assert df["Effect 1 Min Time"].iloc[1] == 250.0
    assert df["Effect 1 Min Amplitude"].iloc[1] == 5.0


# --- FIX 3: read_FE empty file ----------------------------------------------


def test_read_FE_empty_file(tmp_path):
    path = tmp_path / "BL_20.0_Fatigue.txt"
    path.write_text("")

    df = read_FE(path)

    assert len(df) == 0
    assert list(df.columns) == ["Start Time", "No. Vehicles"]


# --- FIX 4: empty-file handling across output/read --------------------------


def test_read_AE_empty_file(tmp_path):
    path = tmp_path / "BL_20.0_AllEvents.txt"
    path.write_text("")
    df = read_AE(path)
    assert len(df) == 0
    assert list(df.columns) == ["Start Time", "No. Vehicles"]


def test_read_TH_empty_file(tmp_path):
    path = tmp_path / "TH_20.0.txt"
    path.write_text("")
    df = read_TH(path)
    assert len(df) == 0
    assert list(df.columns) == ["Time", "No. Vehicles"]


def test_read_POT_C_empty_file(tmp_path):
    path = tmp_path / "PT_C_20.0.txt"
    path.write_text("")
    df = read_POT_C(path)
    assert len(df) == 0
    assert list(df.columns) == ["Block"]


def test_read_POT_S_empty_file(tmp_path):
    path = tmp_path / "PT_S_20.0_1.txt"
    path.write_text("")
    df = read_POT_S(path)
    assert len(df) == 0
    assert list(df.columns) == ["Peak Index", "Time", "No. Vehicles", "Peak Value"]


def test_read_FR_empty_file(tmp_path):
    path = tmp_path / "FR_20.0_1.txt"
    path.write_text("")
    df = read_FR(path)
    assert len(df) == 0
    assert list(df.columns) == ["Amplitude", "No. Cycles"]


def test_read_BM_S_empty_file(tmp_path):
    path = tmp_path / "BM_S_20.0_1.txt"
    path.write_text("")
    df = read_BM_S(path)
    assert len(df) == 0
    assert list(df.columns) == ["Block Index"]


def test_read_E_IS_empty_file(tmp_path):
    path = tmp_path / "SS_IS_20.0_1.txt"
    path.write_text("")
    df = read_E_IS(path)
    assert len(df) == 0
    assert list(df.columns) == [
        "Index",
        "Time",
        "No. Events",
        "No. Vehicles",
        "No. Trucks",
        "Min",
        "Max",
        "Mean",
        "Std Dev",
        "Variance",
        "Skewness",
        "Kurtosis",
    ]


def test_read_E_CS_empty_file(tmp_path):
    path = tmp_path / "SS_CS_20.0_1.txt"
    path.write_text("")
    df = read_E_CS(path)
    assert len(df) == 0
    assert list(df.columns) == [
        "Effect",
        "No. Events",
        "No. Vehicles",
        "No. Trucks",
        "Min",
        "Max",
        "Mean",
        "Std Dev",
        "Variance",
        "Skewness",
        "Kurtosis",
    ]


@pytest.mark.parametrize(
    "reader", [read_event_file, read_BM_All, read_BM_V, read_POT_V]
)
def test_read_event_file_family_empty_file(tmp_path, reader):
    path = tmp_path / "BM_V_20.0_1.txt"
    path.write_text("")
    df = reader(path)
    assert len(df) == 0
    assert list(df.columns) == [
        "Index",
        "Effect",
        "Value",
        "Time",
        "Position on Bridge",
        "No. Vehicles",
        "Trucks",
    ]


# --- FIX 5: "No. Trucks" -> "No. Vehicles" rename ---------------------------


def test_read_AE_has_no_vehicles_column(tmp_path):
    path = tmp_path / "BL_20.0_AllEvents.txt"
    path.write_text("100.000\t3\t50.0\n")
    df = read_AE(path)
    assert "No. Vehicles" in df.columns
    assert df["No. Vehicles"].iloc[0] == 3


def test_read_event_file_has_no_vehicles_column(tmp_path):
    path = tmp_path / "BM_V_20.0_1.txt"
    path.write_text("1\n1 50.0 100.0 12.5 3\n")
    df = read_event_file(path)
    assert "No. Vehicles" in df.columns
    assert df["No. Vehicles"].iloc[0] == 3


def test_read_FE_has_no_vehicles_column(tmp_path):
    path = tmp_path / "BL_20.0_Fatigue.txt"
    path.write_text("1000.00\t100.00\t10.00\n3\t150.00\t50.00\n")
    df = read_FE(path)
    assert "No. Vehicles" in df.columns
    assert df["No. Vehicles"].iloc[0] == 3


def test_read_E_CS_keeps_no_trucks_column(tmp_path):
    # E_cumulative_statistics genuinely reports truck counts; must not rename.
    path = tmp_path / "SS_CS_20.0_1.txt"
    path.write_text("header\n" "1 10 8 5 1.0 5.0 3.0 0.5 0.25 0.1 -0.2 0 0 0\n")
    df = read_E_CS(path)
    assert "No. Trucks" in df.columns
    assert df["No. Trucks"].iloc[0] == 5


# --- FIX 6: read_POT_S Peak Index renumbering -------------------------------


def test_read_POT_S_peak_index_offset_by_start_line(tmp_path):
    path = tmp_path / "PT_S_20.0_1.txt"
    path.write_text(
        "1 100 2 50\n" "2 200 2 51\n" "3 1300 3 53\n" "4 1400 3 54\n" "5 1500 2 55\n"
    )

    df = read_POT_S(path, start_line=3, no_lines=2)

    assert df["Peak Index"].tolist() == [3, 4]
    assert df["Time"].tolist() == [1300, 1400]
    assert df["Peak Value"].tolist() == [53, 54]


# --- FIX 7: plot_TH single-row crash ----------------------------------------


def test_plot_TH_single_row_does_not_crash(tmp_path):
    data = pd.DataFrame({"Time": [0.0], "No. Vehicles": [1], "Effect 1": [12.5]})
    save_to = tmp_path / "th_single_row.png"

    plot_TH(data, save_to=save_to)

    assert save_to.exists()
