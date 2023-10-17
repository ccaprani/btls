from PyBTLS.lib.BTLS_collections import _VehClassAxle, _VehClassPattern, _VehicleTrafficFile, _Vehicle
__all__ = ['load_garage_file', 'get_gvw_from_garage', 'get_vehicle_length_from_garage']


def load_garage_file(garage_path:str, garage_format:int=4, **kwargs) -> list[_Vehicle]:
    """
    Load a list of :class:`PyBTLS.lib.Vehicle` objects from a .txt garage file.
    
    Parameters
    ----------
    garage_path : str
        The path of the garage file.
    garage_format : int, optional
        The format of the .txt garage file.
        1: CASTOR format.
        2: BEDIT format.
        3: DITIS format.
        4 (Default): MON format.

    Keyword Arguments
    -----------------
    vehicle_class_type : str, optional
        axle: Categorise vehicle by axle.
        pattern (Default): Categorise vehicle by pattern.

    Returns
    -------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    """

    if kwargs.get("vehicle_class_type") == "axle":
        vehicle_classification = _VehClassAxle()
    else:
        vehicle_classification = _VehClassPattern()

    garage_txt = _VehicleTrafficFile(vehicle_classification, False, False, 80.0)
    garage_txt._read(garage_path,garage_format)

    return garage_txt._getVehicles()


def get_gvw_from_garage(vehicle_list:list[_Vehicle]) -> list[float]:
    """
    Get the gross vehicle weights from the list of :class:`PyBTLS.lib.Vehicle` objects.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.

    Returns
    -------
    gvw_list : list[float]
        A list of gross vehicle weights.
    """

    gvw_list = []
    for vehicle in vehicle_list:
        gvw_list.append(vehicle.get_gvw())
    return gvw_list


def get_vehicle_length_from_garage(vehicle_list:list[_Vehicle]) -> list[float]:
    """
    Get the vehicle lengths from the list of :class:`PyBTLS.lib.Vehicle` objects.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.

    Returns
    -------
    vehicle_length_list : list[float]
        A list of vehicle lengths.
    """

    vehicle_length_list = []
    for vehicle in vehicle_list:
        vehicle_length_list.append(vehicle.get_length())
    return vehicle_length_list

