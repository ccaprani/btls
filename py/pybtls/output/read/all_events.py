import pandas as pd
from pathlib import Path

__all__ = ["read_AE"]


def read_AE(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the all events data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the all events data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1.\n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        One row per event, with columns:\n
        - "Start Time" : float, seconds.\n
        - "No. Vehicles" : int, the total number of vehicles in the
          event, including cars.\n
        - "Effect 1", "Effect 2", ... : float, the load effect value at
          the event maximum, in the effect's native unit (kN or kN·m
          depending on the influence line). The number of effect
          columns is inferred from the file.\n
        Returns an empty DataFrame with columns ["Start Time",
        "No. Vehicles"] if the file has no data rows.
    """

    # Read data
    try:
        return_data = pd.read_csv(
            file_path,
            sep=r"\s+",
            header=None,
            skiprows=max(0, start_line - 1),
            nrows=no_lines,
        )
    except pd.errors.EmptyDataError:
        return pd.DataFrame(columns=["Start Time", "No. Vehicles"])

    no_effects = len(return_data.columns) - 2

    # Set column ids
    column_ids = ["Start Time", "No. Vehicles"] + [
        f"Effect {i + 1}" for i in range(no_effects)
    ]
    return_data.columns = column_ids

    return return_data
