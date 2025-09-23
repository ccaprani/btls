import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_AE"]


def plot_AE(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the all events data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded all events from read_AE.

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

    # Plotting
    for ax, name in zip(axes, column_names):
        ax.vlines(data["Start Time"], 0, data[name], color="gray")
        ax.set_ylabel(name)  # The amplitude is the maximum during the event
    fig.supxlabel(
        "Start Time (s)"
    )  # Use the event start time to align each load effect maximum (but it is not the maximum occurring moment)

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
