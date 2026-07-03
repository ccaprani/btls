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
        One row per peak, with columns:\n
        - "Peak Index" : int, the peak's 1-based position among the rows
          actually read (i.e. relative to ``start_line``). This is
          renumbered on read and does not preserve the original file's
          block-local peak index.\n
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
    try:
        return_data = pd.read_csv(
            file_path,
            delimiter="\s+",
            names=column_names,
            skiprows=max(0, start_line - 1),
            nrows=no_lines,
        )
    except pd.errors.EmptyDataError:
        return pd.DataFrame(columns=column_names)

    # Renumber Peak Index to reflect each row's actual position in the file.
    return_data["Peak Index"] = range(start_line, start_line + len(return_data))

    return return_data
