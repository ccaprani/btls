"""
Multi-core (spawn pool) runs must produce the same results as single-core
runs. The load-effect stream was always process-independent; the regression
pinned here is the vehicle CLASSIFICATION, which lives outside the Vehicle
property tuple and used to be dropped by pickling — so TrafficLoader sims
run in a worker process counted every vehicle as a car in the flow /
statistics outputs (FlowData, SS_C, SS_S).
"""

import pickle
from pathlib import Path

import pybtls as pb
from utils import remove_folder

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
