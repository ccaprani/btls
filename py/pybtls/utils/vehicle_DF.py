"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

import pandas as pd
from ..lib.BTLS import Vehicle

__all__ = ["vehicle_list_to_df", "df_to_vehicle_list"]


def vehicle_list_to_df(vehicle_list: list[Vehicle]) -> pd.DataFrame:
    """
    Convert a list of Vehicle objects to a pandas DataFrame.

    Parameters
    ----------
    vehicle_list : list[Vehicle]\n
        List of Vehicle objects.

    Returns
    -------
    pd.DataFrame\n
        A DataFrame containing all the vehicle properties, with columns:\n
        - "Head" : vehicle id.\n
        - "Day", "Month", "Year", "Hour", "Min" : int, calendar/clock
          fields.\n
        - "Sec" : float, the only time field with sub-second precision.\n
        - "NoAxles", "NoAxleGroups" : int.\n
        - "GVW" : float, gross vehicle weight in kN.\n
        - "Velocity" : float, in m/s.\n
        - "Length" : float, in m.\n
        - "Lane" : int, 1-based local lane number.\n
        - "Dir" : int, 1 or 2.\n
        - "Trns" : float, transverse position on lane, in metres.\n
        - "AxleWeights" : list[float], all in kN.\n
        - "AxleSpacings" : list[float], all in m.\n
        - "AxleWidths" : list[float], all in m, 1.98 m by default.\n
        - "Acceleration" : float, in m/s^2, negative = braking, 0.0 by
          default.
    """

    if not all(isinstance(vehicle, Vehicle) for vehicle in vehicle_list):
        raise ValueError("All vehicles in the list must be of type Vehicle.")

    data_list = [vehicle._get_all_properties() for vehicle in vehicle_list]
    column_names = [
        "Head",
        "Day",
        "Month",
        "Year",
        "Hour",
        "Min",
        "Sec",
        "NoAxles",
        "NoAxleGroups",
        "GVW",
        "Velocity",
        "Length",
        "Lane",
        "Dir",
        "Trns",
        "AxleWeights",
        "AxleSpacings",
        "AxleWidths",
        "Acceleration",
    ]

    return pd.DataFrame(data_list, columns=column_names)


def df_to_vehicle_list(df: pd.DataFrame) -> list[Vehicle]:
    """
    Convert a pandas DataFrame to a list of Vehicle objects.

    Parameters
    ----------
    df : pd.DataFrame\n
        A DataFrame containing vehicle properties.

    Returns
    -------
    list[Vehicle]\n
        A list of Vehicle objects.
    """

    if not all(
        col in df.columns
        for col in [
            "Head",
            "Year",
            "Month",
            "Day",
            "Hour",
            "Min",
            "Sec",
            "NoAxles",
            "NoAxleGroups",
            "GVW",
            "Velocity",
            "Length",
            "Lane",
            "Dir",
            "Trns",
            "AxleWeights",
            "AxleSpacings",
            "AxleWidths",
            "Acceleration",
        ]
    ):
        raise ValueError("DataFrame must contain all the required columns.")

    # Recalculate the GVW and Length just in case if AxleWeights and AxleSpacings are modified. The NoAxles should not be modified; instead, a new vehicle should be created if user wants to change the number of axles.
    df["GVW"] = df["AxleWeights"].apply(lambda x: sum(x))
    df["Length"] = df["AxleSpacings"].apply(lambda x: sum(x))

    vehicle_list = []
    for row in df.itertuples(index=False):
        vehicle = Vehicle(0)  # 0 does not matter
        vehicle._set_all_properties(tuple(row))
        vehicle_list.append(vehicle)

    return vehicle_list
