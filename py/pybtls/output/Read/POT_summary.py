import pandas as pd
from pathlib import Path

__all__ = ["read_POT_S"]


def read_POT_S(
    file_path: Path, no_lines: int = None, start_line: int = 1
) -> pd.DataFrame:
    """
    Read the POT summary data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the POT summary data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1.\n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The POT summary data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        delimiter="\s+",
        names=["Peak Index", "Time", "No. Trucks", "Peak Value"],
        skiprows=max(0, start_line - 1),
        nrows=no_lines,
    )

    return return_data
