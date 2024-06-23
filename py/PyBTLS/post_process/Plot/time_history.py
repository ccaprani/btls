import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_TH"]


def plot_TH(data: pd.DataFrame, plot_save_path: Path = None) -> None:
    """
    Plot the time history data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded time history from read_TH.

    save_path : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    plt.figure()

    column_names = data.columns.tolist()[2:]

    # Plotting
    for name in column_names:
        plt.plot(data["Time (s)"], data[name], label=name)

    # Adding title and labels
    plt.title("Time History Data")
    plt.xlabel("Time (s)")
    plt.ylabel("Effects")
    plt.legend()

    # Save or display the plot
    if plot_save_path is not None:
        plt.savefig(
            plot_save_path + ".png",
            format="png",
            dpi=500,
            pad_inches=0.1,
            bbox_inches="tight",
        )
    else:
        plt.show()
