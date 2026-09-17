"""
The module that assembles everything together for the simulation. \n
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
The simulations themselves are run by ``_worker``.
"""

from ..lib.BTLS import Vehicle
from ..bridge import Bridge
from ..traffic import TrafficGenerator, TrafficLoader
from ..output import OutputConfig, _OutputManager
from ..output.chunked_manager import _ChunkedOutputManager
from typing import Union
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
import dataclasses
import importlib.metadata as package_metadata
import multiprocessing
import numbers
import numpy as np
import os
import pickle
import random
import sys
import time
import platform
import warnings
from .._kwargs import reject_unknown_kwargs
from ._worker import _SimTask, run_task

__all__ = ["Simulation"]


def _fmt_duration(seconds: float) -> str:
    if seconds >= 3600:
        return f"{int(seconds // 3600)}h{int(seconds % 3600 // 60):02d}m"
    if seconds >= 60:
        return f"{int(seconds // 60)}m{int(seconds % 60):02d}s"
    return f"{seconds:.0f}s"


def _derive_chunk_seed(master_seed: int, index: int) -> int:
    """Mix (master_seed, index) into a chunk seed, so that runs with nearby
    master seeds do not replay each other's traffic streams."""

    seq = np.random.SeedSequence([master_seed, index])
    return int(seq.generate_state(1, dtype=np.uint32)[0])


