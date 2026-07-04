import pandas as pd
from pathlib import Path

from ._empty import read_csv_or_empty

__all__ = ["read_FR"]


def read_FR(file_path: Path) -> pd.DataFrame:
    """
    Read the fatigue rainflow data from pybtls results.\n
    This output file has a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the fatigue rainflow data file.\n

    Returns
    -------
    pd.DataFrame\n
        One row per distinct amplitude bin, with columns:\n
        - "Amplitude" : float, in the load effect's native unit (kN or
          kN·m).\n
        - "No. Cycles" : float, the cycle count for that bin. Can be 0.5
          for ASTM rainflow half-cycles (Rainflow.cpp).\n
        Rows are sorted by "Amplitude", and any duplicate amplitude
        values in the file are merged by summing "No. Cycles"
        (``groupby("Amplitude").sum()``). Returns an empty DataFrame with
        this schema if the file has no data rows.
    """

    # Read data
    return_data = read_csv_or_empty(
        file_path,
        ["Amplitude", "No. Cycles"],
        delimiter="\s+",
        names=["Amplitude", "No. Cycles"],
        skiprows=1,
    )

    # Sort the DataFrame by 'Amplitude'
    return_data.sort_values(by="Amplitude", inplace=True)

    # Group by 'Amplitude', sum 'No. Cycles' for duplicates, and reset index
    return_data = return_data.groupby("Amplitude", as_index=False)["No. Cycles"].sum()

    return return_data
