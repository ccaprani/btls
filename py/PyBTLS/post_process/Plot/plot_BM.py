import pandas as pd
import matplotlib.pyplot as plt

__all__ = ["plot_BM"]


def plot_BM(file_path: str, plot_save_path: str = None) -> None:
    """
    Plot the block maximum data from pybtls results.

    Parameters
    ----------
    filename : str\n
        The name of the file to read data from.

    save_path : str, optional\n
        The path to save the plot to. If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """
    plt.figure()

    # Read data
    TH_data = pd.read_csv(
        file_path, sep="[\s\t]+", header=None, skiprows=1, engine="python"
    )
    no_load_effects = len(TH_data.columns) - 1

    # Set column ids
    column_ids = ["Block Index"]
    for i in range(no_load_effects):
        load_effect_id = str(i + 1) + "-Truck Event"
        column_ids.append(load_effect_id)
    TH_data.columns = column_ids

    # Pick the maximum value
    max_values = TH_data.max(axis=1)

    # Determine the number of bins
    band_width = (
        2
        * (max_values.quantile(0.75) - max_values.quantile(0.25))
        * len(max_values) ** (-1 / 3)
    )
    no_bins = int((max_values.max() - max_values.min()) / band_width)
    # IQR = (3rd quantile - 1st quantile); band_width = 2*IQR*𝑛^(−1/3); no_bins = (max−min)/band_width

    # Plotting
    load_effect_id = file_path.split(".txt")[0].split("_")[-1]
    plt.hist(max_values, bins=no_bins, label="Load Effect " + load_effect_id)

    # Adding title and labels
    plt.title("Block Maximum Data")
    plt.xlabel("Load Effects")
    plt.ylabel("Count")
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