class Simulation:
    """Assembles bridges, traffic, and output settings into runnable simulations, optionally parallelised across cores."""

    def __init__(self, output_dir: Path = Path("./"), overwrite: bool = False):
        """
        This is the class for setting and running simulations.

        Parameters
        ----------
        output_dir : Path, optional\n
            The output directory for the simulation results. The default is "./".

        overwrite : bool, optional\n
            Whether to replace a simulation's output directory if one already
            exists for that tag. The default is False, which raises
            ``FileExistsError``: reusing a tag would otherwise leave the previous
            run's files in place, and they would be read back as this run's
            results. Pass True to re-run a script over its own output, as a
            demo or notebook typically wants to.
        """

        self._sim_count = 0
        self._sim_argument = []
        self._sim_output = {}
        self._chunk_groups = {}
        self._overwrite = bool(overwrite)
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
            The number of days to be simulated (in day). If not provided, the number of days will be the same as the recorded traffic (if given). Recorded traffic is replayed from midnight of its first vehicle's day, so the days count from there and the outputs keep the traffic's own dates. A single-vehicle simulation will ignore this argument.

        output_config : OutputConfig, optional\n
            The output configuration. This argument is essential for traffic simulation. A single-vehicle simulation will ignore this argument.

        time_step : float, optional\n
            The calculation time step (in s) for the simulation (about precision). This argument is only used in load effect calculation for traffic simulation. A single-vehicle simulation will ignore this argument.

        min_gvw : int, optional\n
            The minimum gross vehicle weight (in kN) to be considered in the
            load effect calculation for traffic simulation. A vehicle at or
            below it never enters the bridge: it is still written to the
            vehicle file and counted in the flow statistics, but it does not
            load the bridge and its arrival does not end an event. A
            single-vehicle simulation will ignore this argument.

        vehicle : Vehicle, optional\n
            The vehicle for a single-vehicle simulation. It drives at its own
            velocity (1 m/s if the velocity was never set), which matters for
            the speed-dependent "centrifugal" load effect mode.

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
            the master seed: each chunk's seed is derived from it by
            mixing ``(seed, chunk index)``, so that runs whose master
            seeds are close do not share traffic streams.

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
            The chunks run in spawned processes, so the calling
            script must guard its entry point with
            ``if __name__ == "__main__":``.
            Default is None (no chunking).

        Keyword Arguments
        -----------------
        overlap_avoid_distance : float, optional\n
            The minimum chase distance (in m) between two vehicles to avoid overlap (should equal to bridge length). If the bridge argument has an input then the bridge length is used instead, and a different explicit value triggers a warning. The default is 100.0.

        track_progress : bool, optional\n
            Whether to track the simulation progress. A single-vehicle simulation will ignore this argument. The default is False.

        engine : str, optional\n
            Load-effect engine for traffic simulation. Default "cpu".

            Each value names the device the engine runs on:

            - "cpu": the C++ time-stepping engine, on the **CPU**. Full-featured
              (time history, block maxima, POT, statistics, fatigue/rainflow) and
              the right choice for essentially all runs.
            - "cuda" / "mps" / "xpu": the experimental GPU engine (PyTorch +
              Triton). The engine name IS the torch device:

                * "cuda" -> **NVIDIA**, and **AMD** via ROCm (both use torch's
                  cuda device); float64.
                * "mps"  -> **Apple Silicon** (Metal); runs in float32 (MPS has
                  no float64). The MPS backend may lack ``searchsorted`` /
                  ``scatter_reduce`` — the engine probes for this and raises a
                  clear error; set ``PYTORCH_ENABLE_MPS_FALLBACK=1`` to run those
                  ops on the CPU (slower).
                * "xpu"  -> **Intel** GPU; float64.

              Only "cuda" is currently tested; "mps"/"xpu" are wired but
              unverified (the engine probes each backend for the ops it needs
              and errors clearly if one is missing). Needs ``pybtls[gpu]`` (a build of torch for that
              device). It computes per-effect block-maxima (BM),
              peaks-over-threshold (POT: PT_S/PT_C/PT_V), fatigue rainflow
              (FR), load-effect statistics (SS_C/SS_S), vehicle flow statistics
              (FlowData) and time history (TH) via
              per-vehicle superposition, honouring the matching OutputConfig
              flags. POT/SS rebuild the "cpu" event
              partition, so event / vehicle / truck counts track "cpu" to ~1%
              (uniform sampling merges composition changes inside one time step);
              peak values/times, statistics and fatigue cycle amplitudes carry
              uniform-grid sampling noise. Scope: recorded or generated traffic;
              discrete, built-in or surface influence lines, including a distinct
              IL/weight per lane; vertical / centrifugal modes. It does
              NOT produce the per-event / per-vehicle detail outputs
              (write_each_event, the vehicle file, BM-vehicle / mixed,
              write_fatigue_event); those are skipped with a
              warning — use engine="cpu" for them.

            When is the GPU engine worth it? Only when the load-effect *computation*
            dominates the run — which it usually does NOT. Profiling shows the
            per-step load summation is typically ~15-20% of wall-clock; the
            bottleneck is the output writers (time history, POT, fatigue) plus
            per-event overhead, none of which the GPU engine accelerates.
            Measured against ONE CPU core (RTX 3090 vs Ryzen 9 7950X, float64):
            ~2.5-5x for typical free-flow runs, ~10x for a compute-dominated
            case (long-span congested bridge; the gap grows with the number of
            load effects and of axles simultaneously on the deck). Note that
            for generated traffic, CPU chunk-parallelism (``no_chunk``) scales
            near-linearly across cores and often matches or beats the GPU — the
            GPU engine's clear wins are recorded traffic (which cannot chunk),
            runs needing exact sequential equivalence, and many-effect
            congested/long-span cases. The device working set is tiled
            adaptively to the free VRAM, so small or shared GPUs shrink the
            tile instead of running out of memory. For ordinary short-span
            bridges with a handful of effects, or any run needing the full
            output set, use "cpu".
        """

        reject_unknown_kwargs(
            "add_sim", kwargs, ("overlap_avoid_distance", "track_progress", "engine")
        )

        overlap_avoid_distance = kwargs.get("overlap_avoid_distance", 100.0)
        if (
            "overlap_avoid_distance" in kwargs
            and bridge is not None
            and overlap_avoid_distance != bridge.length
        ):
            warnings.warn(
                f"overlap_avoid_distance={overlap_avoid_distance} is ignored because a "
                f"bridge is given; its length ({bridge.length} m) is used instead.",
                stacklevel=2,
            )
        track_progress = kwargs.get("track_progress", False)
        engine = kwargs.get("engine", "cpu")
        if engine not in ("cpu", "cuda", "mps", "xpu"):
            raise ValueError('engine must be "cpu", "cuda", "mps" or "xpu".')

        # BTLS compares the gross vehicle weight against a size_t in kN, so the
        # CPU path truncates min_gvw while the GPU path compares it as a float.
        # Normalise here instead, so both engines use the same threshold.
        if int(min_gvw) != min_gvw:
            raise ValueError(
                f"min_gvw is a whole number of kN, got {min_gvw!r}. The C++ "
                "engine truncates it, so a fractional threshold would mean "
                "different things on the two engines."
            )
        min_gvw = int(min_gvw)

        # validate the whole request now, so a bad one fails here rather than
        # in a worker at run(), after overwrite has already cleared its directory
        sim_tag = tag if tag is not None else "Sim_" + str(self._sim_count + 1)
        self._validate_tag(sim_tag)
        self._validate_sim(
            bridge, traffic, vehicle, no_day, output_config, active_lane, engine
        )

        task = _SimTask(
            bridge=bridge,
            traffic=traffic,
            no_day=no_day,
            output_config=output_config,
            time_step=time_step,
            min_gvw=min_gvw,
            vehicle=vehicle,
            active_lane=active_lane,
            sim_tag=sim_tag,
            overlap_avoid_distance=overlap_avoid_distance,
            track_progress=track_progress,
            output_root=self._output_root,
            seed=seed,
            engine=engine,
            overwrite=self._overwrite,
        )
        if no_chunk is None or no_chunk == 1:
            self._sim_count += 1
            self._sim_argument.append(task)
            return

        chunk_days = self._validate_chunking(
            traffic, vehicle, no_day, no_chunk, output_config
        )
        self._sim_count += 1
        master_seed = (
            seed if seed is not None else random.SystemRandom().randrange(1, 2**31)
        )

        # Chunk runs keep their rainflow residuals open (written to FRR_*
        # sidecars) so the merged histogram can be spliced exactly. Copy the
        # config so the caller's object is not mutated.
        if output_config._Output.Fatigue.DO_FATIGUE_RAINFLOW:
            output_config = pickle.loads(pickle.dumps(output_config))
            output_config._Output.Fatigue.WRITE_RAINFLOW_RESIDUALS = True

        # each chunk is the same simulation over its own days, tag and seed
        chunk_tags = []
        chunk_seeds = []
        for i in range(no_chunk):
            chunk_tag = f"{sim_tag}/chunk_{i:03d}"
            chunk_tags.append(chunk_tag)
            chunk_seeds.append(_derive_chunk_seed(master_seed, i))
            self._sim_argument.append(
                dataclasses.replace(
                    task,
                    no_day=chunk_days,
                    output_config=output_config,
                    sim_tag=chunk_tag,
                    seed=chunk_seeds[i],
                )
            )

        self._chunk_groups[sim_tag] = {
            "chunk_tags": chunk_tags,
            "chunk_days": [chunk_days] * no_chunk,
            "master_seed": master_seed,
            "chunk_seeds": chunk_seeds,
        }

    def _validate_tag(self, sim_tag) -> None:
        """Refuse a tag whose output directory is, contains or lies inside that
        of a queued simulation: one run would clear or mix the other's files."""

        parts = Path(str(sim_tag)).parts
        queued_tags = [task.sim_tag for task in self._sim_argument]
        for queued in queued_tags + list(self._chunk_groups):
            queued_parts = Path(str(queued)).parts
            common = min(len(parts), len(queued_parts))
            if parts[:common] == queued_parts[:common]:
                raise ValueError(
                    f"Tag {sim_tag!r} clashes with the queued simulation "
                    f"{queued!r}: their output directories coincide or nest."
                )

    @staticmethod
    def _validate_sim(
        bridge, traffic, vehicle, no_day, output_config, active_lane, engine
    ):
        """Validate the arguments of a traffic or single-vehicle simulation."""

        if bridge is not None and not isinstance(bridge, Bridge):
            raise TypeError("Argument bridge needs to be Bridge type.")

        if traffic is not None:
            if not isinstance(traffic, (TrafficGenerator, TrafficLoader)):
                raise TypeError(
                    "traffic should be either TrafficGenerator or TrafficLoader."
                )
            if isinstance(traffic, TrafficGenerator) and no_day is None:
                raise ValueError("Argument no_day is not given.")
            # the GPU engine defaults a missing config to the BM summary
            if (engine == "cpu" or output_config is not None) and not isinstance(
                output_config, OutputConfig
            ):
                raise TypeError("Argument output needs to be OutputConfig type.")
            if bridge is not None and bridge.no_lane != traffic.no_lane:
                raise RuntimeError(
                    "The number of lanes in the bridge and traffic generator are not equal."
                )
            no_lane = traffic.no_lane
        elif vehicle is not None:
            if bridge is None:
                raise TypeError("Argument bridge needs to be Bridge type.")
            if not isinstance(vehicle, Vehicle):
                raise TypeError("Argument vehicle needs to be Vehicle type.")
            no_lane = bridge.no_lane
        else:
            raise ValueError("Either traffic or vehicle should be provided.")

        if active_lane is not None:
            if not isinstance(active_lane, list):
                raise TypeError("Argument active_lane needs to be a list.")
            if not active_lane or not all(
                isinstance(lane, numbers.Integral) and 1 <= lane <= no_lane
                for lane in active_lane
            ):
                raise ValueError(
                    f"active_lane must be a non-empty list of lane indices from 1 "
                    f"to {no_lane} (1-based), got {active_lane!r}."
                )

        if isinstance(traffic, TrafficLoader):
            lanes = traffic._lanes_vehicles
            if active_lane is not None:
                lanes = [lanes[lane - 1] for lane in active_lane]
            if not any(lanes):
                raise ValueError("No vehicles in any simulated lane.")

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
        if any(lane is not None and lane.start_time != 0.0 for lane in traffic._lanes):
            raise ValueError(
                "no_chunk requires a zero lane start time: every chunk starts "
                "its traffic that late, so the merged result would have a gap "
                "of that length at each chunk boundary."
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

        chunk_days = int(no_day) // int(no_chunk)
        chunk_secs = chunk_days * 86400

        out = output_config._Output
        if (
            out.BlockMax.WRITE_BM_VEHICLES
            or out.BlockMax.WRITE_BM_MIXED
            or out.BlockMax.WRITE_BM_SUMMARY
        ):
            block_secs = (
                out.BlockMax.BLOCK_SIZE_DAYS * 86400 + out.BlockMax.BLOCK_SIZE_SECS
            )
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

    def run(self, no_core: int = None, show_progress: bool = True) -> None:
        """
        Run the simulations. \n

        Parameters
        ----------
        no_core : int, optional\n
            The number of cores to be used for multi-core running. \n
            If no_core is one or there is only one added simulation, the running will be single-core. \n
            Otherwise, the running will be multi-core. \n
            By default, max(1, no_cpu_logic_core - 2) processes will be used for multi-core running. \n
            Multi-core running spawns worker processes, so the calling script must guard its entry point with ``if __name__ == "__main__":``.

        show_progress : bool, optional\n
            Print one line as each simulation (or chunk) completes, with
            elapsed time and an ETA. Only the main process prints, so the
            lines do not interleave. Default is True; nothing is printed
            when there is only one task.

        Returns
        -------
        None
        """

        if no_core is not None and no_core < 1:
            raise ValueError("no_core must be >= 1.")

        # GPU engines share one device: running tasks concurrently serialises the
        # device compute (no speed-up) while each worker process replicates its
        # window in host RAM + VRAM, so multi-core only multiplies memory and can
        # OOM. Warn so the default no_core (cpu_count-2) isn't applied to GPU runs.
        effective_cores = (
            no_core if no_core is not None else max(1, multiprocessing.cpu_count() - 2)
        )
        gpu_tasks = sum(
            1 for task in self._sim_argument if task.engine in ("cuda", "mps", "xpu")
        )
        total = len(self._sim_argument)
        if gpu_tasks and effective_cores > 1 and total > 1:
            print(
                f"Warning: {gpu_tasks} GPU task(s) queued with no_core="
                f"{effective_cores}. GPU tasks share one device — concurrency gives "
                "no speed-up, but each process replicates its window in host RAM + "
                "VRAM (risking OOM). Use no_core=1 for GPU runs.",
                file=sys.stderr,
                flush=True,
            )

        start = time.perf_counter()
        done = 0

        def report(index: int) -> None:
            nonlocal done
            done += 1
            if not show_progress or total < 2:
                return
            elapsed = time.perf_counter() - start
            eta = elapsed / done * (total - done)
            width = len(str(total))
            print(
                f"[{done:>{width}}/{total}] {self._sim_argument[index].sim_tag} done, "
                f"elapsed {_fmt_duration(elapsed)}, ETA ~{_fmt_duration(eta)}",
                flush=True,
            )

        results = {}
        try:
            if no_core == 1 or total == 1:
                for i, task in enumerate(self._sim_argument):
                    results[i] = run_task(task)
                    report(i)
            else:
                # An explicit spawn context, not the (mutable) global default:
                # fork workers would break CUDA re-initialisation.
                # ProcessPoolExecutor rather than multiprocessing.Pool because a
                # worker that dies (segfault, OOM kill, a C++ exit()) then raises
                # BrokenProcessPool instead of blocking run() forever.
                ctx = multiprocessing.get_context("spawn")
                with ProcessPoolExecutor(
                    max_workers=effective_cores, mp_context=ctx
                ) as executor:
                    # run_task pickles by name and a task holds only its own
                    # simulation's inputs; a Simulation method would pickle the
                    # whole Simulation, every queued sim's bridge and traffic
                    # included, into each task
                    futures = {
                        executor.submit(run_task, task): i
                        for i, task in enumerate(self._sim_argument)
                    }
                    try:
                        for future in as_completed(futures):
                            i = futures[future]
                            results[i] = future.result()
                            report(i)
                    except BaseException:
                        # a failure (or Ctrl+C) cancels the queued simulations;
                        # the running ones cannot be stopped, so wait for them
                        # and keep those that finish
                        executor.shutdown(cancel_futures=True)
                        for future, i in futures.items():
                            if (
                                i not in results
                                and not future.cancelled()
                                and future.exception() is None
                            ):
                                results[i] = future.result()
                        raise
        finally:
            self._collect_outputs(results)

    def _collect_outputs(self, results: dict) -> None:
        """
        Store the outputs of this run's finished simulations, keyed by index in
        the queue, with one merged view per chunked simulation. A simulation
        that did not finish (the run failed or was interrupted) has no output,
        and neither has a chunked simulation missing any of its chunks.
        """

        # insert in add_sim order so get_output() keys are deterministic
        for i, task in enumerate(self._sim_argument):
            if i in results:
                self._sim_output[task.sim_tag] = results[i]
            else:  # drop what an earlier run() left under this tag
                self._sim_output.pop(task.sim_tag, None)

        for parent_tag, group in self._chunk_groups.items():
            chunk_managers = [
                self._sim_output.pop(chunk_tag, None)
                for chunk_tag in group["chunk_tags"]
            ]
            if any(manager is None for manager in chunk_managers):
                self._sim_output.pop(parent_tag, None)
                continue
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
