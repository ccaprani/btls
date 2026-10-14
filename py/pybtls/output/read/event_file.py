import pandas as pd
from pathlib import Path
from typing import Literal
from ...lib.BTLS import Vehicle
from ._empty import empty_frame

__all__ = ["read_event_file"]


def read_event_file(
    file_path: Path, file_format: Literal[1, 2, 3, 4] = 1
) -> pd.DataFrame:
    """
    A standard read-in function for BM_by_no_trucks, BM_by_mixed and POT_vehicle files.\n
    These files do not have headers. Each event is one summary line
    (Index, Effect, Value, Time, Position on Bridge, No. Vehicles)
    followed by zero or more vehicle lines describing the vehicles
    present in that event; the summary line is repeated once per load
    effect recorded for the event.

    Parameters
    ----------
    file_path : Path\n
        The path to the BM/POT event-and-vehicle data file.\n
    file_format : Literal[1,2,3,4], optional\n
        Default is 1 (CASTOR format), as written by legacy files.\n
        The format the vehicle lines were written in, i.e. the
        ``vehicle_file_format`` the simulation was run with.\n
        1: CASTOR format.\n
        2: BEDIT format.\n
        3: DITIS format.\n
        4: MON format.

    Returns
    -------
    pd.DataFrame\n
        One row per (event, load effect) combination, with columns:\n
        - "Index" : int, the block/event index.\n
        - "Effect" : int, the 1-based load effect number.\n
        - "Value" : float, the load effect value at the event maximum, in
          the effect's native unit (kN or kN·m depending on the influence
          line).\n
        - "Time" : float, the time of the event maximum, in seconds.\n
        - "Position on Bridge" : float, in metres. The distance of the
          first axle of the first truck on the bridge relative to the
          bridge datum, at the time the crossing event maximum effect is
          reached.\n
        - "No. Vehicles" : int, the total number of vehicles in the
          event, including cars.\n
        - "Trucks" : list[Vehicle], the vehicles present in the event.\n
        Returns an empty DataFrame with this schema if the file has no
        data rows.
    """

    column_ids = [
        "Index",
        "Effect",
        "Value",
        "Time",
        "Position on Bridge",
        "No. Vehicles",  # total no. of vehicles in the event, including cars
        "Trucks",
    ]  # The "Position on Bridge" means the distance of the first axle of the first truck on the bridge relative to the bridge datum, at the time of the crossing event maximum effect being reached.

    # Read data
    data_rows = []

    with open(file_path, "r") as file:
        for line in file:
            if " " not in line:
                index = line.strip()
                continue

            split_line = line.strip().split()  # Split by spaces or tabs

            if len(split_line) == 5:
                data_rows.append([index] + split_line + [[]])

            else:
                vehicle = Vehicle(0)  # 0 does not matter, just to get the class
                # Strip the newline: the MON reader infers how many axles
                # the line holds from its length.
                vehicle._create(line.rstrip("\n"), file_format)
                data_rows[-1][-1].append(vehicle)

    if not data_rows:
        return empty_frame(column_ids)

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    return_data.columns = column_ids

    # Convert data types
    return_data["Index"] = return_data["Index"].astype(int)
    return_data["Effect"] = return_data["Effect"].astype(int)
    return_data["Value"] = return_data["Value"].astype(float)
    return_data["Time"] = return_data["Time"].astype(float)
    return_data["Position on Bridge"] = return_data["Position on Bridge"].astype(float)
    return_data["No. Vehicles"] = return_data["No. Vehicles"].astype(int)
    # return_data["Trucks"] keeps unchanged

    return return_data
