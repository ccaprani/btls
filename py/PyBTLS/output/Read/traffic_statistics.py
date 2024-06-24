import pandas as pd
from pathlib import Path

__all__ = ["read_TS"]


def read_TS(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the traffic statistics data from pybtls results.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the traffic statistics data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The traffic statistics data.
    """

    # Check the what vehicle classifier was used
    with open(file_path, "r") as file:
        headline = file.readline().strip()
    if "Axles" in headline:
        column_names = ["Hour", "No. Vehicles", "No. Trucks", "No. Cars", "2-Axles", "3-Axles", "4-Axles", "5-Axles"]
    else:
        column_names = ["Hour", "No. Vehicles", "No. Trucks", "No. Cars", "0: Default", "1: Car", "2: Pattern 11", "3: Pattern 123", "4: Pattern 12", "5: Pattern 1233", "6: Pattern 122", "7: Pattern 112", "8: Pattern 113"]

    # Read data
    return_data = pd.read_csv(
        file_path,
        delimiter='\s+',
        names=column_names,
        skiprows=max(1, start_line),
        nrows=no_lines,
    )

    return return_data
