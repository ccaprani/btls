import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_BM_S"]


def plot_BM_S(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the BM summary data from pybtls results.

    One subplot per truck-count bucket, each showing vertical lines from
    0 to the bucket's load effect value against the block index.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded BM summary from read_BM_S. Must have columns
        "Block Index" and one or more "{i}-Truck Event" columns
        (i = 1, 2, ...); a subplot is drawn for every column after the
        first, so a DataFrame with only "Block Index" raises an error
        from ``plt.subplots``. NaN bucket values are filled with 0.0
        before plotting. The y-axis is labelled "Effect Amplitude" with
        no unit (the "{i}-Truck Event" columns are load effect values in
        kN or kN·m, per read_BM_S; the unit is not known to this
        function).

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

    data = data.fillna(0.0)
    no_event_types = len(data.columns) - 1

    fig, axes = plt.subplots(
        no_event_types, 1, sharex=True, figsize=(8, 5 * no_event_types)
    )
    if no_event_types == 1:
        axes = [axes]

    # Use the Block Index column as the x axis
    the_index = data["Block Index"]

    # Plotting
    for ax, i in zip(axes, range(1, no_event_types + 1)):
        ax.vlines(the_index, 0, data[f"{i}-Truck Event"], color="gray")
        ax.set_ylabel(f"{i}-Truck Event")
    fig.supxlabel("Block Index")
    fig.supylabel("Effect Amplitude")

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
