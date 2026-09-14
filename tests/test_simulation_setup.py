"""
Simulation set-up: the output directory (``overwrite``) and ``add_sim``'s
keyword arguments.
"""

import warnings

import pytest

import pybtls as pb


def test_simulation_overwrite_default_false_still_raises(tmp_path):
    """Re-running a tag without overwrite must stay an error: the previous
    run's files would otherwise be globbed back as this run's results."""
    _run_single_vehicle_sim(tmp_path, overwrite=False)
    with pytest.raises(FileExistsError):
        _run_single_vehicle_sim(tmp_path, overwrite=False)


def test_simulation_overwrite_replaces_previous_output(tmp_path):
    _run_single_vehicle_sim(tmp_path, overwrite=False)
    stale = tmp_path / "ow" / "stale_from_previous_run.txt"
    stale.write_text("stale")
    _run_single_vehicle_sim(tmp_path, overwrite=True)
    assert not stale.exists(), "overwrite=True left the previous run's files behind"


def test_make_sim_dir_refuses_to_delete_outside_the_output_root(tmp_path):
    """overwrite=True must never turn a stray tag into a recursive delete
    somewhere outside the simulation output directory."""
    from pybtls.simulation import _make_sim_dir

    root = tmp_path / "out"
    root.mkdir()
    outside = tmp_path / "precious"
    outside.mkdir()
    (outside / "data.txt").write_text("do not delete")

    with pytest.raises(ValueError):
        _make_sim_dir(outside, root, True)
    assert (outside / "data.txt").exists()


def _run_single_vehicle_sim(root, overwrite):
    inf_line = pb.InfluenceLine(IL_type="built-in")
    inf_line.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=1)
    bridge.add_load_effect(inf_line_surf=inf_line, threshold=0.0)

    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([100.0, 100.0])
    vehicle.set_axle_spacings([5.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])

    sim = pb.Simulation(output_dir=root, overwrite=overwrite)
    sim.add_sim(bridge=bridge, vehicle=vehicle, tag="ow")
    sim.run(no_core=1, show_progress=False)


def test_add_sim_reads_overlap_avoid_distance(tmp_path):
    sim = pb.Simulation(output_dir=tmp_path)
    sim.add_sim(overlap_avoid_distance=42.0)

    assert sim._sim_argument[-1][9] == 42.0


@pytest.mark.parametrize("name", ["overlap_avoid_distanc", "min_chase_distance"])
def test_add_sim_rejects_unknown_keyword(tmp_path, name):
    sim = pb.Simulation(output_dir=tmp_path)
    with pytest.raises(TypeError, match=name):
        sim.add_sim(**{name: 42.0})


def test_add_sim_warns_when_bridge_length_overrides_overlap_avoid_distance(tmp_path):
    sim = pb.Simulation(output_dir=tmp_path)
    bridge = pb.Bridge(length=20.0, no_lane=1)

    with pytest.warns(UserWarning, match="overlap_avoid_distance"):
        sim.add_sim(bridge=bridge, overlap_avoid_distance=42.0)

    with warnings.catch_warnings():  # same value as the bridge: nothing to say
        warnings.simplefilter("error")
        sim.add_sim(bridge=bridge, overlap_avoid_distance=20.0)
