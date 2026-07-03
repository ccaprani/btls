import pandas as pd
from pathlib import Path

__all__ = ["read_E_IS"]


def read_E_IS(
    file_path: Path, no_lines: int = None, start_line: int = 1
) -> pd.DataFrame:
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
        One row per interval (the file covers a single load effect), with
        columns:\n
        - "Index" : int, the 1-based interval index.\n
        - "Time" : int, the interval end time in seconds.\n
        - "No. Events" : int, the number of events counted in the
          interval.\n
        - "No. Vehicles" : int, the total number of vehicles in the
          interval, including cars.\n
        - "No. Trucks" : int, the total number of trucks in the
          interval, excluding cars (unlike the "No. Vehicles"/
          "No. Trucks" columns in the event-file family, which both
          count all vehicles).\n
        - "Min", "Max", "Mean", "Std Dev" : float, in the effect's native
          unit (kN or kN·m).\n
        - "Variance", "Skewness", "Kurtosis" : float, dimensionless.\n
        Returns an empty DataFrame with this schema if the file has no
        data rows.
    """

    # Read data
    data_rows = []

    column_ids = [
        "Index",
        "Time",
        "No. Events",
        "No. Vehicles",
        "No. Trucks",
        "Min",
        "Max",
        "Mean",
        "Std Dev",
        "Variance",
        "Skewness",
        "Kurtosis",
    ]

    with open(file_path, "r") as file:
        for _ in range(max(1, start_line)):
            next(file, None)  # Skip the header and the specified number of lines
        i = 0
        for line in file:
            split_line = line.strip().split()  # Split by spaces or tabs
            data_rows.append(split_line)
            i += 1
            if no_lines is not None and i >= no_lines:
                break

    if not data_rows:
        return pd.DataFrame(columns=column_ids)

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    return_data = return_data.drop(
        return_data.columns[12:], axis=1
    )  # Remove the useless truck presence counts
    # Column names must match CEventStatistics::outputString order:
    # N, vehicles, trucks, min, max, mean, stddev, variance, skewness, kurtosis
    return_data.columns = column_ids

    # Convert data types
    return_data["Index"] = return_data["Index"].astype(int)
    return_data["Time"] = return_data["Time"].astype(int)
    return_data["No. Events"] = return_data["No. Events"].astype(int)
    return_data["No. Vehicles"] = return_data["No. Vehicles"].astype(int)
    return_data["No. Trucks"] = return_data["No. Trucks"].astype(int)
    for col in ("Min", "Max", "Mean", "Std Dev", "Variance", "Skewness", "Kurtosis"):
        return_data[col] = return_data[col].astype(float)

    return return_data
