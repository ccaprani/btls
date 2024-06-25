"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp. 
"""

from ..lib.BTLS import (
    _VehClassAxle,
    _VehClassPattern,
    _VehicleTrafficFile,
    _Vehicle,
)
from pathlib import Path
from typing import Literal

__all__ = ["read_garage_file"]


def read_garage_file(
    garage_path: Path, garage_format: Literal[1, 2, 3, 4], **kwargs
) -> list[_Vehicle]:
    """
    Read a .txt garage file.

    Parameters
    ----------
    garage_path : Path \n
        The path of the garage file.

    garage_format : Literal[1,2,3,4] \n
        The format of the .txt garage file. \n
        1: CASTOR format. \n
        2: BEDIT format. \n
        3: DITIS format. \n
        4: MON format.

    Keyword Arguments
    -----------------
    vehicle_class_type : Literal["axle", "pattern"], optional \n
        axle: Categorise vehicle by axle. \n
        pattern (Default): Categorise vehicle by pattern.

    Returns
    -------
    vehicle_list : list[_Vehicle] \n
        A list of Vehicle objects.
    """

    if kwargs.get("vehicle_class_type") == "axle":
        vehicle_classification = _VehClassAxle()
    else:
        vehicle_classification = _VehClassPattern()

    garage_path = (
        Path(garage_path) if not isinstance(garage_path, Path) else garage_path
    )

    garage_txt = _VehicleTrafficFile(vehicle_classification, False, False, 80.0)
    garage_txt.read(garage_path, garage_format)

    return garage_txt.getVehicles()
