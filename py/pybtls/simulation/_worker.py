"""
The simulations that ``Simulation.run`` executes, one per queued task.

A multi-core run executes ``run_task`` in worker processes, so everything
it needs is pickled: the task, which holds only the inputs of its own
simulation, and the function itself, which pickles by name. Nothing here
reads a ``Simulation``.
"""

from ..lib.BTLS import Vehicle, _VehClassPattern, _VehClassAxle, _VehicleBuffer
from ..bridge import Bridge
from ..traffic import TrafficGenerator, TrafficLoader
from ..output import OutputConfig, _OutputManager
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Union
import os
import pickle
import shutil


@dataclass(frozen=True)
class _SimTask:
    """
    One queued simulation: the validated arguments of ``Simulation.add_sim``,
    and the ``Simulation``'s ``overwrite`` setting.

    A traffic simulation has ``traffic``; a single-vehicle simulation has
    ``vehicle`` and ignores ``no_day``, ``output_config``, ``time_step`` and
    ``min_gvw``.
    """

    bridge: Optional[Bridge]
    traffic: Optional[Union[TrafficGenerator, TrafficLoader]]
    no_day: Optional[int]
    output_config: Optional[OutputConfig]
    time_step: float
    min_gvw: int
    vehicle: Optional[Vehicle]
    active_lane: Optional[list[int]]
    sim_tag: str
    overlap_avoid_distance: float
    track_progress: bool
    output_root: Path
    seed: Optional[int]
    engine: str
    overwrite: bool


def _make_sim_dir(sim_dir: Path, output_root: Path, overwrite: bool) -> None:
    """
    Create one simulation's output directory.

    With ``overwrite=False`` an existing directory is an error: reusing a tag
    would leave the previous run's files in place for ``_OutputManager`` to glob
    back as this run's. With ``overwrite=True`` the directory is replaced.
    """

    if overwrite and sim_dir.is_dir():
        # Only ever delete a directory strictly inside the output root, so a
        # stray tag (an absolute path, or one containing "..") cannot turn
        # overwrite=True into a recursive delete somewhere else.
        if output_root.resolve() not in sim_dir.resolve().parents:
            raise ValueError(
                f"Refusing to overwrite {sim_dir}: it is not inside the "
                f"simulation output directory {output_root}."
            )
        shutil.rmtree(sim_dir)

    os.makedirs(sim_dir, exist_ok=False)


def run_task(task: _SimTask) -> _OutputManager:
    """Run one queued simulation and return its output manager."""

    if task.traffic is not None and task.engine in ("cuda", "mps", "xpu"):
        from ..gpu import run as gpu_run

        return gpu_run(
            task.bridge,
            task.traffic,
            task.no_day,
            task.time_step,
            task.min_gvw,
            task.active_lane,
            task.sim_tag,
            task.overlap_avoid_distance,
            task.output_root,
            task.seed,
            device=task.engine,
            output_config=task.output_config,
            overwrite=task.overwrite,
        )
    if task.traffic is not None:
        return _run_traffic_sim(task)
    return _run_vehicle_sim(task)


def _run_vehicle_sim(task: _SimTask) -> _OutputManager:
    bridge = task.bridge
    sim_dir = task.output_root / str(task.sim_tag)
    _make_sim_dir(sim_dir, task.output_root, task.overwrite)

    if task.active_lane is None:
        lane_for_calc = list(range(1, bridge.no_lane + 1))  # 1-based global index
    else:
        lane_for_calc = task.active_lane  # 1-based global index

    no_dir = 2
    bridge_length = bridge.length

    # The run sets the vehicle's velocity, time, direction and lane, so
    # work on a copy and leave the caller's object as it was.
    vehicle = pickle.loads(pickle.dumps(task.vehicle))

    # Drive at the vehicle's own speed: the "centrifugal" load effect mode
    # scales with v^2. A vehicle whose velocity was never set keeps the
    # historical 1 m/s (a zero velocity would give infinite axle times).
    velocity = vehicle.get_velocity()
    if velocity <= 0.0:
        velocity = 1.0
        vehicle.set_velocity(velocity)

    vehicle_time_gap = 2 * (bridge_length + vehicle.get_length()) / velocity  # in s

    no_lane_dir_1 = [bridge.no_lane, 0]
    no_lane_dir_2 = [0, bridge.no_lane]

    output_config = OutputConfig()
    output_config.set_event_output(
        write_time_history=True, write_each_event=True
    )  # set output

    for i in range(no_dir):
        current_time = 0.0
        vehicle.set_time(current_time)

        dir_path = sim_dir / ("dir" + str(i + 1))
        os.mkdir(dir_path)
        output_config._Output.OUTPUT_DIR = str(dir_path)
        output_config._setRoad(bridge.no_lane, 1, no_lane_dir_1[i], no_lane_dir_2[i])
        load_calc = bridge._get_bridge(
            output_config
        )  # vehicle drive from one dirn then another

        load_calc.initializeDataMgr(current_time, len(lane_for_calc) * vehicle_time_gap)
        load_calc.setCalcTimeStep(0.01 / velocity)  # 1 cm of travel per step

        for j, lane_index in enumerate(lane_for_calc):
            next_arrival_time = (j + 1) * vehicle_time_gap
            vehicle.set_time(current_time)

            vehicle.set_direction(i + 1)
            vehicle.set_local_from_global_lane(lane_index, bridge.no_lane)
            load_calc.addVehicle(vehicle)
            load_calc.update(next_arrival_time, current_time)

            current_time = next_arrival_time

        load_calc.finish()

    return _OutputManager(task.output_root, task.sim_tag, None)


