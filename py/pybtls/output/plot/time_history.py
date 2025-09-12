import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from collections import defaultdict

__all__ = ["plot_TH"]


def plot_TH(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the time history data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded time history from read_TH.

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
    time_step = data["Time (s)"].iloc[1] - data["Time (s)"].iloc[0]

    fig, axes = plt.subplots(no_effects, 1, sharex=True, figsize=(8, 5 * no_effects))
    if no_effects == 1:
        axes = [axes]

    # Pick the data
    column_names = data.columns.tolist()[2:]

    # Fill the data
    data_time = []
    data_val = defaultdict(list)
    for i in range(len(data["Time (s)"]) - 1):
        data_time.append(data["Time (s)"].iloc[i])
        for name in column_names:
            data_val[name].append(data[name].iloc[i])
        time_diff = data["Time (s)"].iloc[i + 1] - data["Time (s)"].iloc[i]
        if abs(time_diff - time_step) > 1e-9:
            data_time.append(data["Time (s)"].iloc[i] + time_step)
            for name in column_names:
                data_val[name].append(0.0)
    data_time.append(data["Time (s)"].iloc[-1])
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
