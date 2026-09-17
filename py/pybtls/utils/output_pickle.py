"""
Persistence for simulation output managers.

The on-disk format is a JSON manifest: an output manager is only a set of
file paths plus the output configuration, so a human-readable manifest is
robust across pybtls/pandas versions (the simulation data itself always
lives in the output text files). The pickle files that pybtls 1.0.1 and
earlier saved are read by ``load_legacy_output``, which may be removed in a
future release.
"""

from ..output.output_manager import _OutputManager
from ..output.chunked_manager import _ChunkedOutputManager
from ..output.output_config import OutputConfig
from pathlib import Path
from typing import Union
import json
import os
import pickle
import warnings

__all__ = ["save_output", "load_output", "load_legacy_output"]

_FORMAT = "pybtls-output-manifest"
_FORMAT_VERSION = 1


def _manager_to_record(manager) -> dict:
    if isinstance(manager, _ChunkedOutputManager):
        return {
            "type": "chunked",
            "sim_tag": manager.tag,
            # a numpy integer seed runs fine but is not JSON serializable
            "master_seed": (
                int(manager.master_seed) if manager.master_seed is not None else None
            ),
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
    # write beside the target and swap it in, so a failed save leaves any
    # existing manifest intact
    tmp_path = file_path.with_name(file_path.name + ".tmp")
    try:
        with open(tmp_path, "w") as file:
            json.dump(manifest, file, indent=1)
        os.replace(tmp_path, file_path)
    finally:
        tmp_path.unlink(missing_ok=True)

    print(f"Outputs have been successfully saved to {file_path}!")


def load_output(
    file_path: Path,
) -> dict[str, Union[_OutputManager, _ChunkedOutputManager]]:
    """
    Load output managers from a manifest file written by ``save_output``.

    Parameters
    ----------
    file_path : Path\n
        The path of the manifest file.

    Returns
    -------
    dict[str, Union[_OutputManager, _ChunkedOutputManager]]\n
        The output managers, keyed by sim_tag.

    Raises
    ------
    RuntimeError\n
        If the file is not a pybtls output manifest, or is a manifest of a
        format version this pybtls cannot read. A .pkl file saved by
        pybtls 1.0.1 or earlier is read with ``load_legacy_output`` instead.
    """

    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path

    try:
        with open(file_path, "r") as file:
            manifest = json.load(file)
    except (UnicodeDecodeError, json.JSONDecodeError):
        manifest = None
    if not isinstance(manifest, dict) or manifest.get("format") != _FORMAT:
        raise RuntimeError(
            f"{file_path} is not a pybtls output manifest. A .pkl file saved by "
            "pybtls 1.0.1 or earlier can be read with load_legacy_output()."
        )
    if manifest.get("version") != _FORMAT_VERSION:
        raise RuntimeError(
            f"{file_path} is a pybtls output manifest of version "
            f"{manifest.get('version')}, which this pybtls cannot read (it reads "
            f"version {_FORMAT_VERSION}). Upgrade pybtls to load it."
        )

    output = {
        tag: _record_to_manager(record) for tag, record in manifest["outputs"].items()
    }
    print(f"Outputs have been successfully loaded from {file_path}!")
    return output


def load_legacy_output(
    file_path: Path,
) -> dict[str, Union[_OutputManager, _ChunkedOutputManager]]:
    """
    Load output managers from a pickle file that ``save_output`` wrote in
    pybtls 1.0.1 or earlier.

    Deprecated: this reader is kept for compatibility only and may be removed
    in a future release. Load such a file once and re-save it with
    ``save_output`` to convert it to a JSON manifest. Only load files you trust:
    unpickling a file can run arbitrary code.

    Parameters
    ----------
    file_path : Path\n
        The path of the pickle file.

    Returns
    -------
    dict[str, Union[_OutputManager, _ChunkedOutputManager]]\n
        The output managers, keyed by sim_tag.

    Raises
    ------
    RuntimeError\n
        If the file does not hold output managers.
    """

    warnings.warn(
        "load_legacy_output reads the pickle files of pybtls 1.0.1 and earlier and "
        "may be removed in a future release; re-save the outputs with save_output() "
        "to convert them to a JSON manifest.",
        FutureWarning,
        stacklevel=2,
    )
    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path

    with open(file_path, "rb") as file:
        output = pickle.load(file)
    if not isinstance(output, dict) or not all(
        isinstance(obj, (_OutputManager, _ChunkedOutputManager))
        for obj in output.values()
    ):
        raise RuntimeError(f"{file_path} does not hold pybtls output managers.")
    print(f"Outputs have been successfully loaded from {file_path}!")
    return output
