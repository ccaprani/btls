import pycba as cba
import pybtls as pb
from pathlib import Path

__all__ = [
    "remove_folder",
    "get_cba_IL",
    "make_vehicle",
    "make_il7_bridge",
    "loader_from_rows",
    "run_loader_cpu",
    "run_loader_torch",
    "read_records",
    "write_mon_traffic",
    "make_4lane_bridge",
]


def remove_folder(folder):
    path = Path(folder)
    if path.exists() and path.is_dir():
        for child in path.iterdir():
            if child.is_file():
                child.unlink()
            else:
                remove_folder(child)
        path.rmdir()


def get_cba_IL(
    beam_string: str, beam_position: float, beam_LE: str, resolution: float = 0.1
):

    (L, EI, R, eType) = cba.parse_beam_string(beam_string)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=resolution)
    x, eta = ils.get_il(beam_position, beam_LE)

    return x, eta


# --- a small recorded-traffic scenario shared by the end-of-run, chunk-merge
# and flow-statistics tests: a 20 m single-lane bridge with built-in influence
# line 7, crossed by 2-axle vehicles given as (time, weight[, speed]) rows ----


def make_vehicle(t, weight, speed=1.0):
    """A 2-axle vehicle of ``weight`` kN, 1 m axle spacing, arriving at ``t`` s."""
    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([weight / 2] * 2)
    vehicle.set_axle_spacings([1.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])
    vehicle.set_time(t)
    vehicle.set_velocity(speed)
    return vehicle


def make_il7_bridge():
    bridge = pb.Bridge(length=20.0, no_lane=1)
    il = pb.InfluenceLine("built-in")
    il.set_IL(id=7, length=20.0)
    bridge.add_load_effect(il, threshold=1.0)
    return bridge


def loader_from_rows(rows):
    traffic = pb.TrafficLoader(1)
    traffic.add_traffic([make_vehicle(*row) for row in rows])
    return traffic


def run_loader_cpu(root, tag, rows, cfg, min_gvw=10):
    """One simulated day of ``rows`` on the C++ engine; returns its output manager."""
    sim = pb.Simulation(root, overwrite=True)
    sim.add_sim(
        make_il7_bridge(),
        loader_from_rows(rows),
        no_day=1,
        output_config=cfg,
        time_step=0.1,
        min_gvw=min_gvw,
        tag=tag,
    )
    sim.run(show_progress=False)
    return sim.get_output()[tag]


def run_loader_torch(root, tag, rows, cfg, min_gvw=10):
    """The same day on the GPU engine's torch-CPU backend (needs PyTorch, no CUDA)."""
    from pybtls.gpu.runner import run

    return run(
        make_il7_bridge(),
        loader_from_rows(rows),
        1,
        0.1,
        min_gvw,
        None,
        tag,
        20.0,
        root,
        None,
        device="cpu",
        output_config=cfg,
        overwrite=True,
    )


def read_records(manager, key):
    """``read_data(key)`` as plain records, for equality checks between engines."""
    return {stem: df.to_dict("records") for stem, df in manager.read_data(key).items()}


# --- recorded traffic with its dates moved: the first records of the 4-lane MON
# test traffic file, which starts on BTLS day 0 (1/1/2010), dated another year --


def write_mon_traffic(path, n_records, year):
    """Write the first ``n_records`` of the MON test traffic file to ``path``,
    dated ``year`` instead of 2010 (the MON year field, columns 14-17): MON
    years count from 2010, so 2019 starts the traffic on BTLS day 9*250."""
    source = Path(__file__).parent / "test_data/test_traffic_file.txt"
    lines = source.read_text().splitlines()[:n_records]
    path.write_text("".join(f"{line[:13]}{year:4d}{line[17:]}\n" for line in lines))
    return path


def make_4lane_bridge():
    """A 20 m, 4-lane bridge for the MON test traffic: built-in influence line 1
    on every lane, POT threshold 500."""
    il = pb.InfluenceLine(IL_type="built-in")
    il.set_IL(id=1, length=20.0)
    bridge = pb.Bridge(length=20.0, no_lane=4)
    bridge.add_load_effect(inf_line_surf=il, inf_weight=[1.0] * 4, threshold=500.0)
    return bridge
