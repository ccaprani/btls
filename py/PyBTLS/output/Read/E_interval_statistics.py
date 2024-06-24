import pandas as pd
from pathlib import Path

__all__ = ["read_E_IS"]


def read_E_IS(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the effect interval statistics data from pybtls results.\n
    This output file has a header. 

    Parameters
    ----------
    file_path : Path\n
        The path to the effect interval statistics data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        The effect interval statistics data.
    """

    # Read data
    data_rows = []

    with open(file_path, 'r') as file:
        for _ in range(max(1, start_line)):
            next(file)  # Skip the header and the specified number of lines
        i = 0
        for line in file:
            split_line = line.strip().split()  # Split by spaces or tabs
            data_rows.append(split_line)
            i += 1
            if no_lines is not None and i >= no_lines:
                break

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    return_data = return_data.drop(return_data.columns[10:], axis=1)  # Remove the useless truck presence counts
    return_data.columns = ["Index", "Time", "No. Events", "No. Vehicles", "No. Trucks", "Mean", "Std Dev", "Variance", "Skewness", "Kurtosis"]

    # Convert data types
    return_data["Index"] = return_data["Index"].astype(int)
    return_data["Time"] = return_data["Time"].astype(int)
    return_data["No. Events"] = return_data["No. Events"].astype(int)
    return_data["No. Vehicles"] = return_data["No. Vehicles"].astype(int)
    return_data["No. Trucks"] = return_data["No. Trucks"].astype(int)
    return_data["Mean"] = return_data["Mean"].astype(float)
    return_data["Std Dev"] = return_data["Std Dev"].astype(float)
    return_data["Variance"] = return_data["Variance"].astype(float)
    return_data["Skewness"] = return_data["Skewness"].astype(float)
    return_data["Kurtosis"] = return_data["Kurtosis"].astype(float)

    return return_data
