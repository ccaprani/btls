import pandas as pd
from pathlib import Path

__all__ = ["read_BM_S"]


def read_BM_S(
    file_path: Path, no_lines: int = None, start_line: int = 1
) -> pd.DataFrame:
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
    data_rows = []

    with open(file_path, "r") as file:
        for _ in range(max(0, start_line - 1)):
            next(file)  # Skip the specified number of lines
        i = 0

        for line in file:
            split_line = line.strip().split()  # Split by spaces or tabs
            data_rows.append(split_line)

            i += 1
            if no_lines is not None and i >= no_lines:
                break

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    no_event_types = len(return_data.columns) - 1

    # Set column ids
    column_ids = ["Block Index"] + [
        f"{i + 1}-Truck Event" for i in range(no_event_types)
    ]
    return_data.columns = column_ids

    # Convert data types
    return_data["Block Index"] = return_data["Block Index"].astype(int)
    for i in range(no_event_types):
        return_data[f"{i + 1}-Truck Event"] = pd.to_numeric(
            return_data[f"{i + 1}-Truck Event"], errors="coerce"
        )

    return return_data
