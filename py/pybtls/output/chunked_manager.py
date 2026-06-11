"""
Read-time merged view over the per-chunk outputs of an auto-chunked
parallel simulation. Presents the same interface as ``_OutputManager``,
so downstream code does not need to know a simulation was chunked.
"""

from ._merge import (
    MERGE_REGISTRY,
    merge_bin_sum,
    merge_concat,
    merge_cumulative_stats,
    merge_rainflow,
    merge_vehicle_traffic,
)
from .output_manager import _OutputManager
from pathlib import Path
from typing import Union
import pandas as pd

__all__ = ["_ChunkedOutputManager"]

_SECS_PER_DAY = 86400.0


class _ChunkedOutputManager:
    def __init__(
        self,
        chunk_managers: list[_OutputManager],
        chunk_days: list[int],
        sim_tag: str,
        master_seed: int = None,
    ):
        """
        The merged view over one chunked simulation's outputs. \n
        Its instance should not be created by the user; ``Simulation``
        builds it when a simulation was added with ``no_chunk > 1``.

        Parameters
        ----------
        chunk_managers : list[_OutputManager]\n
            The per-chunk output managers, in chunk (time) order.\n
        chunk_days : list[int]\n
            The number of simulated days per chunk, in the same order.\n
        sim_tag : str\n
            The tag of the parent (chunked) simulation.\n
        master_seed : int, optional\n
            The master RNG seed (chunk i ran with master_seed + i).
        """

        if len(chunk_managers) != len(chunk_days):
            raise ValueError("chunk_managers and chunk_days must have equal length.")

        self._chunks = chunk_managers
        self._chunk_days = list(chunk_days)
        self._tag = sim_tag
        self._master_seed = master_seed

        # Start offset of each chunk on the merged timeline.
        self._day_offsets = [0]
        for days in self._chunk_days[:-1]:
            self._day_offsets.append(self._day_offsets[-1] + int(days))
        self._sec_offsets = [d * _SECS_PER_DAY for d in self._day_offsets]

    def get_summary(self, with_path: bool = False) -> Union[list, dict]:
        """
        Get to know the available outputs.

        Parameters
        ----------
        with_path : bool, optional\n
            Default is False.\n
            If True, the return will include the output file paths of
            every chunk.

        Returns
        -------
        Union[list,dict]\n
            The available outputs.
        """

        if not with_path:
            return self._chunks[0].get_summary()

        merged: dict[str, list] = {}
        for chunk in self._chunks:
            for key, paths in chunk.get_summary(with_path=True).items():
                merged.setdefault(key, []).extend(paths)
        return merged

    def read_data(self, key: str) -> dict[str, pd.DataFrame]:
        """
        Read and merge the data of all chunks, as if the simulation had
        been run in one piece.

        Parameters
        ----------
        key : str\n
            One of the output types listed by ``get_summary()``.

        Returns
        -------
        dict[str, pd.DataFrame]\n
            The merged data. The key is the file name without .txt.
        """

        spec = MERGE_REGISTRY[key]
        per_chunk = [chunk.read_data(key) for chunk in self._chunks]

        stems = set(per_chunk[0])
        for chunk_data in per_chunk[1:]:
            if set(chunk_data) != stems:
                raise RuntimeError(
                    f"Chunks produced different '{key}' file sets: "
                    f"{sorted(stems)} vs {sorted(chunk_data)}."
                )

        merged: dict[str, pd.DataFrame] = {}
        for stem in sorted(stems):
            frames = [chunk_data[stem] for chunk_data in per_chunk]
            if spec.category == "concat":
                merged[stem] = merge_concat(frames, spec, self._sec_offsets)
            elif spec.category == "bin_sum":
                merged[stem] = merge_bin_sum(frames, spec)
            elif spec.category == "moment_merge":
                merged[stem] = merge_cumulative_stats(frames)
            elif spec.category == "vehicles_concat":
                merged[stem] = merge_vehicle_traffic(frames, self._day_offsets)
            elif spec.category == "rainflow_splice":
                merged[stem] = self._merge_rainflow_stem(stem, frames, spec)
            else:  # pragma: no cover - registry and dispatch must stay in sync
                raise NotImplementedError(
                    f"No merge implementation for category '{spec.category}'."
                )
        return merged

    def _merge_rainflow_stem(
        self, stem: str, frames: list[pd.DataFrame], spec
    ) -> pd.DataFrame:
        """Exact residue splicing when the FRR_* sidecars exist; otherwise
        fall back to summing the per-chunk histograms."""

        residual_paths = [
            chunk._output_root / chunk._this_output_dir
            / (stem.replace("FR_", "FRR_", 1) + ".txt")
            for chunk in self._chunks
        ]
        if not all(path.is_file() for path in residual_paths):
            return merge_bin_sum(frames, spec)

        residual_seqs = []
        decimal, cutoff = None, None
        for path in residual_paths:
            with open(path, "r") as file:
                header = file.readline().split()
                decimal, cutoff = int(header[0]), float(header[1])
                residual_seqs.append([float(line) for line in file if line.strip()])

        return merge_rainflow(frames, residual_seqs, decimal, cutoff)

    def read_chunk_data(self, key: str) -> list[dict[str, pd.DataFrame]]:
        """
        Read the data of each chunk separately (no merging), e.g. for
        debugging or custom aggregation.

        Parameters
        ----------
        key : str\n
            One of the output types listed by ``get_summary()``.

        Returns
        -------
        list[dict[str, pd.DataFrame]]\n
            One dict per chunk, in chunk (time) order.
        """

        return [chunk.read_data(key) for chunk in self._chunks]

    def relocate(self, output_root: Path) -> None:
        """
        Relocate the output root directory.

        Parameters
        ----------
        output_root : Path\n
            The new root directory of all the output folders.
        """

        for chunk in self._chunks:
            chunk.relocate(output_root)

    @property
    def tag(self) -> str:
        return self._tag

    @property
    def master_seed(self) -> int:
        return self._master_seed

    @property
    def no_chunk(self) -> int:
        return len(self._chunks)

    @property
    def chunks(self) -> list[_OutputManager]:
        return self._chunks
