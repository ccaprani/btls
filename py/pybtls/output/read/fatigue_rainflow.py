import pandas as pd
from pathlib import Path

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
        The fatigue rainflow data.
    """

    # Read data
    return_data = pd.read_csv(
        file_path,
        delimiter="\s+",
        names=["Amplitude", "No. Cycles"],
        skiprows=1,
    )

    # Sort the DataFrame by 'Amplitude'
    return_data.sort_values(by="Amplitude", inplace=True)

    # Group by 'Amplitude', sum 'No. Cycles' for duplicates, and reset index
    return_data = return_data.groupby("Amplitude", as_index=False)["No. Cycles"].sum()

    return return_data
