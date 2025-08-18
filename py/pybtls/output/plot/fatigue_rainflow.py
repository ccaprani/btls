import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_FR"]


def plot_FR(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the fatigue rainflow counting data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded fatigue rainflow data from read_FR.

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
    amplitude = data["Amplitude"]
    no_cycles = data["No. Cycles"]

    # Plotting
    ax.hlines(amplitude, 0, no_cycles, color="gray")
    ax.set_xlabel("No. Cycles")
    ax.set_ylabel("Amplitude")

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
