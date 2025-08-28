import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_POT_S"]


def plot_POT_S(data: pd.DataFrame, threshold: float, save_to: Path = None) -> None:
    """
    Plot the POT summary data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded POT summary from read_POT_S.

    threshold : float\n
        The POT threshold value.

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

    # Pick the data
    the_index = data["Peak Index"][data["Peak Value"] >= threshold]
    the_data = data["Peak Value"][data["Peak Value"] >= threshold]

    # Plotting
    ax.vlines(the_index, threshold, the_data, color="gray")
    ax.set_xlabel("Peak index")
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
