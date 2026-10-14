import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

__all__ = ["plot_SV"]

_GAP_FACTOR = 10.0  # a time step this many times the usual one starts a new pass


def _lane_passes(data: pd.DataFrame) -> list:
    """Split a single-vehicle time history into one frame per lane pass.

    The run drives the vehicle across each active lane in turn, and the time
    history holds a row only while the bridge is loaded, so the passes are
    separated by a jump in "Time" rather than by rows of zero effect: a step
    of more than ``_GAP_FACTOR`` times the median step starts a new pass.
    """

    time = data["Time"].to_numpy()
    if time.size < 2:
        return [data]
    step = np.diff(time)
    breaks = np.flatnonzero(step > _GAP_FACTOR * np.median(step)) + 1
    return [
        data.iloc[start:stop]
        for start, stop in zip(
            np.concatenate(([0], breaks)), np.concatenate((breaks, [time.size]))
        )
    ]


def plot_SV(data: dict, save_to: Path = None) -> None:
    """
    Plot the load effects of a single-vehicle simulation, one line per lane
    pass.

    A single-vehicle run drives one vehicle across every active lane, in
    both directions, and is the check on the bridge definition: the shape of
    each curve is the influence line the vehicle travelled over, and the
    differences between the lanes are the lane weights. ``plot_TH`` draws
    the same data as one series per direction, with the passes end to end;
    this separates them.

    Parameters
    ----------
    data : dict\n
        The loaded time history of a single-vehicle simulation, i.e.
        ``read_data("time_history")`` of its output manager: the keys are
        the direction folders ("dir1", "dir2") and the values are the
        DataFrames of read_TH, with columns "Time", "No. Vehicles" and one
        or more "Effect i". The file holds a row only while the bridge is
        loaded, so the passes are told apart by the gap in "Time" between
        them. A subplot is drawn per direction and a line per lane pass,
        numbered in the order the lanes were driven (the
        ``active_lane`` order of ``add_sim``, or lane 1 upwards). The x axis
        is the time since the start of the pass, in seconds, so the passes
        can be compared.

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

    directions = list(data)
    fig, axes = plt.subplots(
        len(directions), 1, sharex=True, figsize=(8, 4 * len(directions))
    )
    if len(directions) == 1:
        axes = [axes]

    for ax, direction in zip(axes, directions):
        frame = data[direction]
        effect_names = [
            name for name in frame.columns if name not in ("Time", "No. Vehicles")
        ]
        for i, one_pass in enumerate(_lane_passes(frame), start=1):
            elapsed = one_pass["Time"] - one_pass["Time"].iloc[0]
            for name in effect_names:
                ax.plot(
                    elapsed,
                    one_pass[name],
                    label=f"Pass {i}, {name}" if len(effect_names) > 1 else f"Pass {i}",
                )
        ax.set_ylabel(direction)
        ax.legend(fontsize=10)
    fig.supxlabel("Time Since the Start of the Pass (s)")
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
