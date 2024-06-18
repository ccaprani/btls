import random
from collections import defaultdict
from pybtls.lib.BTLS_collections import _Vehicle
from .load import load_garage_file
__all__ = ['assemble_garage']


def assemble_garage(source_garage_path:list[str], source_garage_percentage:list[float], source_garage_format:int=4, garage_size:int=1000000, **kwargs) -> list[_Vehicle]:
    """
    Assemble a list of :class:`pybtls.lib.Vehicle` objects from the source .txt garage files

    Parameters
    ----------
    source_garage_path : list[str]
        A list with the path of source garage files.
    source_garage_percentage : list[float]
        A list with the percentages of source garage files.
    source_garage_format : int, optional
        The format of the .txt source garage files.

            - 1: CASTOR format.
            - 2: BEDIT format.
            - 3: DITIS format.
            - 4 (Default): MON format.
    garage_size : int, optional
        The number of vehicles in the generated list (Default: 1000000).

    Keyword Arguments
    -----------------
    vehicle_class_type : str, optional
    
        - axle: Categorise vehicle by axle.
        - pattern (Default): Categorise vehicle by pattern.

    Returns
    -------
    return_garage : list[Vehicle]
        The assembled list of :class:`pybtls.lib.Vehicle` objects.
    """

    if len(source_garage_path) != len(source_garage_percentage):
        raise ValueError("The elements of source_garage_path and source_garage_percentage must be equal!")
    if abs(sum(source_garage_percentage)-1.0) > 0.01:
        raise ValueError("The sum of source_garage_percentage should be 1.0!")
    
    source_garage_dict = defaultdict(list)
    source_garage_no_vehicle_dict = defaultdict(int)
    return_garage = []

    for i in range(len(source_garage_path)):
        source_garage_dict[source_garage_path[i]] = load_garage_file(source_garage_path[i],source_garage_format,**kwargs)
        source_garage_no_vehicle_dict[source_garage_path[i]] = int(garage_size*source_garage_percentage[i])

    for i in range(len(source_garage_path)):
        for _ in range(source_garage_no_vehicle_dict[source_garage_path[i]]):
            return_garage.append(random.choice(source_garage_dict[source_garage_path[i]]))

    return return_garage

