import pandas as pd
from pathlib import Path

from ._empty import empty_frame

__all__ = ["read_FE"]


def read_FE(file_path: Path, no_lines: int = None, start_line: int = 1) -> pd.DataFrame:
    """
    Read the fatigue events data from pybtls results.\n
    This output file does not have a header.

    Parameters
    ----------
    file_path : Path\n
        The path to the fatigue events data file.\n
    no_lines : int, optional\n
        The number of data lines to read from the file.\n
        If not specified, all lines will be read.\n
    start_line : int, optional\n
        Default is 1. \n
        The line to start reading data from.

    Returns
    -------
    pd.DataFrame\n
        One row per fatigue event, with columns:\n
        - "Start Time" : float, seconds.\n
        - "No. Vehicles" : int, the total number of vehicles in the
          event, including cars.\n
        - "Effect N Max Time", "Effect N Max Amplitude", "Effect N Min
          Time", "Effect N Min Amplitude" (for each recorded effect N):
          float. Time in seconds, amplitude in the effect's native unit
          (kN or kN·m). Each event is written as two lines (one extreme
          per line); the two are re-ordered here by comparing amplitudes
          so "Max" is always the larger value, regardless of which one
          occurred first in the file. The comparison is on the signed
          value, whereas the engine picks its event extremes by absolute
          magnitude, so for an effect with negative ordinates (e.g. a
          hogging influence line) "Max" is the algebraically larger of
          the pair, not the largest-magnitude one.\n
        The number of effects is inferred from the file. Returns an
        empty DataFrame with this schema if the file has no data rows.
    """

    # Read data
    data_rows = []
    no_effects = 0

    with open(file_path, "r") as file:
        for _ in range(max(0, 2 * (start_line - 1))):
            next(file, None)  # Skip the specified number of lines
        i = 0

        for line in file:
            line_2 = next(file, None)
            if line_2 is None:
                break  # A truncated half event; drop it.

            split_line_1 = line.strip().split()  # Split by spaces or tabs
            split_line_2 = line_2.strip().split()  # Split by spaces or tabs

            no_effects = int((len(split_line_1) - 1) / 2)

            # The two lines hold the (max, min) pair for each effect in
            # chronological order (whichever occurs first is written
            # first), not max-first: compare the amplitudes to tell which
            # is which.
            ordered_line = [
                split_line_1[0],
                split_line_2[0],
            ]  # Event Time, No. Vehicles
            for j in range(no_effects):
                time_1 = split_line_1[2 * j + 1]
                value_1 = split_line_1[2 * j + 2]
                time_2 = split_line_2[2 * j + 1]
                value_2 = split_line_2[2 * j + 2]
                if float(value_1) >= float(value_2):
                    ordered_line.extend([time_1, value_1, time_2, value_2])
                else:
                    ordered_line.extend([time_2, value_2, time_1, value_1])

            data_rows.append(ordered_line)

            i += 1
            if no_lines is not None and i >= no_lines:
                break

    # Set column ids
    column_ids = ["Start Time", "No. Vehicles"]
    for i in range(no_effects):
        time_max_id = f"Effect {i + 1} Max Time"
        max_id = f"Effect {i + 1} Max Amplitude"
        time_min_id = f"Effect {i + 1} Min Time"
        min_id = f"Effect {i + 1} Min Amplitude"
        column_ids.extend([time_max_id, max_id, time_min_id, min_id])

    if not data_rows:
        return empty_frame(column_ids)

    # Convert to DataFrame
    return_data = pd.DataFrame(data_rows, columns=column_ids)

    # Convert data types
    return_data = return_data.astype(float)
    return_data["No. Vehicles"] = return_data["No. Vehicles"].astype(int)

    return return_data
