"""
Tests for the centrifugal / braking load-effect modes (ported from the
pybtls_centrifugal_braking branch) and the Vehicle acceleration property.
"""

import pickle

import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder

GRAVITY = 9.80665
SPEED = 1.0  # m/s - the single-vehicle simulation drives at 1 m/s


def _run_single_vehicle(
    out_dir: Path, mode: str, braking_factor: float = 0.0, acceleration: float = 0.0
) -> float:
    """Run the single-vehicle simulation and return the peak of effect 1."""
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    if mode == "braking":
        inf_line.set_mode(mode, braking_factor=braking_factor)
    elif mode != "vertical":
        inf_line.set_mode(mode)

    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=0.0)

    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([100.0, 100.0])
    vehicle.set_axle_spacings([5.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])
    if acceleration:
        vehicle.set_acceleration(acceleration)

    sim = pb.Simulation(output_dir=out_dir)
    sim.add_sim(
        bridge=bridge,
        vehicle=vehicle,
        tag=mode + str(braking_factor) + str(acceleration),
    )
    sim.run(no_core=1)

    output = next(iter(sim.get_output().values()))
    history = output.read_data("time_history")
    peak = max(df["Effect 1"].abs().max() for df in history.values())
    assert peak > 0.0
    return peak


@pytest.fixture(scope="module")
def peaks():
    root = Path(__file__).parent / "temp_le_modes"
    remove_folder(root)
    values = {
        "vertical": _run_single_vehicle(root / "v", "vertical"),
        "centrifugal": _run_single_vehicle(root / "c", "centrifugal"),
        "braking_fallback": _run_single_vehicle(
            root / "bf", "braking", braking_factor=0.3
        ),
        "braking_accel": _run_single_vehicle(root / "ba", "braking", acceleration=-2.0),
    }
    yield values
    remove_folder(root)


def test_centrifugal_scales_with_v_squared_over_g(peaks):
    expected = SPEED**2 / GRAVITY
    assert peaks["centrifugal"] / peaks["vertical"] == pytest.approx(expected, rel=1e-4)


def test_braking_fallback_factor(peaks):
    assert peaks["braking_fallback"] / peaks["vertical"] == pytest.approx(0.3, rel=1e-4)


def test_braking_uses_vehicle_acceleration(peaks):
    expected = 2.0 / GRAVITY
    assert peaks["braking_accel"] / peaks["vertical"] == pytest.approx(
        expected, rel=1e-4
    )


def test_surface_centrifugal_mode():
    # set_mode on an InfluenceSurface must reach the C++ load calculation
    # through Bridge's internal wrapping.
    root = Path(__file__).parent / "temp_le_surface"
    remove_folder(root)

    lane_position = [(0.5, 4.0), (4.0, 7.5)]
    IS_matrix = [
        [0.0, 0.0, 8.0],
        [0.0, 0.0, 0.0],
        [10.0, 5.0, 5.0],
        [20.0, 0.0, 0.0],
    ]

    peaks = {}
    for mode in ("vertical", "centrifugal"):
        inf_surf = pb.InfluenceSurface()
        inf_surf.set_IS(IS_matrix, lane_position)
        if mode != "vertical":
            inf_surf.set_mode(mode)

        bridge = pb.Bridge(length=20.0, no_lane=2)
        bridge.add_load_effect(inf_line_surf=inf_surf, threshold=0.0)

        vehicle = pb.Vehicle(no_axles=2)
        vehicle.set_axle_weights([100.0, 100.0])
        vehicle.set_axle_spacings([5.0, 0.0])
        vehicle.set_axle_widths([2.0, 2.0])

        sim = pb.Simulation(output_dir=root / mode)
        sim.add_sim(bridge=bridge, vehicle=vehicle, tag=mode)
        sim.run(no_core=1)
        output = next(iter(sim.get_output().values()))
        history = output.read_data("time_history")
        peaks[mode] = max(df["Effect 1"].abs().max() for df in history.values())

    assert peaks["vertical"] > 0.0
    assert peaks["centrifugal"] / peaks["vertical"] == pytest.approx(
        SPEED**2 / GRAVITY, rel=1e-4
    )
    remove_folder(root)


def test_invalid_mode_raises():
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    with pytest.raises(ValueError):
        inf_line.set_mode("sideways")


@pytest.mark.parametrize("bad", [-0.3, float("nan"), float("inf"), "0.3", None])
def test_invalid_braking_factor_raises(bad):
    # braking_factor is a magnitude: the travel-direction sign is the engine's
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    with pytest.raises(ValueError):
        inf_line.set_mode("braking", braking_factor=bad)


def test_braking_factor_ignored_outside_braking_mode_warns():
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    with pytest.warns(UserWarning, match="only used by 'braking' mode"):
        inf_line.set_mode("centrifugal", braking_factor=0.3)


def test_mode_is_captured_per_load_effect():
    # The mode is snapshotted by add_load_effect, like the weight and threshold:
    # one influence line added twice carries a different mode for each effect,
    # and a later set_mode() does not reach back into the effect already added.
    root = Path(__file__).parent / "temp_le_per_effect"
    remove_folder(root)

    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)

    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=0.0)  # vertical
    inf_line.set_mode("centrifugal")
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=0.0)  # centrifugal

    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([100.0, 100.0])
    vehicle.set_axle_spacings([5.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])

    sim = pb.Simulation(output_dir=root)
    sim.add_sim(bridge=bridge, vehicle=vehicle, tag="per_effect")
    sim.run(no_core=1)

    history = next(iter(sim.get_output().values())).read_data("time_history")
    peak_1 = max(df["Effect 1"].abs().max() for df in history.values())
    peak_2 = max(df["Effect 2"].abs().max() for df in history.values())

    assert peak_1 > 0.0
    assert peak_2 / peak_1 == pytest.approx(SPEED**2 / GRAVITY, rel=1e-4)
    remove_folder(root)


def test_vehicle_acceleration_survives_pickle():
    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([80.0, 90.0])
    vehicle.set_axle_spacings([4.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])
    vehicle.set_acceleration(-3.5)

    clone = pickle.loads(pickle.dumps(vehicle))
    assert clone.get_acceleration() == pytest.approx(-3.5)
    assert clone.get_no_axles() == 2
