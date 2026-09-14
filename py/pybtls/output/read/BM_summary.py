import pandas as pd
from pathlib import Path

from ._empty import empty_frame

__all__ = ["read_BM_S"]


def read_BM_S(
    file_path: Path, no_lines: int = None, start_line: int = 1
) -> pd.DataFrame:
    """
    Read the BM summary data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the BM summary data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1.\n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        One row per block, with columns:\n
        - "Block Index" : int, 1-based block number.\n
        - "1-Truck Event", "2-Truck Event", ... : float, the maximum load
          effect value recorded for that bucket in the block
          (BlockMaxManager.cpp getMaxEffect().getValue()), in the
          effect's native unit (kN or kN·m). Despite the column name,
          this is a load effect value, not an event count, and the
          bucket index is the number of vehicles on the bridge
          (BlockMaxManager.cpp getNoVehicles()), which equals the number
          of trucks only when cars are kept out of the load calculation
          (no car flow, or ``min_gvw`` above the car GVW). A bucket the
          block never filled holds 0.0, whether the engine wrote it or the
          block's row stops short of a later block's buckets (a missing
          bucket means the block had no event of that size); a value that
          fails to parse is NaN.\n
        The number of bucket columns is inferred from the file. Returns
        a DataFrame with only the "Block Index" column (no rows) if the
        file has no data rows, since the number of buckets cannot be
        inferred without any data.
    """

    # Read data
    data_rows = []

    with open(file_path, "r") as file:
        for _ in range(max(0, start_line - 1)):
            next(file, None)  # Skip the specified number of lines
        i = 0

        for line in file:
            split_line = line.strip().split()  # Split by spaces or tabs
            data_rows.append(split_line)

            i += 1
            if no_lines is not None and i >= no_lines:
                break

    if not data_rows:
        # The number of vehicle-count buckets cannot be inferred without
        # any data; return the one column that is always known.
        return empty_frame(["Block Index"])

    # A row lists one value per vehicle-count bucket the block had opened by
    # its end, so a shorter row simply had no event of the missing sizes: pad
    # it with the 0.0 the engine itself writes for an opened-but-empty bucket
    # (CBlockMaxManager::WriteSummaryFiles) rather than leaving NaN, which
    # downstream (fit_gev) would read as an unobserved block.
    width = max(len(row) for row in data_rows)
    data_rows = [row + ["0.0"] * (width - len(row)) for row in data_rows]

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows)
    no_event_types = len(return_data.columns) - 1

    # Set column ids
    column_ids = ["Block Index"] + [
        f"{i + 1}-Truck Event" for i in range(no_event_types)
    ]
    return_data.columns = column_ids

    # Convert data types
    return_data["Block Index"] = return_data["Block Index"].astype(int)
    for i in range(no_event_types):
        return_data[f"{i + 1}-Truck Event"] = pd.to_numeric(
            return_data[f"{i + 1}-Truck Event"], errors="coerce"
        )

    return return_data
