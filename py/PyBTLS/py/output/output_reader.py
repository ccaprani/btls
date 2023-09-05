from PyBTLS.py.output.output import (
    IntervalStatistics,
    CumulativeStatistics,
    BlockMaximaSummary,
)
from PyBTLS.py.vehicle.events import BlockMaximaEvent
from glob import glob
import numpy as np

"""
This file provide functions to import BTLS ouput for a given bridge span.
These functions are for convenience; 
the output file names from BTLS are sometimes inconsistent.
To import directly from a know file path, please use the appropriate Classes directly.
"""

def read_interval_statistics_files(froot, bridge_length):
    """
    Read interval statistics file.
    Contains information such as the average load effect, standard deviation, minimum and maximum of each day (or period) of the simulation.

    Arguments:
    ----------
    froot: str
        Path to the directory containing the output files.
    bridge_length: list of float
        Length of the bridge span.
    
    Returns:
    --------
    list of IntervalStatistics
        The interval statistics object, for each bridge supplied in the `bridge_length` argument.
    """
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
    """
    Read cumulative statistics file.
    Contains information such as the average load effect, standard deviation, minimum and maximum of the entire simulation.

    Arguments:
    ----------
    froot: str
        Path to the directory containing the output files.
    bridge_length: list of float
        Length of the bridge span.

    Returns:
    --------
    list of CumulativeStatistics
        The cumulative statistics object, for each bridge supplied in the `bridge_length` argument.
    """
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
    """
    Read block maxima summary file.
    Contain the maxima of each day (or period), separated by event type.

    Arguments:
    ----------
    froot: str
        Path to the directory containing the output files.
    bridge_length: list of float
        Length of the bridge span.
    
    Returns:
    --------
    list of BlockMaximaSummary
        The block maxima summary object, for each bridge supplied in the `bridge_length` argument.
    """
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
    """
    Read block maxima event file.
    Contains information of the event producing the maximum load effect for each day (or period),
    separated by event type (i.e., number of vehicles on the bridge).
    See `BlockMaximaEvent` for more information.

    Arguments:
    ----------
    froot: str
        Path to the directory containing the output files.
    file_format: str
        Format of the vehicle text file.
        Either "CASTOR", "BeDIT", "DITIS", or "MON".

    Returns:
    --------
    BlockMaximaEvent
        The block maxima event object.
    """
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
    """
    Read block maxima event file.
    Contains information of the event producing the maximum load effect for each day (or period).
    The maxima is not separated by event type (i.e., number of vehicles on the bridge), i.e., it is mixed.
    See `BlockMaximaEvent` for more information.

    Arguments:
    ----------
    froot: str
        Path to the directory containing the output files.
    file_format: str
        Format of the vehicle text file.
        Either "CASTOR", "BeDIT", "DITIS", or "MON".

    Returns:
    --------
    BlockMaximaEvent
        The block maxima event object.
    """
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
