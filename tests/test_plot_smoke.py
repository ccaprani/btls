"""
Smoke tests for output/plot/*. Column names match the current read_*
counterparts, matching the actual on-disk column set produced after the
recent "No. Vehicles" rename in output/read/*.
"""

import matplotlib

matplotlib.use("Agg")

import pandas as pd

from pybtls.output.plot import (
    plot_AE,
    plot_BM_S,
    plot_FR,
    plot_POT_S,
    plot_SV,
    plot_TH,
    plot_TS,
)
from pybtls.output.plot.single_vehicle import _lane_passes


def test_plot_AE_smoke(tmp_path):
    data = pd.DataFrame(
        {
            "Start Time": [0.0, 10.0, 20.0],
            "No. Vehicles": [1, 2, 1],
            "Effect 1": [12.5, 20.0, 8.0],
        }
    )
    save_to = tmp_path / "ae.png"

    plot_AE(data, save_to=save_to)

    assert save_to.exists()


def test_plot_BM_S_smoke(tmp_path):
    data = pd.DataFrame(
        {
            "Block Index": [1, 2, 3],
            "1-Truck Event": [10.0, 12.0, 8.0],
            "2-Truck Event": [15.0, None, 18.0],
        }
    )
    save_to = tmp_path / "bm_s.png"

    plot_BM_S(data, save_to=save_to)

    assert save_to.exists()


def test_plot_FR_smoke(tmp_path):
    data = pd.DataFrame(
        {
            "Amplitude": [1.0, 2.0, 3.0],
            "No. Cycles": [10, 5.5, 2],
        }
    )
    save_to = tmp_path / "fr.png"

    plot_FR(data, save_to=save_to)

    assert save_to.exists()


def test_plot_POT_S_smoke(tmp_path):
    data = pd.DataFrame(
        {
            "Peak Index": [1, 2, 3],
            "Time": [0.0, 10.0, 20.0],
            "No. Vehicles": [1, 1, 2],
            "Peak Value": [5.0, 15.0, 25.0],
        }
    )
    save_to = tmp_path / "pot_s.png"

    plot_POT_S(data, threshold=10.0, save_to=save_to)

    assert save_to.exists()


def test_plot_TH_single_row_does_not_crash(tmp_path):
    data = pd.DataFrame({"Time": [0.0], "No. Vehicles": [1], "Effect 1": [12.5]})
    save_to = tmp_path / "th_single_row.png"

    plot_TH(data, save_to=save_to)

    assert save_to.exists()


def test_plot_TS_smoke(tmp_path):
    data = pd.DataFrame(
        {
            "Hour": [1, 2, 3],
            "No. Vehicles": [40, 55, 30],
            "No. Trucks": [30, 45, 22],
            "No. Cars": [10, 10, 8],
            "2: Pattern 11": [12, 20, 9],
            "4: Pattern 12": [18, 25, 13],
        }
    )
    save_to = tmp_path / "ts.png"

    plot_TS(data, save_to=save_to)

    assert save_to.exists()


def test_plot_SV_smoke(tmp_path):
    # two lane passes: the time history holds rows only while the bridge is
    # loaded, so a jump in "Time" separates them
    frame = pd.DataFrame(
        {
            "Time": [0.0, 0.1, 0.2, 5.0, 5.1, 5.2],
            "No. Vehicles": [1] * 6,
            "Effect 1": [0.0, 10.0, 0.0, 0.0, 6.0, 0.0],
        }
    )
    save_to = tmp_path / "sv.png"

    plot_SV({"dir1": frame, "dir2": frame}, save_to=save_to)

    assert save_to.exists()


def test_plot_SV_splits_the_passes_on_the_time_gap():
    frame = pd.DataFrame(
        {
            "Time": [0.0, 0.1, 0.2, 5.0, 5.1],
            "No. Vehicles": [1] * 5,
            "Effect 1": [0.0, 10.0, 0.0, 0.0, 6.0],
        }
    )

    passes = _lane_passes(frame)

    assert [len(one_pass) for one_pass in passes] == [3, 2]
    # a zero effect inside a pass does not split it
    assert passes[0]["Effect 1"].tolist() == [0.0, 10.0, 0.0]
