import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_TS"]


def plot_TS(data: pd.DataFrame, save_to: Path = None) -> None:
    """
    Plot the traffic statistics data from pybtls results.

    Two subplots against the hour of the simulation: the vehicle, truck and
    car counts, and the composition by vehicle class as stacked bars. This
    is the check on generated traffic - whether the flow and the mix that
    came out match the LaneFlowComposition that went in.

    Parameters
    ----------
    data : pd.DataFrame\n
        The loaded traffic statistics from read_TS. Must have columns
        "Hour", "No. Vehicles", "No. Trucks" and "No. Cars"; every further
        column is taken as a vehicle class count ("0: Default", "1: Car",
        "2: 2-axle" ... for the axle classifier, "2: Pattern 11" ... for
        the pattern classifier) and is drawn in the lower subplot.

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

    hour = data["Hour"]
    class_names = [
        name
        for name in data.columns
        if name not in ("Hour", "No. Vehicles", "No. Trucks", "No. Cars")
    ]

    fig, axes = plt.subplots(2, 1, sharex=True, figsize=(8, 8))

    # distinct colours as well as styles: the truck and vehicle counts
    # coincide wherever the traffic holds no cars
    for name, style, colour in (
        ("No. Vehicles", "-", "black"),
        ("No. Trucks", "--", "tab:red"),
        ("No. Cars", ":", "tab:blue"),
    ):
        axes[0].plot(hour, data[name], style, color=colour, label=name)
    axes[0].set_ylabel("Count")
    axes[0].legend()

    bottom = pd.Series(0.0, index=data.index)
    for name in class_names:
        axes[1].bar(hour, data[name], bottom=bottom, width=1.0, label=name)
        bottom = bottom + data[name].fillna(0.0)
    axes[1].set_ylabel("Count by Class")
    axes[1].set_xlabel("Hour")
    axes[1].legend(fontsize=10, ncol=2)

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
