from collections import defaultdict
from types import SimpleNamespace
from PyBTLS.lib.BTLS_collections import _Vehicle
from .region_criteria import *
from .load import get_gvw_from_garage, get_vehicle_length_from_garage
from .write import write_garage_file
__all__ = ['divide_by_regional_classification', 'divide_by_no_axle', 'divide_by_gvw_percentile', 'divide_by_gvw_value', 'divide_by_vehicle_length_percentile', 'divide_by_vehicle_length_value', 'write_divide_garage_file']


class SubWIMDict(SimpleNamespace):
    """
    SubWIMDict is a subclass of SimpleNamespace, which is for storing the data with a tag.

    Attributes
    ----------
    data : defaultdict
        The data attribute stores the divided list of :class:`PyBTLS.lib.Vehicle` objectss.
    tag : str
        The tag attribute indicates the data information.
    """
    
    def __init__(self, data:defaultdict, tag=None):
        """
        Constructor of SubWIMDict.

        Parameters
        ----------
        data : defaultdict
            The data to be stored.
        tag : str, optional
            The tag of the data (Default: None).
        """

        super().__init__(data=data,tag=tag)


def divide_by_regional_classification(vehicle_list:list[_Vehicle], region_criterion:CriterionBase) -> SubWIMDict:
    """
    Divide vehicle garage by regional classification criterion. 

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    region_criterion : CriterionBase
        A CriterionBase object representing the classification criterion (AUCriterion, EUCriterion, etc.).

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    sub_WIM_dict = SubWIMDict(defaultdict(list))

    for vehicle in vehicle_list:
        region_criterion.set_vehicle_property(vehicle)
        sub_WIM_dict.data[region_criterion.get_vehicle_class()].append(vehicle)

    sub_WIM_dict.tag = region_criterion.__class__.__name__

    return sub_WIM_dict


def divide_by_no_axle(vehicle_list:list[_Vehicle]) -> SubWIMDict:
    """
    Divide the vehicle garage by number of vehicle axles. 

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    sub_WIM_dict = SubWIMDict(defaultdict(list))

    for vehicle in vehicle_list:
        sub_WIM_dict.data[str(vehicle.get_no_axles())].append(vehicle)
    sub_WIM_dict.tag = "NoAxle"

    return sub_WIM_dict


def divide_by_gvw_percentile(vehicle_list:list[_Vehicle], gvw_div_percentile_list:list) -> SubWIMDict:  # np.percentile() is based on the raw data pdf, not the max and min values (Should be based on the max and min values)!
    """
    Divide the vehicle garage by the percentiles of gross vehicle weights. 

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    gvw_div_percentile_list : list[float]
        A list of percentiles (in decimal) for division.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    gvw_list = get_gvw_from_garage(vehicle_list)
    gvw_div_value_list = _percentile_to_value(gvw_div_percentile_list, gvw_list)

    sub_WIM_dict = divide_by_gvw_value(vehicle_list, gvw_div_value_list, gvw_list=gvw_list)

    return sub_WIM_dict


def divide_by_gvw_value(vehicle_list:list[_Vehicle], gvw_div_value_list:list, **kwargs) -> SubWIMDict:
    """
    Divide the vehicle garage by the values of gross vehicle weights.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    gvw_div_value_list : list[float]
        A list of gvw values for division.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    if "gvw_list" in kwargs.keys():
        gvw_list = kwargs.get("gvw_list")
    else:
        gvw_list = get_gvw_from_garage(vehicle_list)

    sub_WIM_dict = _divide_by_value(vehicle_list, gvw_list, gvw_div_value_list)
    sub_WIM_dict.tag = "GVW"

    return sub_WIM_dict


