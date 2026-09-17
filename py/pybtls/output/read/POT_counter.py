import pandas as pd
from pathlib import Path

from ._empty import read_csv_or_empty

__all__ = ["read_POT_C"]


def read_POT_C(
    file_path: Path, no_lines: int = None, start_line: int = 1
) -> pd.DataFrame:
    """
    Read the POT counter data from pybtls results.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the POT counter data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        One row per POT counting block, with columns:\n
        - "Block" : int, the block index.\n
        - "Effect 1", "Effect 2", ... : unsigned int, the number of
          threshold exceedances (peaks over threshold) recorded for that
          load effect in the block. Unlike the "Effect N" columns
          elsewhere in this package (e.g. read_AE, read_TH), these are
          exceedance *counts*, not load effect values.\n
        The number of effect columns is inferred from the file. Returns
        a DataFrame with only the "Block" column (no rows) if the file
        has no data rows.
    """

    # Read data
    return_data = read_csv_or_empty(
        file_path,
        ["Block"],
        sep=r"\s+",
        header=None,
        skiprows=max(1, start_line),
        nrows=no_lines,
    )

    no_effects = len(return_data.columns) - 1

    # Set column ids
    column_ids = ["Block"] + [f"Effect {i + 1}" for i in range(no_effects)]
    return_data.columns = column_ids

    return return_data
