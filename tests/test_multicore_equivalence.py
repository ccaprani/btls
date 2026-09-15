"""
Multi-core (spawn pool) runs must produce the same results as single-core
runs. The load-effect stream was always process-independent; the regression
pinned here is the vehicle CLASSIFICATION, which lives outside the Vehicle
property tuple and used to be dropped by pickling — so TrafficLoader sims
run in a worker process counted every vehicle as a car in the flow /
statistics outputs (FlowData, SS_C, SS_S). Also pinned: what reaches a worker
is only the task's own simulation, and a config keeps every field there.
"""

import pickle
import threading
from pathlib import Path

import pybtls as pb
from utils import loader_from_rows, make_il7_bridge, remove_folder

TRAFFIC = Path(__file__).parent / "test_data/test_traffic_file.txt"
ROOT = Path(__file__).parent / "temp_multicore"

# files whose content depends on vehicle classification
STAT_FILES = ("SS_C_20.txt", "FlowData_1_1.txt", "FlowData_2_3.txt")


def _loader():
    ld = pb.TrafficLoader(no_lane=4)
    ld.add_traffic(
        traffic=TRAFFIC,
        traffic_format=4,
        use_average_speed=False,
        use_const_speed=False,
    )
    return ld


def _bridge():
    il = pb.InfluenceLine(IL_type="built-in")
    il.set_IL(id=1, length=20.0)
    b = pb.Bridge(length=20.0, no_lane=4)
    b.add_load_effect(inf_line_surf=il, threshold=5.0)
    return b


def _config():
    cfg = pb.OutputConfig()
    cfg.set_BM_output(write_summary=True)
    cfg.set_stats_output(write_flow_stats=True, write_overall=True)
    return cfg


def _add(sim, tag):
    sim.add_sim(
        bridge=_bridge(),
        traffic=_loader(),
        output_config=_config(),
        time_step=0.1,
        min_gvw=35,
        tag=tag,
        seed=7,
        engine="cpu",
    )


def test_vehicle_pickle_preserves_classification():
    # The pickle state carries the classification alongside the property
    # tuple; a roundtrip must be lossless (the class drives IsCar() and all
    # truck counting).
    vehicles = [v for lane in _loader()._lanes_vehicles for v in lane]
    states = [v.__getstate__() for v in vehicles]
    assert any(s[1] != 0 for s in states), "fixture must contain classified trucks"
    truck = next(v for v, s in zip(vehicles, states) if s[1] != 0)
    clone = pickle.loads(pickle.dumps(truck))
    assert clone.__getstate__() == truck.__getstate__()


def test_loader_stats_identical_across_cores():
    # A TrafficLoader sim run in a spawn worker (no_core > 1) must write the
    # same statistics files as the in-process (no_core = 1) run; also the
    # get_output() keys keep add_sim order under the unordered pool.
    remove_folder(ROOT)

    ref = pb.Simulation(output_dir=ROOT)
    _add(ref, "ref")
    ref.run(no_core=1, show_progress=False)

    pooled = pb.Simulation(output_dir=ROOT)
    for tag in ("m0", "m1"):
        _add(pooled, tag)
    pooled.run(no_core=2, show_progress=False)

    assert list(pooled.get_output()) == ["m0", "m1"]
    for name in STAT_FILES:
        ref_txt = (ROOT / "ref" / name).read_text()
        for tag in ("m0", "m1"):
            assert (
                ROOT / tag / name
            ).read_text() == ref_txt, (
                f"{name} differs between no_core=1 and pooled {tag}"
            )
    remove_folder(ROOT)


def test_a_task_carries_only_its_own_simulation(tmp_path):
    # A task sends a worker its own sim's arguments, not the Simulation: that
    # would copy every queued sim's bridge and traffic into every task. The
    # lock stands for anything on the Simulation that cannot be pickled.
    sim = pb.Simulation(output_dir=tmp_path)
    for tag in ("a", "b"):
        cfg = pb.OutputConfig()
        cfg.set_BM_output(write_summary=True)
        sim.add_sim(
            bridge=make_il7_bridge(),
            traffic=loader_from_rows([(10.0, 200.0, 20.0)]),
            no_day=1,
            output_config=cfg,
            time_step=0.1,
            min_gvw=10,
            tag=tag,
        )
    sim._lock = threading.Lock()
    sim.run(no_core=2, show_progress=False)
    assert all(out.read_data("BM_summary") for out in sim.get_output().values())


def _bound_fields(obj, prefix=""):
    """(path, value) of every field a config struct binds to Python."""
    for name in dir(type(obj)):
        if isinstance(getattr(type(obj), name), property):
            value = getattr(obj, name)
            if isinstance(value, (bool, int, float, str)):
                yield prefix + name, value
            else:
                yield from _bound_fields(value, prefix + name + ".")


def _attribute(obj, path):
    for name in path.split("."):
        obj = getattr(obj, name)
    return obj


def test_config_pickle_keeps_every_field_python_can_set():
    # The pickle state is built from the same table as the bindings, so a field
    # Python can set survives the trip to a worker (six used to reset to their
    # defaults, NO_OVERLAP_LENGTH and HEADWAY_MODEL among them)
    config = pb.OutputConfig()
    changed = {}
    for path, value in _bound_fields(config):
        if isinstance(value, bool):
            new = not value
        elif isinstance(value, str):
            new = value + "_changed"
        else:
            new = value + 3
        *parents, leaf = path.split(".")
        setattr(_attribute(config, ".".join(parents)) if parents else config, leaf, new)
        changed[path] = new
    assert "_Gen.NO_OVERLAP_LENGTH" in changed and len(changed) > 50

    clone = pickle.loads(pickle.dumps(config))
    assert {path: _attribute(clone, path) for path in changed} == changed


def test_config_state_without_a_newer_field_keeps_its_default():
    # a pickle or save_output manifest written before a field existed still loads
    state = pb.OutputConfig().__getstate__()
    state["Output"]["BlockMax"]["BLOCK_SIZE_DAYS"] = 7
    del state["Gen"]["NO_OVERLAP_LENGTH"]
    del state["Output"]["Fatigue"]["WRITE_RAINFLOW_RESIDUALS"]

    config = pb.OutputConfig.__new__(pb.OutputConfig)
    config.__setstate__(state)
    assert config._Output.BlockMax.BLOCK_SIZE_DAYS == 7
    assert config._Gen.NO_OVERLAP_LENGTH == pb.OutputConfig()._Gen.NO_OVERLAP_LENGTH
    assert config._Output.Fatigue.WRITE_RAINFLOW_RESIDUALS is False
