"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp. 
"""

from ..output.output_config import OutputConfig
from ..lib.BTLS import (
    _VehClassAxle,
    _VehClassPattern,
    _VehicleBuffer,
    _Vehicle,
)
from typing import Literal
from pathlib import Path
import os

__all__ = ["write_garage_file"]


def write_garage_file(
    vehicle_list: list[_Vehicle],
    out_path: Path,
    out_garage_format: Literal[1, 2, 3, 4],
    **kwargs,
) -> None:
    """
    Write a .txt garage file from a list of Vehicle objects.

    Parameters
    ----------
    vehicle_list : list[_Vehicle] \n
        A list of Vehicle objects. \n
    out_path : Path \n
        The path of the output garage file. \n
    out_garage_format : Literal[1,2,3,4] \n
        The format of the output .txt garage file. \n
        1: CASTOR format. \n
        2: BEDIT format. \n
        3: DITIS format. \n
        4: MON format (Recommended).

    Keyword Arguments
    -----------------
    vehicle_class_type : Literal["axle", "pattern"], optional \n
        axle: Categorise vehicle by axle. \n
        pattern (Default): Categorise vehicle by pattern. \n
    """

    current_dir = Path("./").resolve()
    file_name = out_path.name
    absolute_out_dir = (
        Path(out_path).resolve().parent
        if not isinstance(out_path, Path)
        else out_path.resolve().parent
    )

    os.makedirs(absolute_out_dir, exist_ok=True)
    os.chdir(absolute_out_dir)

    config = OutputConfig()
    config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_name=file_name,
        vehicle_file_format=out_garage_format,
    )

    if kwargs.get("vehicle_class_type") == "axle":
        vehicle_classification = _VehClassAxle()
    else:
        vehicle_classification = _VehClassPattern()
    vehicle_buffer = _VehicleBuffer(config, vehicle_classification, 0.0)

    for vehicle in vehicle_list:
        vehicle_buffer.addVehicle(vehicle)

    vehicle_buffer.flushBuffer()

    os.chdir(current_dir)
