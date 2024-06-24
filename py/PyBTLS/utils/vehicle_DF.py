"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp. 
"""

from ..lib.BTLS import Vehicle, _Vehicle
import pandas as pd

__all__ = ["vehicle_list_to_df", "df_to_vehicle_list"]


def vehicle_list_to_df(vehicle_list: list[_Vehicle]) -> pd.DataFrame:
    """
    Convert a list of Vehicle objects to a pandas DataFrame.

    Parameters
    ----------
    vehicle_list : list[_Vehicle]\n
        List of Vehicle objects. \n
        _Vehicle and Vehicle are the same.

    Returns
    -------
    pd.DataFrame\n
        A DataFrame containing all the vehicle properties.
    """

    if not all(isinstance(vehicle, (Vehicle, _Vehicle)) for vehicle in vehicle_list):
        raise ValueError("All vehicles in the list must be of type Vehicle.")

    data_list = [vehicle.get_all_properties() for vehicle in vehicle_list]
    column_names = [
        "Head",  # vehicle id
        "Year",
        "Month",
        "Day",
        "Hour",
        "Min",
        "Sec",  # the only float value related to time
        "NoAxles",
        "NoAxleGroups",
        "GVW",  # in kN
        "Velocity",  # in m/s
        "Length",  # in m
        "Lane",  # 1-based local lane number
        "Dir",  # 1 or 2
        "Trns",  # in meters, transverse position on lane
        "AxleWeights",  # list, all in kN
        "AxleSpacings",  # list, all in m
        "AxleWidths",  # list, all in m, 1.98m by default
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
        ]
    ):
        raise ValueError("DataFrame must contain all the required columns.")

    # Recalculate the GVW and Length just in case if AxleWeights and AxleSpacings are modified. The NoAxles should not be modified; instead, a new vehicle should be created if user wants to change the number of axles. 
    df["GVW"] = df["AxleWeights"].apply(lambda x: sum(x))
    df["Length"] = df["AxleSpacings"].apply(lambda x: sum(x))

    vehicle_list = []
    for row in df.itertuples(index=False):
        vehicle = Vehicle(0)  # 0 does not matter
        vehicle.set_all_properties(tuple(row))
        vehicle_list.append(vehicle)

    return vehicle_list
