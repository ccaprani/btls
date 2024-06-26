import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_BM_S"]


def plot_BM_S(data: pd.DataFrame, save_as: Path = None) -> None:
    """
    Plot the BM summary data from pybtls results.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded BM summary from read_TH.

    save_as : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    plt.figure()

    data_columns = data.iloc[:, 1:]

    # Pick the maximum value
    max_values = data_columns.max(axis=1)

    # Determine the number of bins
    band_width = (
        2
        * (max_values.quantile(0.75) - max_values.quantile(0.25))
        * len(max_values) ** (-1 / 3)
    )
    no_bins = int((max_values.max() - max_values.min()) / band_width)
    # IQR = (3rd quantile - 1st quantile); band_width = 2*IQR*𝑛^(−1/3); no_bins = (max−min)/band_width

    # Plotting
    plt.hist(max_values, bins=no_bins)

    # Adding title and labels
    plt.title("Block Maximum Data")
    plt.xlabel("Effect Amplitude")
    plt.ylabel("Count")
    plt.legend()

    # Save or display the plot
    if save_as is not None:
        plt.savefig(
            save_as,
            format="png",
            dpi=500,
            pad_inches=0.1,
            bbox_inches="tight",
        )
    else:
        plt.show()
