"""
Smoke tests for output/plot/* (plot_TH is already covered in
test_audit_fixes_output.py). Column names match the current read_*
counterparts, matching the actual on-disk column set produced after the
recent "No. Vehicles" rename in output/read/*.
"""

import matplotlib

matplotlib.use("Agg")

import pandas as pd

from pybtls.output.plot import plot_AE, plot_BM_S, plot_FR, plot_POT_S


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
