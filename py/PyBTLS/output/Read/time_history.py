import pandas as pd
from pathlib import Path

__all__ = ["read_TH"]


def read_TH(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the time history data from pybtls results.\n
    Warning: This could blow up the computer memory.\n
    Do set a no_lines to partially read the data.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the time history data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The time history data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        sep="[\s\t]+",
        header=None,
        skiprows=max(1, start_line),
        nrows=no_lines,
        engine="python",
    )
    no_effects = len(return_data.columns) - 2

    # Set column ids
    column_ids = ["Time (s)", "No. Trucks"]
    for i in range(no_effects):
        effect_id = f"Effect {i + 1}"
        column_ids.append(effect_id)
    return_data.columns = column_ids

    return return_data
