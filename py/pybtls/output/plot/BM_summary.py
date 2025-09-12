import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_BM_S"]


def plot_BM_S(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the BM summary data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded BM summary from read_BM_S.

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

    fig, ax = plt.subplots(figsize=(8, 5))

    # Pick the maximum value among different no_truck events
    the_index = data["Block Index"]
    the_data = data.iloc[:, 1:]
    max_data = the_data.max(axis=1)

    # Plotting
    ax.vlines(the_index, 0, max_data, color="gray")
    ax.set_xlabel("Block index")
    ax.set_ylabel("Effect amplitude")

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
