from .event_file import read_event_file
from pathlib import Path
from typing import Literal
import pandas as pd

__all__ = ["read_BM_V"]


def read_BM_V(file_path: Path, file_format: Literal[1, 2, 3, 4] = 1) -> pd.DataFrame:
    """
    Read the BM by certain no. trucks data from pybtls results.\n
    The "Index" column in the return df indicates the current block. \n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the BM by certain no. trucks data file.\n
    file_format : Literal[1,2,3,4], optional\n
        Default is 1 (CASTOR format), as written by legacy files.\n
        The format the vehicle lines were written in, i.e. the
        ``vehicle_file_format`` the simulation was run with.\n
        1: CASTOR format.\n
        2: BEDIT format.\n
        3: DITIS format.\n
        4: MON format.

    Returns
    -------
    pd.DataFrame\n
        One row per (block, load effect) combination. See
        ``read_event_file`` for the full column schema (Index, Effect,
        Value [native unit, kN or kN·m], Time [s], Position on Bridge
        [m], No. Vehicles [incl. cars], Trucks).
    """

    return read_event_file(file_path, file_format)
