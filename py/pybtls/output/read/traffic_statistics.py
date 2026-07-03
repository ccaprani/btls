import pandas as pd
from pathlib import Path

__all__ = ["read_TS"]


def read_TS(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the traffic statistics data from pybtls results.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the traffic statistics data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        One row per reporting interval (despite the name, "Hour" is a
        1-based interval index, not necessarily one hour - it matches
        whatever interval size the simulation was configured with).
        Columns:\n
        - "Hour" : int, 1-based interval index.\n
        - "No. Vehicles" : int, total vehicles in the interval, including
          cars.\n
        - "No. Trucks" : int, trucks in the interval, excluding cars.\n
        - "No. Cars" : int.\n
        - "0: Default", "1: Car", ... : int, per-class vehicle counts.
          The schema is chosen dynamically by sniffing the file's header
          line: if it contains "-axle" (case-insensitive), the remaining
          columns are the axle-count classifier ("2: 2-axle" ...
          "5: 5-axle"); otherwise they are the vehicle-pattern classifier
          ("2: Pattern 11" ... "8: Pattern 113"). The two schemas have
          different column counts and names.\n
        Returns an empty DataFrame with the schema inferred from the
        header line if the file has no data rows (the header line, and
        therefore the schema choice, is still required to exist).
    """

    # Check the what vehicle classifier was used
    with open(file_path, "r") as file:
        headline = file.readline().strip()
    if "-axle" in headline.lower():
        column_names = [
            "Hour",
            "No. Vehicles",
            "No. Trucks",
            "No. Cars",
            "0: Default",
            "1: Car",
            "2: 2-axle",
            "3: 3-axle",
            "4: 4-axle",
            "5: 5-axle",
        ]
    else:
        column_names = [
            "Hour",
            "No. Vehicles",
            "No. Trucks",
            "No. Cars",
            "0: Default",
            "1: Car",
            "2: Pattern 11",
            "3: Pattern 123",
            "4: Pattern 12",
            "5: Pattern 1233",
            "6: Pattern 122",
            "7: Pattern 112",
            "8: Pattern 113",
        ]

    # Read data
    try:
        return_data = pd.read_csv(
            file_path,
            delimiter="\s+",
            names=column_names,
            skiprows=max(1, start_line),
            nrows=no_lines,
        )
    except pd.errors.EmptyDataError:
        return_data = pd.DataFrame(columns=column_names)

    return return_data