def _run_traffic_sim(task: _SimTask) -> _OutputManager:
    bridge, traffic = task.bridge, task.traffic

    if task.seed is not None:
        from ..lib import libbtls

        libbtls.seed(task.seed)

    sim_dir = task.output_root / str(task.sim_tag)
    _make_sim_dir(sim_dir, task.output_root, task.overwrite)

    if task.no_day is None:  # recorded traffic (add_sim requires it otherwise)
        no_day = traffic.sim_day
    else:
        no_day = int(task.no_day)

    # all C++ writers place their files under OUTPUT_DIR; work on a copy
    # so the caller's config object is not mutated
    output_config = pickle.loads(pickle.dumps(task.output_config))
    output_config._Output.OUTPUT_DIR = str(sim_dir)

    output_config._setRoad(
        traffic.no_lane,
        traffic.no_dir,
        traffic.no_lane_dir_1,
        traffic.no_lane_dir_2,
    )  # this info is used by VehBuffer and EventManager(in CBridge)

    if traffic.vehicle_classifier == 0:
        vehicle_classifier = _VehClassAxle()
    else:
        vehicle_classifier = _VehClassPattern()

    # generated traffic starts at t=0; recorded traffic keeps its own dates,
    # so its run starts at midnight of its first day (as in PrepareSim.cpp)
    current_time = traffic.start_time if isinstance(traffic, TrafficLoader) else 0.0
    end_time = current_time + no_day * 86400.0  # 24*3600

    vehicle_buffer = _VehicleBuffer(output_config, vehicle_classifier, current_time)
    bridge_length = task.overlap_avoid_distance  # magic number to avoid vehicle overlap

    if task.track_progress:
        current_day = int(current_time // 86400)
        sim_progress_print = ""
        print("Starting simulation...")
        print("Day complete...")

    if isinstance(bridge, Bridge):
        load_calc = bridge._get_bridge(output_config)
        load_calc.initializeDataMgr(current_time, end_time)
        load_calc.setCalcTimeStep(task.time_step)
        bridge_length = bridge.length

    if isinstance(traffic, TrafficGenerator):
        lane_list = traffic._get_traffic_generator(bridge_length)
    else:
        lane_list = traffic._get_traffic_loader()

    if task.active_lane is None:
        lane_for_calc = lane_list
    else:
        lane_for_calc = [lane_list[i - 1] for i in task.active_lane]

    if isinstance(traffic, TrafficLoader):
        # An initially-empty lane keeps CLane's default next-arrival time
        # (0.0), so it would sort first and end the merge loop on the
        # first iteration — exclude empty lanes from the calculation.
        # (add_sim has checked that some simulated lane has vehicles.)
        lane_for_calc = [lane for lane in lane_for_calc if lane.getNoVehicles() > 0]

    # The run is the set of vehicles arriving in [start, end]: each is
    # counted once and, if it loads the bridge, crossed to completion. The
    # bridge is advanced only to the arrivals of the vehicles put on it: an
    # event ends when the set of vehicles ON the bridge changes, and a
    # vehicle at or below min_gvw never joins it, so its arrival must not
    # cut the running event. bridge_time is the last such arrival, where
    # the bridge clock stands between updates. (The C++ program's
    # doSimulation in PrepareSim.cpp is the same loop.)
    bridge_time = current_time
    while True:
        lane_for_calc = sorted(lane_for_calc, key=lambda t: t.getNextArrivalTime())

        next_arrival_time = lane_for_calc[0].getNextArrivalTime()
        vehicle = lane_for_calc[0].getNextVehicle()

        # end of the recorded traffic, or the first arrival beyond the
        # window: neither is part of the run
        if vehicle is None or vehicle.get_time() > end_time:
            break

        vehicle_buffer.addVehicle(vehicle)
        if isinstance(bridge, Bridge) and vehicle.get_gvw() > int(task.min_gvw):
            # (BTLS requires min_gvw in size_t kN)
            load_calc.update(next_arrival_time, bridge_time)
            load_calc.addVehicle(vehicle)
            bridge_time = next_arrival_time

        current_time = vehicle.get_time()

        if task.track_progress:
            if current_time > 86400 * (current_day + 1):
                current_day += 1
                sim_progress_print += "\t" + str(current_day)
                if current_day % 10 == 0:
                    print(sim_progress_print)
                    sim_progress_print = ""

    if isinstance(bridge, Bridge):
        # run the bridge on until it empties: the last vehicles' crossings,
        # and the events they form after the end time, belong to the run
        load_calc.update(float("inf"), bridge_time)
        load_calc.finish()

    vehicle_buffer.flushBuffer(end_time)

    return _OutputManager(task.output_root, task.sim_tag, output_config)
