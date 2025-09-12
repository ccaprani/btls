import pandas as pd
from pathlib import Path
from collections import defaultdict

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
    column_ids = ["Time (s)", "No. Trucks"] + [
        f"Effect {i + 1}" for i in range(no_effects)
    ]
    return_data.columns = column_ids

    # # Fill the data (if use, remove the corresponding part in plot/time_history.py)
    # time_step = return_data["Time (s)"].iloc[1] - return_data["Time (s)"].iloc[0]
    # column_names = return_data.columns.tolist()[2:]
    # data_time = []
    # data_no_trucks = []
    # data_val = defaultdict(list)
    # for i in range(len(return_data["Time (s)"]) - 1):
    #     data_time.append(return_data["Time (s)"].iloc[i])
    #     data_no_trucks.append(return_data["No. Trucks"].iloc[i])
    #     for name in column_names:
    #         data_val[name].append(return_data[name].iloc[i])
    #     time_diff = return_data["Time (s)"].iloc[i + 1] - return_data["Time (s)"].iloc[i]
    #     if abs(time_diff - time_step) > 1e-9:
    #         data_time.append(return_data["Time (s)"].iloc[i] + time_step)
    #         data_no_trucks.append(0)
    #         for name in column_names:
    #             data_val[name].append(0.0)
    # data_time.append(return_data["Time (s)"].iloc[-1])
    # data_no_trucks.append(return_data["No. Trucks"].iloc[-1])
    # for name in column_names:
    #     data_val[name].append(return_data[name].iloc[-1])

    # return_data["Time (s)"] = pd.Series(data_time)
    # return_data["No. Trucks"] = pd.Series(data_no_trucks)
    # for name in column_names:
    #     return_data[name] = pd.Series(data_val[name])

    return return_data
