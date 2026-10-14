import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from collections import defaultdict

__all__ = ["plot_TH"]


def plot_TH(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the time history data from pybtls results.

    One subplot per load effect column, plotted against time.

    Before plotting, gaps in "Time" are filled: the step size is
    inferred from the first two rows (data["Time"].iloc[1] -
    data["Time"].iloc[0]), and wherever a step to the next row is larger
    than that inferred step (by more than 1e-6), a single synthetic
    point is inserted immediately after the current row, one step later
    in time, with all effect values set to 0.0. This changes what is
    actually plotted relative to the raw data. Gap-filling is skipped
    entirely if the DataFrame has fewer than 2 rows.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded time history from read_TH. Must have columns "Time",
        "No. Vehicles", and one or more "Effect N" columns; a subplot is
        drawn for every column after the first two, so a DataFrame with
        fewer than 3 columns raises an error from ``plt.subplots``. The
        x-axis is labelled "Time (s)" assuming "Time" is in seconds (per
        read_TH); the y-axis unit of each "Effect N" column (kN or kN·m)
        is not known to this function and is not labelled.

    save_to : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    plt.rcParams["font.family"] = "Times New Roman"
    plt.rcParams["font.size"] = 16
    plt.rcParams["mathtext.fontset"] = "stix"

    no_effects = len(data.columns) - 2

    fig, axes = plt.subplots(no_effects, 1, sharex=True, figsize=(8, 5 * no_effects))
    if no_effects == 1:
        axes = [axes]

    # Pick the data
    column_names = data.columns.tolist()[2:]

    if len(data) < 2:
        # Not enough points to detect gaps; plot the data directly.
        data_time = data["Time"].tolist()
        data_val = {name: data[name].tolist() for name in column_names}
    else:
        time_step = data["Time"].iloc[1] - data["Time"].iloc[0]

        # Fill the data
        data_time = []
        data_val = defaultdict(list)
        for i in range(len(data["Time"]) - 1):
            data_time.append(data["Time"].iloc[i])
            for name in column_names:
                data_val[name].append(data[name].iloc[i])
            time_diff = data["Time"].iloc[i + 1] - data["Time"].iloc[i]
            if (time_diff - time_step) > 1e-6:
                data_time.append(data["Time"].iloc[i] + time_step)
                for name in column_names:
                    data_val[name].append(0.0)
        data_time.append(data["Time"].iloc[-1])
        for name in column_names:
            data_val[name].append(data[name].iloc[-1])

    # Plotting
    for ax, name in zip(axes, column_names):
        ax.plot(data_time, data_val[name], color="gray")
        ax.set_ylabel(name)
    fig.supxlabel("Time (s)")

    fig.tight_layout()

    if save_to is not None:
        fig.savefig(
            save_to,
            format="png",
            dpi=500,
            pad_inches=0.1,
            bbox_inches="tight",
        )
    else:
        plt.show()

    return None
