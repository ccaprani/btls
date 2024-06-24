import pandas as pd
from pathlib import Path

__all__ = ["read_BM_S"]


def read_BM_S(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the BM summary data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the BM summary data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1.\n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The BM summary data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        sep="[\s\t]+",
        header=None,
        skiprows=max(0, start_line - 1),
        nrows=no_lines,
        engine="python",
    )
    no_effect_types = len(return_data.columns) - 1
    
    # Set column ids
    column_ids = ["Block Index"]
    for i in range(no_effect_types):
        effect_type = f"{i + 1}-Truck Event"
        column_ids.append(effect_type)
    return_data.columns = column_ids

    return return_data
