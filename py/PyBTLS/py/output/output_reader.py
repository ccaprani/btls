from PyBTLS.py.output.output import (
    IntervalStatistics,
    CumulativeStatistics,
    BlockMaximaSummary,
)
from PyBTLS.py.vehicle.events import BlockMaximaEvent
from glob import glob
import numpy as np

"""
Provide functions to import BTLS ouput for a given bridge span.

"""


def read_interval_statistics_files(froot, bridge_length):
    fpath = np.array(glob(froot + "/SS_S_*_Eff_*.txt"))
    # Get the span
    f_span = np.array([f[len(froot) + 5 :].split("_")[0] for f in fpath])
    # Filter out the invalid spans
    filter = [np.isclose(float(f_s), bridge_length) for f_s in f_span]
    fpath = fpath[filter]
    if len(fpath) == 0:
        raise ValueError(
            "ERROR: No interval statistics file(s) was found in the `froot` directory for the `bridge_length` given"
        )
    out = []
    for i, f in enumerate(fpath):
        # Get LE number
        LE_num = str(int(f[len(froot) + 5 :].split("_")[2].split(".")[0]))
        out.append(
            IntervalStatistics(
                descriptor="Load Effect " + str(LE_num),
                path=f,
            )
        )
    return out


def read_cumulative_statistics_file(froot, bridge_length):
    fpath = np.array(glob(froot + "/SS_C_*.txt"))
    # Get the span
    f_span = np.array([f[len(froot) + 5 :].split("_")[0].split(".")[0] for f in fpath])
    # Filter out the invalid spans
    filter = [np.isclose(float(f_s), bridge_length) for f_s in f_span]
    fpath = fpath[filter]
    if len(fpath) == 0:
        raise ValueError(
            "ERROR: No cumulative statistics file was found in the `froot` directory for the `bridge_length` given"
        )
    return CumulativeStatistics(path=fpath[0])


def read_block_maxima_summary_file(froot, bridge_length):
    fpath = np.array(glob(froot + "/BM_S_*_Eff_*.txt"))
    # Get the span
    f_span = np.array([f[len(froot) + 5 :].split("_")[0] for f in fpath])
    # Filter out the invalid spans
    filter = [np.isclose(float(f_s), bridge_length) for f_s in f_span]
    fpath = fpath[filter]
    if len(fpath) == 0:
        raise ValueError(
            "ERROR: No block maxima summary file(s) was found in the `froot` directory for the `bridge_length` given"
        )
    out = []
    for i, f in enumerate(fpath):
        # Get LE number
        LE_num = str(int(f[len(froot) + 5 :].split("_")[2].split(".")[0]))
        out.append(
            BlockMaximaSummary(
                descriptor="Load Effect " + str(LE_num),
                path=f,
            )
        )
    return out


def read_block_maxima_event_separated_vehicles_file(froot, bridge_length, file_format):
    fpath = np.array(glob(froot + "/BM_V_*_*.txt"))
    # Remove the one with All in the filename
    filter = [f.replace(froot, "") for f in fpath]
    filter = [False if "All" in s else True for s in filter]
    fpath = fpath[filter]
    # Get the span, filter out the invalid spans
    f_span = np.array([f[len(froot) + 5 :].split("_")[0] for f in fpath])
    filter = [np.isclose(float(f_s), bridge_length) for f_s in f_span]
    fpath = fpath[filter]
    if len(fpath) == 0:
        raise ValueError(
            "ERROR: No block maxima separated by event file(s) was found in the `froot` directory for the `bridge_length` given"
        )
    out = []
    for i, f in enumerate(fpath):
        out.append(BlockMaximaEvent(path=f, file_format=file_format))
    return out


def read_block_maxima_event_mixed_vehicles_file(froot, bridge_length, file_format):
    fpath = np.array(glob(froot + "/BM_V_*_All.txt"))
    # Get the span, filter out the invalid spans
    f_span = np.array([f[len(froot) + 5 :].split("_")[0] for f in fpath])
    filter = [np.isclose(float(f_s), bridge_length) for f_s in f_span]
    fpath = fpath[filter]
    if len(fpath) == 0:
        raise ValueError(
            "ERROR: No mixed block maxima file was found in the `froot` directory for the `bridge_length` given"
        )
    return BlockMaximaEvent(path=fpath[0], file_format=file_format)
