"""
Generated traffic: lane registration on TrafficGenerator, the nominal,
garage and Grave vehicle models, and the congested headway model.
"""

import numpy as np
import pytest

import pybtls as pb
from pybtls.lib import libbtls


def _lane_fixture(lane_index, lane_dir=1):
    """Build a minimal (lfc, vehicle_gen, headway_gen) triple for a lane."""

    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([100.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])

    lfc = pb.LaneFlowComposition(lane_index=lane_index, lane_dir=lane_dir)
    lfc.assign_lane_data(
        hourly_truck_flow=[80] * 24,
        hourly_car_flow=[20] * 24,
        hourly_speed_mean=[100.0] * 24,
        hourly_speed_std=[5.0] * 24,
        hourly_truck_composition=[[25.0, 25.0, 25.0, 25.0] for _ in range(24)],
    )

    vehicle_gen = pb.VehicleGenNominal(nominal_vehicle=vehicle, COV_list=[0.1, 0.1])
    headway_gen = pb.HeadwayGenConstant(constant_speed=36.0, constant_gap=5.0)

    return lfc, vehicle_gen, headway_gen


def test_traffic_generator_lane_order_independent_of_add_order():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc2, vg2, hg2 = _lane_fixture(lane_index=2)
    lfc1, vg1, hg1 = _lane_fixture(lane_index=1)

    # lane 2 added first, lane 1 added second
    tg.add_lane(vehicle_gen=vg2, headway_gen=hg2, lfc=lfc2)
    tg.add_lane(vehicle_gen=vg1, headway_gen=hg1, lfc=lfc1)

    assert tg._lanes[0].index == 1
    assert tg._lanes[1].index == 2

    # set_start_time (list form) must map by global lane_index, not call order
    tg.set_start_time([10.0, 20.0])
    assert tg._lanes[0].start_time == 10.0
    assert tg._lanes[1].start_time == 20.0


def test_traffic_generator_duplicate_lane_index_raises():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc1a, vg1a, hg1a = _lane_fixture(lane_index=1)
    lfc1b, vg1b, hg1b = _lane_fixture(lane_index=1)

    tg.add_lane(vehicle_gen=vg1a, headway_gen=hg1a, lfc=lfc1a)
    with pytest.raises(ValueError):
        tg.add_lane(vehicle_gen=vg1b, headway_gen=hg1b, lfc=lfc1b)


def test_traffic_generator_out_of_range_lane_index_raises():
    tg = pb.TrafficGenerator(no_lane=2)

    lfc3, vg3, hg3 = _lane_fixture(lane_index=3)
    with pytest.raises(ValueError):
        tg.add_lane(vehicle_gen=vg3, headway_gen=hg3, lfc=lfc3)


def test_traffic_generator_missing_lane_raises():
    tg = pb.TrafficGenerator(no_lane=2)
    lfc1, vg1, hg1 = _lane_fixture(lane_index=1)
    tg.add_lane(vehicle_gen=vg1, headway_gen=hg1, lfc=lfc1)

    with pytest.raises(ValueError, match="lane index"):
        tg._get_traffic_generator(20.0)


def test_garage_kernel_validation_rejects_wrong_number_of_sublists():
    with pytest.raises(ValueError):
        pb.VehicleGenGarage(garage=[], kernel=[[100.0, 10.0], [50.0, 5.0]])


def test_garage_kernel_validation_rejects_malformed_sublist():
    with pytest.raises(ValueError):
        pb.VehicleGenGarage(garage=[], kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1, 0.1]])


def test_garage_kernel_validation_accepts_valid_kernel():
    vehicle_gen = pb.VehicleGenGarage(
        garage=[], kernel=[[1.0, 0.1], [1.0, 0.1], [1.0, 0.1]]
    )
    assert vehicle_gen is not None


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
    tg.add_lane(
        vehicle_gen=vehicle_gen, headway_gen=headway_gen, lfc=_trucks_only_lfc()
    )
    libbtls.seed(seed)
    lane = tg._get_traffic_generator(50.0)[0]
    return [lane.getNextVehicle() for _ in range(n)]


# COV_list is [COV_axle_spacing, COV_axle_weight]: the C++ kernels used to be
# handed over in the opposite order (2026-09 audit, D2).
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


# congested_gap_coef_var is a coefficient of variation of the mean gap, not an
# absolute standard deviation in seconds (2026-09 audit, D3).
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


# Grave GVW per lane direction: CVehModelDataGrave::GetGVW picks the direction-1
# or direction-2 distribution, and the direction handed to it used to be an
# unset member (lost in the 2019 generator restructure), so every lane sampled
# the direction-2 distribution.
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
