from .event_file import read_event_file
from pathlib import Path
import pandas as pd

__all__ = ["read_POT_V"]


def read_POT_V(file_path: Path) -> pd.DataFrame:
    """
    Read the POT by certain no. trucks data from pybtls results.\n
    The "Index" column in the return df indicates the particular loading event in legacy files. \n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the POT by certain no. trucks data file.

    Returns
    -------
    pd.DataFrame\n
        One row per (event, load effect) combination. See
        ``read_event_file`` for the full column schema (Index, Effect,
        Value [native unit, kN or kN·m], Time [s], Position on Bridge
        [m], No. Vehicles [incl. cars], Trucks).
    """

    return read_event_file(file_path)
