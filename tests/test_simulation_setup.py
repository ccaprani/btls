"""
Simulation set-up: the output directory (``overwrite``), ``add_sim``'s
keyword arguments and validation, and what ``run`` keeps when a simulation
fails.
"""

import dataclasses
import subprocess
import sys
import warnings

import pytest

import pybtls as pb
from pybtls._sim_worker import _SimTask
from pybtls.output.chunked_manager import _ChunkedOutputManager
from utils import loader_from_rows, make_il7_bridge


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
    from pybtls._sim_worker import _make_sim_dir

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
    sim.add_sim(**_loader_sim(), overlap_avoid_distance=42.0)

    assert sim._sim_argument[-1].overlap_avoid_distance == 42.0


@pytest.mark.parametrize("name", ["overlap_avoid_distanc", "min_chase_distance"])
def test_add_sim_rejects_unknown_keyword(tmp_path, name):
    sim = pb.Simulation(output_dir=tmp_path)
    with pytest.raises(TypeError, match=name):
        sim.add_sim(**{name: 42.0})


def test_add_sim_warns_when_bridge_length_overrides_overlap_avoid_distance(tmp_path):
    sim = pb.Simulation(output_dir=tmp_path)
    bridge = make_il7_bridge()

    with pytest.warns(UserWarning, match="overlap_avoid_distance"):
        sim.add_sim(**_loader_sim(bridge=bridge), overlap_avoid_distance=42.0)

    with warnings.catch_warnings():  # same value as the bridge: nothing to say
        warnings.simplefilter("error")
        sim.add_sim(**_loader_sim(bridge=bridge), overlap_avoid_distance=20.0)


def _loader_sim(**overrides):
    """add_sim arguments for a one-day recorded run on the IL7 bridge."""
    config = pb.OutputConfig()
    config.set_BM_output(write_summary=True)
    arguments = dict(
        bridge=make_il7_bridge(),
        traffic=loader_from_rows([(10.0, 200.0, 20.0)]),
        no_day=1,
        output_config=config,
        min_gvw=10,
    )
    arguments.update(overrides)
    return arguments


@pytest.mark.parametrize(
    "first, second", [("a", "a"), ("a", "a/b"), ("a/b", "a"), (None, "Sim_1")]
)
def test_add_sim_rejects_a_tag_whose_directory_clashes_with_a_queued_one(
    tmp_path, first, second
):
    sim = pb.Simulation(output_dir=tmp_path)
    sim.add_sim(**_loader_sim(), tag=first)
    with pytest.raises(ValueError, match="clashes"):
        sim.add_sim(**_loader_sim(), tag=second)
    assert sim.get_no_sim() == 1


@pytest.mark.parametrize("active_lane", [[0], [-1], [2], [1, 0], []])
def test_add_sim_rejects_an_active_lane_outside_the_road(tmp_path, active_lane):
    sim = pb.Simulation(output_dir=tmp_path)
    with pytest.raises(ValueError, match="active_lane"):
        sim.add_sim(**_loader_sim(), active_lane=active_lane)


@pytest.mark.parametrize(
    "overrides, error",
    [
        (dict(output_config=None), TypeError),
        (dict(bridge="not a bridge"), TypeError),
        (dict(traffic=None), ValueError),
        (dict(traffic=pb.TrafficLoader(1)), ValueError),
    ],
)
def test_add_sim_rejects_bad_arguments_before_overwrite_clears_anything(
    tmp_path, overrides, error
):
    sim = pb.Simulation(output_dir=tmp_path, overwrite=True)
    sim.add_sim(**_loader_sim(), tag="t")
    sim.run(no_core=1, show_progress=False)
    previous = sorted(p.name for p in (tmp_path / "t").iterdir())

    rerun = pb.Simulation(output_dir=tmp_path, overwrite=True)
    with pytest.raises(error):
        rerun.add_sim(**_loader_sim(**overrides), tag="t")
    assert sorted(p.name for p in (tmp_path / "t").iterdir()) == previous

    # a rejected add_sim does not use up a default tag
    rerun.add_sim(**_loader_sim())
    assert rerun._sim_argument[0].sim_tag == "Sim_1"


def test_a_failing_run_cancels_the_queue_and_keeps_the_finished_outputs(tmp_path):
    (tmp_path / "s01").mkdir()  # without overwrite, the second sim fails to start
    sim = pb.Simulation(output_dir=tmp_path)
    # ~0.4 s a simulation, so that one worker starting later than the other
    # cannot let the queue drain before the failure reaches run()
    rows = [(10.0 + 8.0 * i, 200.0, 20.0) for i in range(4000)]
    tags = [f"s{i:02d}" for i in range(30)]
    for tag in tags:
        sim.add_sim(
            **_loader_sim(traffic=loader_from_rows(rows), time_step=0.001), tag=tag
        )

    with pytest.raises(FileExistsError):
        sim.run(no_core=2, show_progress=False)

    ran = [tag for tag in tags if tag != "s01" and (tmp_path / tag).is_dir()]
    assert len(ran) < len(tags) // 2, f"the queue was not cancelled: {ran} ran"
    # every simulation that ran finished, and its output is kept
    assert list(sim.get_output()) == ran
    assert all(sim.get_output()[tag].get_summary() for tag in ran)


def test_collected_outputs_leave_out_a_chunked_sim_missing_a_chunk(tmp_path):
    sim = pb.Simulation(output_dir=tmp_path)
    tags = ["plain", "X/chunk_000", "X/chunk_001", "Y/chunk_000", "Y/chunk_001"]
    blank = {field.name: None for field in dataclasses.fields(_SimTask)}
    sim._sim_argument = [_SimTask(**{**blank, "sim_tag": tag}) for tag in tags]
    for parent in ("X", "Y"):
        sim._chunk_groups[parent] = {
            "chunk_tags": [f"{parent}/chunk_000", f"{parent}/chunk_001"],
            "chunk_days": [1, 1],
            "master_seed": 1,
        }

    sim._collect_outputs({0: "plain output", 1: "x0", 2: "x1", 3: "y0"})

    assert list(sim.get_output()) == ["plain", "X"]
    assert isinstance(sim.get_output()["X"], _ChunkedOutputManager)


def test_simulation_leaves_the_global_start_method_alone(tmp_path):
    script = (
        "import multiprocessing, pybtls\n"
        f"pybtls.Simulation(output_dir={str(tmp_path)!r})\n"
        "print(multiprocessing.get_start_method(allow_none=True))\n"
    )
    result = subprocess.run(
        [sys.executable, "-c", script], capture_output=True, text=True, check=True
    )
    assert result.stdout.strip() == "None"
