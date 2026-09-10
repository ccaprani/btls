"""
Persistence for simulation output managers.

The on-disk format is a JSON manifest: an output manager is only a set of
file paths plus the output configuration, so a human-readable manifest is
robust across pybtls/pandas versions (the simulation data itself always
lives in the output text files). Legacy pickle files written by earlier
versions are still loadable.
"""

from ..output.output_manager import _OutputManager
from ..output.chunked_manager import _ChunkedOutputManager
from ..output.output_config import OutputConfig
from pathlib import Path
from typing import Union
import json
import pickle
import warnings

__all__ = ["save_output", "load_output"]

_FORMAT = "pybtls-output-manifest"
_FORMAT_VERSION = 1


def _manager_to_record(manager) -> dict:
    if isinstance(manager, _ChunkedOutputManager):
        return {
            "type": "chunked",
            "sim_tag": manager.tag,
            "master_seed": manager.master_seed,
            "chunk_days": list(manager._chunk_days),
            "chunks": [_manager_to_record(chunk) for chunk in manager.chunks],
        }
    return {
        "type": "single",
        "output_root": str(manager._output_root),
        "sim_tag": manager._this_output_dir,
        "config": (
            dict(manager._output_config.__getstate__())
            if manager._output_config is not None
            else None
        ),
    }


def _record_to_manager(record: dict):
    if record["type"] == "chunked":
        chunks = [_record_to_manager(rec) for rec in record["chunks"]]
        return _ChunkedOutputManager(
            chunks,
            record["chunk_days"],
            record["sim_tag"],
            master_seed=record["master_seed"],
        )
    config = None
    if record["config"] is not None:
        config = OutputConfig.__new__(OutputConfig)
        config.__setstate__(record["config"])
    output_dir = Path(record["output_root"]) / record["sim_tag"]
    if not output_dir.is_dir():
        # the manifest stores the directory, not the file list, so a missing
        # tree would otherwise load as an empty but "available" result set
        warnings.warn(
            f"Output directory {output_dir} does not exist; the loaded output "
            "manager will report no results. Use its relocate() method to point "
            "it at the moved output root."
        )
    return _OutputManager(Path(record["output_root"]), record["sim_tag"], config)


def save_output(
    output: dict[str, Union[_OutputManager, _ChunkedOutputManager]],
    file_path: Path,
) -> None:
    """
    Save the output managers to a JSON manifest file.

    The manifest records the output directory and configuration of each
    manager (the simulation data itself stays in the output text files), so
    it remains readable across pybtls versions. The output files themselves
    are re-discovered on loading, so the output tree must still be in place.
    Use ``load_output`` to restore.

    Parameters
    ----------
    output : dict[str, Union[_OutputManager, _ChunkedOutputManager]]\n
        The output managers to save, e.g. from ``Simulation.get_output()``.\n
        The keys are the sim_tags.\n
    file_path : Path\n
        The path of the manifest file (".json" recommended).
    """

    if not all(
        isinstance(obj, (_OutputManager, _ChunkedOutputManager))
        for obj in output.values()
    ):
        raise ValueError("All values in the output dictionary must be output managers.")
    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path
    file_path.parent.mkdir(parents=True, exist_ok=True)

    manifest = {
        "format": _FORMAT,
        "version": _FORMAT_VERSION,
        "outputs": {tag: _manager_to_record(mgr) for tag, mgr in output.items()},
    }
    with open(file_path, "w") as file:
        json.dump(manifest, file, indent=1)

    print(f"Outputs have been successfully saved to {file_path}!")


def load_output(
    file_path: Path,
) -> dict[str, Union[_OutputManager, _ChunkedOutputManager]]:
    """
    Load output managers from a manifest file.

    Reads the JSON manifest written by ``save_output``; legacy binary
    .pkl files written by earlier pybtls versions are also accepted.

    Parameters
    ----------
    file_path : Path\n
        The path of the manifest file.

    Returns
    -------
    dict[str, Union[_OutputManager, _ChunkedOutputManager]]\n
        The output managers, keyed by sim_tag.
    """

    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path

    try:
        with open(file_path, "r") as file:
            manifest = json.load(file)
    except (UnicodeDecodeError, json.JSONDecodeError):
        # legacy pickle format
        warnings.warn(
            "Loading a legacy pickle output file; re-save it with "
            "save_output() to convert it to the JSON manifest format.",
            stacklevel=2,
        )
        with open(file_path, "rb") as file:
            output = pickle.load(file)
        if not all(
            isinstance(obj, (_OutputManager, _ChunkedOutputManager))
            for obj in output.values()
        ):
            raise RuntimeError("The output pkl is damaged.")
        print(f"Outputs have been successfully loaded from {file_path}!")
        return output

    if manifest.get("format") != _FORMAT:
        raise RuntimeError(f"{file_path} is not a pybtls output manifest.")

    output = {
        tag: _record_to_manager(record) for tag, record in manifest["outputs"].items()
    }
    print(f"Outputs have been successfully loaded from {file_path}!")
    return output
