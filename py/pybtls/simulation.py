"""
The module that assembles everything together for the simulation. \n
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from .lib.BTLS import Vehicle, _VehClassPattern, _VehClassAxle, _VehicleBuffer
from .bridge import Bridge
from .traffic import TrafficGenerator, TrafficLoader
from .output import OutputConfig, _OutputManager
from .output.chunked_manager import _ChunkedOutputManager
from typing import Union
from pathlib import Path
import importlib.metadata as package_metadata
import multiprocessing
import os
import pickle
import random
import sys
import platform

__all__ = ["Simulation"]


class Simulation:
    def __init__(self, output_dir: Path = Path("./")):
        """
        This is the class for setting and running simulations.

        Parameters
        ----------
        output_dir : Path, optional\n
            The output directory for the simulation results. The default is "./".
        """

        try:
            multiprocessing.set_start_method("spawn")
        except:
            pass

        self._sim_count = 0
        self._sim_argument = []
        self._sim_output = {}
        self._chunk_groups = {}
        self._output_root = (
            Path(output_dir).resolve()
            if not isinstance(output_dir, Path)
            else output_dir.resolve()
        )
        self._write_version_info()

    def add_sim(
        self,
        bridge: Bridge = None,
        traffic: Union[TrafficGenerator, TrafficLoader] = None,
        no_day: int = None,
        output_config: OutputConfig = None,
        time_step: float = 0.1,
        min_gvw: int = 0,
        vehicle: Vehicle = None,
        active_lane: list[int] = None,
        tag: str = None,
        seed: int = None,
        no_chunk: int = None,
        **kwargs,
    ) -> None:
        """
        Add a simulation to the simulation queue.

        Parameters
        ----------
        bridge : Bridge, optional\n
            The bridge to be calculated load effect. If not provided, there will not be load effect calculation.

        traffic : Union[TrafficGenerator,TrafficLoader], optional\n
            The traffic can be either generated or recorded.

        no_day : int, optional\n
            The number of days to be simulated (in day). If not provided, the number of days will be the same as the recorded traffic (if given). A single-vehicle simulation will ignore this argument.

        output_config : OutputConfig, optional\n
            The output configuration. This argument is essential for traffic simulation. A single-vehicle simulation will ignore this argument.

        time_step : float, optional\n
            The calculation time step (in s) for the simulation (about precision). This argument is only used in load effect calculation for traffic simulation. A single-vehicle simulation will ignore this argument.

        min_gvw : int, optional\n
            The minimum gross vehicle weight (in kN) to be considered in the load effect calculation for traffic simulation. A single-vehicle simulation will ignore this argument.

        vehicle : Vehicle, optional\n
            The vehicle for a single-vehicle simulation.

        active_lane : list[int], optional\n
            The active bridge lanes during the simulation. The default is None, which means all lanes are active. [1-based global index].

        tag : str, optional\n
            The tag for the simulation. The default tag will be "Sim_{Simulation Order}".

        seed : int, optional\n
            Seed for the C++ random number generator. If provided,
            ``libbtls.seed(seed)`` is called at the start of this
            simulation, making the traffic generation deterministic.
            If None (default), the RNG keeps its current state
            (non-reproducible, same as the pre-seed-API behaviour).
            For a chunked simulation (``no_chunk > 1``) this acts as
            the master seed: chunk i runs with ``seed + i``.

        no_chunk : int, optional\n
            Split this simulation into ``no_chunk`` independent
            day-chunks that run in parallel across cores (each chunk
            gets its own RNG stream), then merge the outputs into a
            single result on ``get_output()``. The merged outputs are
            statistically equivalent to - and formatted identically
            to - a single sequential run. Requires a TrafficGenerator
            traffic and ``no_day`` divisible by ``no_chunk``; the
            chunk length must also align with the configured BM / POT
            block sizes and statistics intervals (validated here).
            Default is None (no chunking).

        Keyword Arguments
        -----------------
        overlap_avoid_distance : float, optional\n
            The minimum chase distance (in m) between two vehicles to avoid overlap (should equal to bridge length). If the bridge argument has an input then no need to specify this argument. The default is 100.0.

        track_progress : bool, optional\n
            Whether to track the simulation progress. A single-vehicle simulation will ignore this argument. The default is False.
        """

        self._sim_count += 1
        if tag is None:
            sim_tag = "Sim_" + str(self._sim_count)
        else:
            sim_tag = tag

        overlap_avoid_distance = kwargs.get("min_chase_distance", 100.0)
        track_progress = kwargs.get("track_progress", False)

        if no_chunk is None or no_chunk == 1:
            self._sim_argument.append(
                (
                    bridge,
                    traffic,
                    no_day,
                    output_config,
                    time_step,
                    min_gvw,
                    vehicle,
                    active_lane,
                    sim_tag,
                    overlap_avoid_distance,
                    track_progress,
                    self._output_root,
                    seed,
                )
            )
            return

        chunk_days = self._validate_chunking(traffic, vehicle, no_day, no_chunk, output_config)
        master_seed = seed if seed is not None else random.SystemRandom().randrange(1, 2**31)

        # Chunk runs keep their rainflow residuals open (written to FRR_*
        # sidecars) so the merged histogram can be spliced exactly. Copy the
        # config so the caller's object is not mutated.
        if output_config._Output.Fatigue.DO_FATIGUE_RAINFLOW:
            output_config = pickle.loads(pickle.dumps(output_config))
            output_config._Output.Fatigue.WRITE_RAINFLOW_RESIDUALS = True

        chunk_tags = []
        for i in range(no_chunk):
            chunk_tag = f"{sim_tag}/chunk_{i:03d}"
            chunk_tags.append(chunk_tag)
            self._sim_argument.append(
                (
                    bridge,
                    traffic,
                    chunk_days,
                    output_config,
                    time_step,
                    min_gvw,
                    vehicle,
                    active_lane,
                    chunk_tag,
                    overlap_avoid_distance,
                    track_progress,
                    self._output_root,
                    master_seed + i,
                )
            )

        self._chunk_groups[sim_tag] = {
            "chunk_tags": chunk_tags,
            "chunk_days": [chunk_days] * no_chunk,
            "master_seed": master_seed,
        }

    def _validate_chunking(
        self, traffic, vehicle, no_day, no_chunk, output_config
    ) -> int:
        """Validate a chunked add_sim request; return the days per chunk."""

        if vehicle is not None:
            raise ValueError("no_chunk does not apply to single-vehicle simulations.")
        if not isinstance(traffic, TrafficGenerator):
            raise ValueError(
                "no_chunk requires a TrafficGenerator traffic: recorded "
                "traffic (TrafficLoader) cannot be re-seeded per chunk."
            )
        if not isinstance(no_chunk, int) or no_chunk < 2:
            raise ValueError("no_chunk must be an integer >= 2.")
        if no_day is None:
            raise ValueError("no_chunk requires no_day to be given.")
        if no_day % no_chunk != 0:
            raise ValueError(
                f"no_day ({no_day}) must be divisible by no_chunk ({no_chunk}) "
                "so that chunks cover whole days."
            )
        if not isinstance(output_config, OutputConfig):
            raise TypeError("Argument output needs to be OutputConfig type.")

        chunk_days = int(no_day) // int(no_chunk)
        chunk_secs = chunk_days * 86400

        out = output_config._Output
        if out.BlockMax.WRITE_BM_VEHICLES or out.BlockMax.WRITE_BM_MIXED or out.BlockMax.WRITE_BM_SUMMARY:
            block_secs = out.BlockMax.BLOCK_SIZE_DAYS * 86400 + out.BlockMax.BLOCK_SIZE_SECS
            if block_secs == 0 or chunk_secs % block_secs != 0:
                raise ValueError(
                    f"Chunk length ({chunk_days} days) must be a multiple of "
                    f"the block-maximum block size ({block_secs} s) for the "
                    "merged result to equal a sequential run."
                )
        if out.POT.WRITE_POT_COUNTER:
            pot_secs = out.POT.POT_COUNT_SIZE_DAYS * 86400 + out.POT.POT_COUNT_SIZE_SECS
            if pot_secs == 0 or chunk_secs % pot_secs != 0:
                raise ValueError(
                    f"Chunk length ({chunk_days} days) must be a multiple of "
                    f"the POT counter block size ({pot_secs} s) for the "
                    "merged result to equal a sequential run."
                )
        if out.Stats.WRITE_SS_INTERVALS:
            interval = out.Stats.WRITE_SS_INTERVAL_SIZE
            if interval == 0 or chunk_secs % interval != 0:
                raise ValueError(
                    f"Chunk length ({chunk_days} days) must be a multiple of "
                    f"the statistics interval size ({interval} s) for the "
                    "merged result to equal a sequential run."
                )

        return chunk_days

    def run(self, no_core: int = None) -> None:
        """
        Run the simulations. \n

        Parameters
        ----------
        no_core : int, optional\n
            The number of cores to be used for multi-core running. \n
            If no_core is one or there is only one added simulation, the running will be single-core. \n
            Otherwise, the running will be multi-core. \n
            By default, (no_cpu_logic_core - 2) processes will be used for multi-core running.

        Returns
        -------
        None
        """

        if no_core == 1 or len(self._sim_argument) == 1:
            for sim_arg in self._sim_argument:
                self._sim_output[sim_arg[8]] = self._single_sim(sim_arg)
        else:
            no_processes = (
                no_core if no_core is not None else multiprocessing.cpu_count() - 2
            )
            with multiprocessing.Pool(processes=no_processes) as pool:
                temp = pool.map(self._single_sim, self._sim_argument)
            for i, sim_arg in enumerate(self._sim_argument):
                self._sim_output[sim_arg[8]] = temp[i]

        self._reduce_chunk_groups()

    def _reduce_chunk_groups(self) -> None:
        """Replace per-chunk outputs with one merged view per chunked sim."""

        for parent_tag, group in self._chunk_groups.items():
            chunk_managers = [
                self._sim_output.pop(chunk_tag) for chunk_tag in group["chunk_tags"]
            ]
            self._sim_output[parent_tag] = _ChunkedOutputManager(
                chunk_managers,
                group["chunk_days"],
                parent_tag,
                master_seed=group["master_seed"],
            )

    def get_output(self) -> dict[str, Union[_OutputManager, _ChunkedOutputManager]]:
        """
        Get the output manager for each simulation.

        Returns
        -------
        dict[str, Union[_OutputManager, _ChunkedOutputManager]]
            A dict storing the output manager for each simulation.\n
            The keys are the sim_tags. A simulation added with
            ``no_chunk > 1`` is represented by a single
            ``_ChunkedOutputManager`` that reads as one merged result.
        """

        return self._sim_output

    def get_no_sim(self) -> int:
        """
        Get the number of simulations.

        Returns
        -------
        int
            The number of simulations.
        """

        return self._sim_count

    def _single_sim(self, args: tuple) -> _OutputManager:
        (
            bridge,
            traffic,
            no_day,
            output_config,
            time_step,
            min_gvw,
            vehicle,
            active_lane,
            sim_tag,
            overlap_avoid_distance,
            track_progress,
            output_root,
            seed,
        ) = args

        if traffic is not None:
            return self._single_traffic_sim(
                bridge,
                traffic,
                no_day,
                output_config,
                time_step,
                min_gvw,
                active_lane,
                sim_tag,
                overlap_avoid_distance,
                track_progress,
                output_root,
                seed,
            )
        elif vehicle is not None:
            return self._single_vehicle_sim(
                bridge, vehicle, active_lane, sim_tag, output_root
            )
        else:
            raise ValueError("Either traffic or vehicle should be provided.")

    def _single_vehicle_sim(
        self, bridge, vehicle, active_lane, sim_tag, output_root
    ) -> _OutputManager:
        os.makedirs(output_root / str(sim_tag), exist_ok=False)

        if not isinstance(bridge, Bridge):
            raise TypeError("Argument bridge needs to be Bridge type.")

        if not isinstance(vehicle, Vehicle):
            raise TypeError("Argument vehicle needs to be Vehicle type.")

        if active_lane is None:
            lane_for_calc = list(range(1, bridge.no_lane + 1))  # 1-based global index
        else:
            if not isinstance(active_lane, list):
                raise TypeError("Argument active_lane needs to be a list.")
            if max(active_lane) > bridge.no_lane:
                raise ValueError(
                    "The maximum lane index in active_lane is larger than the number of lanes on the bridge."
                )
            lane_for_calc = active_lane  # 1-based global index

        no_dir = 2
        bridge_length = bridge.length
        vehicle_time_gap = 2 * (bridge_length + vehicle.get_length()) / 1.0  # in s

        vehicle.set_velocity(1.0)  # 1 m/s

        no_lane_dir_1 = [bridge.no_lane, 0]
        no_lane_dir_2 = [0, bridge.no_lane]

        output_config = OutputConfig()
        output_config.set_event_output(
            write_time_history=True, write_each_event=True
        )  # set output

        for i in range(no_dir):
            current_time = 0.0
            vehicle.set_time(current_time)

            dir_path = output_root / str(sim_tag) / ("dir" + str(i + 1))
            os.mkdir(dir_path)
            output_config._Output.OUTPUT_DIR = str(dir_path)
            output_config._setRoad(
                bridge.no_lane, 1, no_lane_dir_1[i], no_lane_dir_2[i]
            )
            load_calc = bridge._get_bridge(
                output_config
            )  # vehicle drive from one dirn then another

            load_calc.initializeDataMgr(current_time)
            load_calc.setCalcTimeStep(0.01)

            for j, lane_index in enumerate(lane_for_calc):
                next_arrival_time = (j + 1) * vehicle_time_gap
                vehicle.set_time(current_time)

                vehicle.set_direction(i + 1)
                vehicle.set_local_from_global_lane(lane_index, bridge.no_lane)
                load_calc.addVehicle(vehicle)
                load_calc.update(next_arrival_time, current_time)

                current_time = next_arrival_time

            load_calc.finish()

        return _OutputManager(output_root, sim_tag, None)

    def _single_traffic_sim(
        self,
        bridge,
        traffic,
        no_day,
        output_config,
        time_step,
        min_gvw,
        active_lane,
        sim_tag,
        overlap_avoid_distance,
        track_progress,
        output_root,
        seed=None,
    ) -> _OutputManager:
        if seed is not None:
            from .lib import libbtls
            libbtls.seed(seed)

        sim_dir = output_root / str(sim_tag)
        os.makedirs(sim_dir, exist_ok=False)

        if isinstance(traffic, TrafficGenerator) and no_day is None:
            raise ValueError("Argument no_day is not given.")
        elif isinstance(traffic, TrafficLoader) and no_day is None:
            no_day = traffic.sim_day
        else:
            no_day = int(no_day)

        if not isinstance(output_config, OutputConfig):
            raise TypeError("Argument output needs to be OutputConfig type.")

        # all C++ writers place their files under OUTPUT_DIR; work on a copy
        # so the caller's config object is not mutated
        output_config = pickle.loads(pickle.dumps(output_config))
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

        current_time = 0.0
        end_time = no_day * 86400.0  # 24*3600

        vehicle_buffer = _VehicleBuffer(output_config, vehicle_classifier, current_time)
        bridge_length = overlap_avoid_distance  # magic number, to avoid vehicle overlap

        if track_progress:
            current_day = 0
            sim_progress_print = ""
            print("Starting simulation...")
            print("Day complete...")

        if isinstance(bridge, Bridge):
            load_calc = bridge._get_bridge(output_config)
            if bridge.no_lane != traffic.no_lane:
                raise RuntimeError(
                    "The number of lanes in the bridge and traffic generator are not equal."
                )
            load_calc.initializeDataMgr(current_time)
            load_calc.setCalcTimeStep(time_step)
            bridge_length = bridge.length

        if isinstance(traffic, TrafficGenerator):
            lane_list = traffic._get_traffic_generator(bridge_length)
        elif isinstance(traffic, TrafficLoader):
            lane_list = traffic._get_traffic_loader()
        else:
            raise TypeError(
                "traffic should be either TrafficGenerator or TrafficLoader."
            )

        if active_lane is None:
            lane_for_calc = lane_list
        else:
            if not isinstance(active_lane, list):
                raise TypeError("Argument active_lane needs to be a list.")
            if max(active_lane) > traffic.no_lane:
                raise ValueError(
                    "The maximum lane index in active_lane is larger than the number of lanes on the bridge."
                )
            lane_for_calc = [lane_list[i - 1] for i in active_lane]

        while current_time <= end_time:
            lane_for_calc = sorted(lane_for_calc, key=lambda t: t.getNextArrivalTime())

            next_arrival_time = lane_for_calc[0].getNextArrivalTime()
            vehicle = lane_for_calc[0].getNextVehicle()

            if (
                vehicle is None
            ):  # This is to skip the empty (NoneType) vehicle at the end of Read&Sim
                break

            vehicle_buffer.addVehicle(vehicle)
            if isinstance(bridge, Bridge):
                load_calc.update(next_arrival_time, current_time)
                if vehicle.get_gvw() > int(min_gvw):  # BTLS requires in size_t kN
                    load_calc.addVehicle(vehicle)

            current_time = vehicle.get_time()

            if track_progress:
                if current_time > 86400 * (current_day + 1):
                    current_day += 1
                    sim_progress_print += "\t" + str(current_day)
                    if current_day % 10 == 0:
                        print(sim_progress_print)
                        sim_progress_print = ""

        if isinstance(bridge, Bridge):
            load_calc.finish(end_time)

        vehicle_buffer.flushBuffer(end_time)

        return _OutputManager(output_root, sim_tag, output_config)

    def _write_version_info(self):
        """
        This method writes the Python and pybtls version information to the output directory.
        """

        os.makedirs(self._output_root, exist_ok=True)
        version_file_path = self._output_root / "sim_version_info.txt"

        pybtls_version = package_metadata.version("pybtls")
        python_version = sys.version
        cpu_architecture = platform.machine()

        with open(version_file_path, "w") as version_file:
            version_file.write(f"PyBTLS Version: {pybtls_version}\n")
            version_file.write(f"Python Version: {python_version}\n")
            version_file.write(f"CPU Architecture: {cpu_architecture}\n")
