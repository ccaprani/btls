import pandas as pd
from pathlib import Path

__all__ = ["read_FE"]


def read_FE(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the fatigue events data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the fatigue events data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The fatigue events data.
    """

    # Read data
    data_rows = []

    with open(file_path, "r") as file:
        for _ in range(max(0, 2 * (start_line - 1))):
            next(file)  # Skip the specified number of lines
        i = 0

        for line in file:
            split_line_1 = line.strip().split()  # Split by spaces or tabs
            split_line_2 = next(file).strip().split()  # Split by spaces or tabs

            no_effects = int((len(split_line_1) - 1) / 2)

            ordered_line = [split_line_1[0], split_line_2[0]]  # Event Time, No. Trucks
            for j in range(no_effects):
                ordered_line.append(split_line_1[2 * j + 1])  # Time Max
                ordered_line.append(split_line_1[2 * j + 2])  # Max
                ordered_line.append(split_line_2[2 * j + 1])  # Time Min
                ordered_line.append(split_line_2[2 * j + 2])  # Min

            data_rows.append(ordered_line)

            i += 1
            if no_lines is not None and i >= no_lines:
                break

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)

    # Set column ids
    column_ids = ["Start Time", "No. Trucks"]
    for i in range(no_effects):
        time_max_id = f"Effect {i + 1} Max Time"
        max_id = f"Effect {i + 1} Max Amplitude"
        time_min_id = f"Effect {i + 1} Min Time"
        min_id = f"Effect {i + 1} Min Amplitude"
        column_ids.extend([time_max_id, max_id, time_min_id, min_id])
    return_data.columns = column_ids

    # Convert data types
    return_data = return_data.astype(float)
    return_data["No. Trucks"] = return_data["No. Trucks"].astype(int)

    return return_data
