from .event_file import read_event_file
from pathlib import Path
import pandas as pd

__all__ = ["read_BM_V"]


def read_BM_V(file_path: Path) -> pd.DataFrame:
    """
    Read the BM by certain no. trucks data from pybtls results.\n 
    The "Index" column in the return df indicates the current block. \n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the BM by certain no. trucks data file.

    Returns
    -------
    pd.DataFrame\n
        The BM by certain no. trucks data.
    """

    return read_event_file(file_path)
    