def divide_by_vehicle_length_percentile(vehicle_list:list[_Vehicle], vehicle_length_div_percentile_list:list) -> SubWIMDict:  # np.percentile() is based on the raw data pdf, not the max and min values (Should be based on the max and min values)!
    """
    Divide the vehicle garage by the percentiles of vehicle lengths. 

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    vehicle_length_div_percentile_list : list[float]
        A list of percentiles (in decimal) for division.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    vehicle_length_list = get_vehicle_length_from_garage(vehicle_list)
    vehicle_length_div_value_list = _percentile_to_value(vehicle_length_div_percentile_list, vehicle_length_list)

    sub_WIM_dict = divide_by_vehicle_length_value(vehicle_list, vehicle_length_div_value_list, vehicle_length_list=vehicle_length_list)

    return sub_WIM_dict


def divide_by_vehicle_length_value(vehicle_list:list[_Vehicle], vehicle_length_div_value_list:list, **kwargs) -> SubWIMDict:
    """
    Divide the vehicle garage by the values of vehicle lengths. 

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    vehicle_length_div_value_list : list[float]
        A list of vehicle length values for division.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    """

    if "vehicle_length_list" in kwargs.keys():
        vehicle_length_list = kwargs.get("vehicle_length_list")
    else:
        vehicle_length_list = get_vehicle_length_from_garage(vehicle_list)

    sub_WIM_dict = _divide_by_value(vehicle_list, vehicle_length_list, vehicle_length_div_value_list)
    sub_WIM_dict.tag = "VehicleLength"

    return sub_WIM_dict


def _percentile_to_value(percentile_list:list[float], data:list[float]) -> list[float]:
    """
    Convert the percentile list to value list. 

    Parameters
    ----------
    percentile_list : list[float]
        A list of percentiles (in decimal).
    data : list[float]
        A list of source data.

    Returns
    -------
    value_list : list[float]
        A list of values from the data corresponding to the percentiles.
    """
    
    data_min = min(data)
    data_max = max(data)

    value_list = [i*(data_max-data_min)+data_min for i in percentile_list]

    return value_list


def _divide_by_value(vehicle_list:list[_Vehicle], vehicle_property_list:list[float], div_value_list:list) -> SubWIMDict:
    """
    Divide the vehicle garage by the specified values.

    Parameters
    ----------
    vehicle_list : list[Vehicle]
        A list of :class:`PyBTLS.lib.Vehicle` objects.
    vehicle_property_list : list[float]
        A list of vehicle property values.
    div_value_list : list[float]
        A list of values for division.

    Returns
    -------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss (without tag).
    """

    vehicle_list_copy = vehicle_list.copy()
    vehicle_property_list_copy = vehicle_property_list.copy()
    sub_WIM_dict = SubWIMDict(defaultdict(list))

    div_value_list = sorted(div_value_list)
    for div_value in div_value_list:
        gvw_index = [i for i, gvw in enumerate(vehicle_property_list_copy) if gvw <= div_value]
        for j in sorted(gvw_index, reverse=True):
            sub_WIM_dict.data["under_"+str(div_value)].append(vehicle_list_copy[j])
            vehicle_list_copy.pop(j)
            vehicle_property_list_copy.pop(j)

    return sub_WIM_dict


def write_divide_garage_file(sub_WIM_dict:SubWIMDict, output_folder_path:str, out_garage_format:int=4, **kwargs) -> None:
    """
    Write the divided list of :class:`PyBTLS.lib.Vehicle` objectss from sub_WIM_dict to a specified folder. 

    Parameters
    ----------
    sub_WIM_dict : SubWIMDict
        A SubWIMDict object containing the divided list of :class:`PyBTLS.lib.Vehicle` objectss and the tag of the criterion.
    output_folder_path : str
        The path of the folder to save the output divided garages.
    out_garage_format : int, optional
        The format of the output .txt garage files.

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

    for key in sub_WIM_dict.data.keys():
        write_garage_file(sub_WIM_dict.data[key],output_folder_path+"/"+sub_WIM_dict.tag+"_"+key,out_garage_format,**kwargs)

    return None

