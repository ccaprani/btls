import pandas as pd
from pathlib import Path

from ._empty import read_csv_or_empty

__all__ = ["read_E_CS"]


def read_E_CS(file_path: Path) -> pd.DataFrame:
    """
    Read the effect cumulative statistics data from pybtls results.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the effect cumulative statistics data file.

    Returns
    -------
    pd.DataFrame\n
        One row per load effect, with columns:\n
        - "Effect" : int, the 1-based load effect number.\n
        - "No. Events" : int, the number of events counted.\n
        - "No. Vehicles" : int, the total number of vehicles across all
          events, including cars.\n
        - "No. Trucks" : int, the total number of trucks across all
          events, excluding cars (unlike the "No. Vehicles"/"No. Trucks"
          columns in the event-file family, which both count all
          vehicles).\n
        - "Min", "Max", "Mean", "Std Dev" : float, in the effect's native
          unit (kN or kN·m).\n
        - "Variance", "Skewness", "Kurtosis" : float, dimensionless.\n
        Returns an empty DataFrame with this schema if the file has no
        data rows.
    """

    column_ids = [
        "Effect",
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

    # Read data
    return_data = read_csv_or_empty(
        file_path,
        column_ids,
        delimiter="\s+",
        header=None,
        skiprows=1,
    )

    # Remove the truck presence counts (it could mislead user to a wrong number of trucks presence since a truck could be involved in multiple events).
    return_data = return_data.drop(return_data.columns[11:], axis=1)

    # Set the column names (must match CEventStatistics::outputString order:
    # N, vehicles, trucks, min, max, mean, stddev, variance, skewness, kurtosis)
    return_data.columns = column_ids

    return return_data

    # # These are to be used to combine the excluded truck presence counts into a single column
    # existing_columns = list(return_data.columns)
    # for i, name in enumerate(column_names):
    #     existing_columns[i] = name
    # return_data.columns = existing_columns

    # # Combine the truck presence counts into a single column
    # no_truck_class = len(return_data.columns) - 9
    # return_data["Truck Presence Counts"] = return_data.apply(lambda row: row[-no_truck_class:].tolist(), axis=1)
