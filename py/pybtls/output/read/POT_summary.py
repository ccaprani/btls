import pandas as pd
from pathlib import Path

from ._empty import read_csv_or_empty

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
        One row per peak, with columns:\n
        - "Peak Index" : int, the row's 1-based line number in the file
          (``start_line``, clamped to 1, for the first row read). This
          is renumbered on read and does not preserve the original
          file's block-local peak index.\n
        - "Time" : float, seconds.\n
        - "No. Vehicles" : int, the total number of vehicles in the
          event, including cars.\n
        - "Peak Value" : float, in the effect's native unit (kN or
          kN·m).\n
        Returns an empty DataFrame with this schema if the file has no
        data rows.
    """

    # Read data
    column_names = ["Peak Index", "Time", "No. Vehicles", "Peak Value"]
    return_data = read_csv_or_empty(
        file_path,
        column_names,
        delimiter="\s+",
        names=column_names,
        skiprows=max(0, start_line - 1),
        nrows=no_lines,
    )

    # Renumber Peak Index to reflect each row's actual position in the file.
    first_index = max(1, start_line)
    return_data["Peak Index"] = range(first_index, first_index + len(return_data))

    return return_data
