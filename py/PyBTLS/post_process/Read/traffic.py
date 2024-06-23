from pybtls.pre_process.GarageProcessing import load_garage_file
from pathlib import Path



def read_traffic(file_path: Path, format:int) -> dict:
    """
    Read the traffic data from pybtls results.

    Parameters
    ----------
    file_path : Path\n
        The path to the traffic data file.

    Returns
    -------
    dict\n
        The traffic data.
    """

    # Read data
    return_data = load_garage_file(file_path)

    return return_data
    
