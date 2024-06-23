import pandas as pd
from pathlib import Path

__all__ = ["read_AE"]


def read_AE(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the all events data from pybtls results.

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
        The all events data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        sep="[\s\t]+",
        header=None,
        skiprows=start_line-1,
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
