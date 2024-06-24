import pandas as pd
from pathlib import Path
from ...lib.BTLS import Vehicle

__all__ = ["read_event_file"]


def read_event_file(file_path: Path) -> pd.DataFrame:
    """
    A standard read-in function for BM_by_no_trucks, BM_by_mixed and POT_vehicle files.\n
    These files do not have headers.

    Parameters
    ----------
    file_path : Path\n
        The path to the effect interval statistics data file.

    Returns
    -------
    pd.DataFrame\n
        The effect interval statistics data.
    """

    # Read data
    data_rows = []

    with open(file_path, 'r') as file:
        for line in file:

            if " " not in line:
                index = line.strip()
                continue

            split_line = line.strip().split()  # Split by spaces or tabs

            if len(split_line) == 5:
                data_rows.append([index] + split_line + [[]])

            else:
                vehicle = Vehicle(0)  # 0 does not matter, just to get the class
                vehicle._create(line, 1)
                data_rows[-1][-1].append(vehicle)

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    return_data.columns = ["Index", "Effect", "Value", "Time", "Position on Bridge", "No. Trucks", "Trucks"]  # The "Position on Bridge" means the distance of the first axle of the first truck on the bridge relative to the bridge datum, at the time of the crossing event maximum effect being reached.

    # Convert data types
    return_data["Index"] = return_data["Index"].astype(int)
    return_data["Effect"] = return_data["Effect"].astype(int)
    return_data["Value"] = return_data["Value"].astype(float)
    return_data["Time"] = return_data["Time"].astype(float)
    return_data["Position on Bridge"] = return_data["Position on Bridge"].astype(float)
    return_data["No. Trucks"] = return_data["No. Trucks"].astype(int)
    # return_data["Trucks"] keeps unchanged

    return return_data
