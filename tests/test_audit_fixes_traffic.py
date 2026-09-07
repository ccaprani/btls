"""
Regression tests for the 2026-09 traffic-generation / loading audit.

D2 - CVehModelDataNominal::getKernels returned the axle-spacing and
     axle-weight kernels swapped (declaration and call site used the
     opposite parameter order to the definition), so COV_AS acted on axle
     weights and COV_AW on axle spacings.
D3 - CFlowModelDataCongested used CONGESTED_GAP_COEF_VAR as an absolute
     standard deviation in seconds instead of multiplying it by the mean gap.
D4 - TrafficLoader.add_traffic(use_average_speed=True) was forwarded to C++
     with the master UseConstSpeed switch off, so it silently did nothing.
"""

from pathlib import Path

import numpy as np
import pytest

import pybtls as pb
from pybtls.lib import libbtls

TRAFFIC_FILE = Path(__file__).parent / "test_data/test_traffic_file.txt"  # MON, 4 lanes


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


# --- D3: congested gap coefficient of variation ------------------------------


@pytest.mark.parametrize("spacing_m", [20.0, 40.0])
def test_congested_gap_std_scales_with_mean_gap(spacing_m):
    speed_kmh, cov = 36.0, 0.25
    mean_gap = spacing_m / (speed_kmh / 3.6)  # CONGESTED_GAP, in seconds
    vehicles = _generate(
        pb.VehicleGenNominal(nominal_vehicle=_nominal_truck(), COV_list=[0.0, 0.0]),
        pb.HeadwayGenCongested(
            congested_spacing=spacing_m,
            congested_speed=speed_kmh,
            congested_gap_coef_var=cov,
        ),
        n=4000,
        seed=5,
    )
    # headway = (constant vehicle passage time) + Normal(mean_gap, mean_gap * cov)
    headways = np.diff([v.get_time() for v in vehicles])

    assert headways.std() == pytest.approx(cov * mean_gap, rel=0.1)


# --- D4: loader speed overrides ----------------------------------------------


def _loaded_speeds(**speed_kwargs):
    loader = pb.TrafficLoader(no_lane=4)
    loader.add_traffic(traffic=TRAFFIC_FILE, traffic_format=4, **speed_kwargs)
    return np.array([v.get_velocity() for lane in loader._lanes_vehicles for v in lane])


def test_loader_use_average_speed_applies_file_mean_to_all_vehicles():
    recorded = _loaded_speeds()
    assert len(np.unique(recorded)) > 1  # the file really has varying speeds

    averaged = _loaded_speeds(use_average_speed=True)

    assert len(np.unique(averaged)) == 1
    assert averaged[0] == pytest.approx(recorded.mean())


def test_loader_use_const_speed_applies_given_value():
    speeds = _loaded_speeds(use_const_speed=True, const_speed_value=54.0)

    assert np.all(speeds == pytest.approx(54.0 / 3.6))
