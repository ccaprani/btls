from pybtls.lib import OutputConfig
from pybtls.lib.BTLS_collections import _VehClassAxle, _VehClassPattern, _VehicleBuffer, _Vehicle
__all__ = ['write_garage_file']


def write_garage_file(vehicle_list:list[_Vehicle], out_garage_path:str, out_garage_format:int=4, **kwargs) -> None:
    """
    Write a .txt garage file from the list of :class:`pybtls.lib.Vehicle` objects.
    
    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`pybtls.lib.Vehicle` objects.
    out_garage_path : str
        The path of the output garage file.
    out_garage_format : int, optional
        The format of the output .txt garage file.

            - 1: CASTOR format.
            - 2: BEDIT format.
            - 3: DITIS format.
            - 4 (Default): MON format.

    Keyword Arguments
    -----------------
    vehicle_class_type : str, optional
    
        - axle: Categorise vehicle by axle.
        - pattern (Default): Categorise vehicle by pattern.

    Returns
    -------
    None.
    """

    config = OutputConfig()
    config.set_vehicle_file_output(write_vehicle_file=True,vehicle_file_name=out_garage_path+".txt",vehicle_file_format=out_garage_format)
    
    if kwargs.get("vehicle_class_type") == "axle":
        vehicle_classification = _VehClassAxle()
    else:
        vehicle_classification = _VehClassPattern()
    vehicle_buffer = _VehicleBuffer(config,vehicle_classification,0.0)

    for vehicle in vehicle_list:
        vehicle_buffer.addVehicle(vehicle)

    vehicle_buffer.flushBuffer()
    return None

