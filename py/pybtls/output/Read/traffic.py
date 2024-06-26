from ...garage.read import read_garage_file
from ...utils.vehicle_DF import vehicle_list_to_df
from pathlib import Path
from typing import Literal
import pandas as pd

__all__ = ["read_traffic"]


def read_traffic(file_path: Path, traffic_format: Literal[1, 2, 3, 4]) -> pd.DataFrame:
    """
    Read the traffic data from pybtls results.

    Parameters
    ----------
    file_path : Path\n
        The path to the traffic data file.\n
    traffic_format : Literal[1,2,3,4]\n
        The format of the .txt traffic file.\n
        1: CASTOR format.\n
        2: BEDIT format.\n
        3: DITIS format.\n
        4: MON format.

    Returns
    -------
    pd.DataFrame\n
        The traffic data.
    """

    # Read data
    vehicle_list = read_garage_file(file_path, traffic_format)
    return_data = vehicle_list_to_df(vehicle_list)

    return return_data
