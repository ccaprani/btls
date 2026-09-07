"""
Regression tests for the 2026-09 traffic-generation / loading audit.

D2 - CVehModelDataNominal::getKernels returned the axle-spacing and
     axle-weight kernels swapped (declaration and call site used the
     opposite parameter order to the definition), so COV_AS acted on axle
     weights and COV_AW on axle spacings.
"""

import numpy as np
import pytest

import pybtls as pb
from pybtls.lib import libbtls


def _trucks_only_lfc():
    lfc = pb.LaneFlowComposition(lane_index=1, lane_dir=1)
    lfc.assign_lane_data(
        hourly_truck_flow=[500] * 24,
        hourly_car_flow=[0] * 24,
        hourly_speed_mean=[250.0] * 24,
        hourly_speed_std=[10.0] * 24,
        hourly_truck_composition=[[100.0, 0.0, 0.0, 0.0]] * 24,
    )
    return lfc


def _nominal_truck():
    vehicle = pb.Vehicle(no_axles=3)
    vehicle.set_axle_weights([60.0, 100.0, 100.0])
    vehicle.set_axle_spacings([4.0, 1.3, 0.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])
    return vehicle


def _generate(vehicle_gen, headway_gen, n, seed):
    tg = pb.TrafficGenerator(no_lane=1)
    tg.add_lane(vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=_trucks_only_lfc())
    libbtls.seed(seed)
    lane = tg._get_traffic_generator(50.0)[0]
    return [lane.getNextVehicle() for _ in range(n)]


# --- D2: nominal-vehicle kernels ---------------------------------------------


def test_nominal_cov_list_order_is_spacing_then_weight():
    cov_as, cov_aw = 0.30, 0.01
    vehicles = _generate(
        pb.VehicleGenNominal(
            nominal_vehicle=_nominal_truck(), COV_list=[cov_as, cov_aw], kernel_type=0
        ),
        pb.HeadwayGenFreeflow(),
        n=3000,
        seed=11,
    )
    spacing = np.array([v.get_axle_spacing(0) for v in vehicles])
    weight = np.array([v.get_axle_weight(0) for v in vehicles])

    assert spacing.std() / spacing.mean() == pytest.approx(cov_as, rel=0.15)
    assert weight.std() / weight.mean() == pytest.approx(cov_aw, rel=0.15)
