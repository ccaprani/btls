"""
Regression test for the Grave vehicle generator's per-direction GVW selection.

CVehModelDataGrave::GetGVW(dir, nAxles) picks the direction-1 or direction-2
GVW distribution, but the direction handed to it by CVehicleGenGrave was a
member that no constructor ever wrote (lost in the 2019 generator restructure,
commit 1374270). Every lane therefore sampled the direction-2 distribution.
"""

import numpy as np
import pytest

import pybtls as pb
from pybtls.lib import libbtls

# Mixture means (kg/100) of the 5-axle GVW distribution per direction in
# data/GraveParameters/Auxerre/GVWpdf.csv: sum(weight_i * mean_i).
AUXERRE_5AXLE_GVW_MEAN = {1: 394.8, 2: 434.0}
KN_PER_KG100 = 0.981  # the generator converts kg/100 to kN with this factor


def _five_axle_lfc(lane_index, lane_dir):
    lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
    lfc.assign_lane_data(
        hourly_truck_flow=[500] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[250.0] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[0.0, 0.0, 0.0, 100.0]] * 24,
    )
    return lfc


def _gvw_samples_by_direction(n_per_lane=4000, seed=7):
    """One lane per direction on the same generator; GVW samples in kg/100."""
    tg = pb.TrafficGenerator(no_lane=2)
    for lane_index, lane_dir in ((1, 1), (2, 2)):
        tg.add_lane(
            vehicle_gen=pb.VehicleGenGrave(traffic_site="Auxerre"),
            headway_gen=pb.HeadwayGenFreeflow(),
            lfc=_five_axle_lfc(lane_index, lane_dir),
        )
    libbtls.seed(seed)
    lanes = tg._get_traffic_generator(50.0)
    return {
        lane_dir: np.array([lane.getNextVehicle().get_gvw() for _ in range(n_per_lane)])
        / KN_PER_KG100
        for lane_dir, lane in zip((1, 2), lanes)
    }


def test_grave_gvw_follows_lane_direction():
    samples = _gvw_samples_by_direction()

    for lane_dir, expected in AUXERRE_5AXLE_GVW_MEAN.items():
        assert samples[lane_dir].mean() == pytest.approx(expected, rel=0.015), (
            f"direction {lane_dir}: mean GVW {samples[lane_dir].mean():.1f} kg/100, "
            f"expected ~{expected}"
        )

    # the two directions must not be drawing from the same distribution
    assert abs(samples[1].mean() - samples[2].mean()) > 20.0
