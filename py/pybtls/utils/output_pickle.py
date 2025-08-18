"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from ..output.output_manager import _OutputManager
from pathlib import Path
import pickle

__all__ = ["save_output", "load_output"]


def save_output(output: dict[str, _OutputManager], file_path: Path) -> None:
    """
    Save the output dir data to a binary pkl file.

    Parameters
    ----------
    output : dict[str, _OutputManager]\n
        The output dir data to save.\n
        The keys are the sim_tags.\n
        The values are the _OutputManager objects.\n
    file_path : Path\n
        The path to the file where the output dir data will be saved (end by .pkl).
    """

    if not all(isinstance(obj, _OutputManager) for obj in output.values()):
        raise ValueError(
            "All values in the output dictionary must be of type _OutputManager."
        )
    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path
    file_path.parent.mkdir(parents=True, exist_ok=True)
    with open(file_path, "wb") as file:
        pickle.dump(output, file)

    print(f"Outputs have been successfully saved to {file_path}!")


def load_output(file_path: Path) -> dict[str, _OutputManager]:
    """
    Load the output dir data from a binary pkl file.

    Parameters
    ----------
    file_path : Path\n
        The path to the file where the output dir data is saved (end by .pkl).

    Returns
    -------
    dict[str, _OutputManager]\n
        The output dir data.
    """

    file_path = Path(file_path) if not isinstance(file_path, Path) else file_path
    with open(file_path, "rb") as file:
        output = pickle.load(file)

    if not all(isinstance(obj, _OutputManager) for obj in output.values()):
        raise RuntimeError("The output pkl is damaged.")
    else:
        print(f"Outputs have been successfully loaded from {file_path}!")

    return output
