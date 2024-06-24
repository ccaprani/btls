import pandas as pd
from pathlib import Path

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
        The effect cumulative statistics data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        delimiter="\s+",
        header=None,
        skiprows=1,
    )

    # Remove the truck presence counts (it could mislead user to a wrong number of trucks presence since a truck could be involved in multiple events).
    return_data = return_data.drop(return_data.columns[9:], axis=1)

    # Set the column names
    return_data.columns = [
        "Effect",
        "No. Events",
        "No. Vehicles",
        "No. Trucks",
        "Mean",
        "Std Dev",
        "Variance",
        "Skewness",
        "Kurtosis",
    ]

    return return_data

    # # These are to be used to combine the excluded truck presence counts into a single column
    # existing_columns = list(return_data.columns)
    # for i, name in enumerate(column_names):
    #     existing_columns[i] = name
    # return_data.columns = existing_columns

    # # Combine the truck presence counts into a single column
    # no_truck_class = len(return_data.columns) - 9
    # return_data["Truck Presence Counts"] = return_data.apply(lambda row: row[-no_truck_class:].tolist(), axis=1)
