from .BTLS_collections import _LaneFlowComposition
from typing import Literal
__all__ = ["LaneFlowComposition"]


class LaneFlowComposition():
    
    def __init__(self, lane_index:int, lane_dir:Literal[1,2], no_block:int=24, block_size:int=3600, tag:str="Now"):
        """
        The LaneFlowComposition instance stores the data for creating a CLaneFlowComposition instance in C++.
        Lane flow composition. The hourly flow, speed, and truck composition data for a lane.

        Parameters
        ----------
        lane_index : int\n
            The index of the lane. 1-based global index.

        lane_dir : Literal[1,2]\n
            The direction of the lane. 

        no_block : int, optional\n
            The number of blocks in a day. The default is 24.

        block_size : int, optional\n
            The size of a block in seconds. The default is 3600.

        tag : str, optional\n
            To tag the moment of the lane flow composition. The default is "Now".
        """

        self._tag = tag

        self._lane_index = lane_index  # at here it is still 1-based
        self._lane_dir = lane_dir
        self._no_block = no_block
        self._block_size = block_size

        self._hourly_truck_flow = [1]*no_block  # in veh/h
        self._hourly_car_percentage = [0.0]*no_block  # in percent
        self._hourly_speed_mean = [248]*no_block  # in dm/s
        self._hourly_speed_std = [10]*no_block  # in dm/s
        self._hourly_truck_composition = [[100.0] for _ in range(no_block)]  # in percent

        self._flow_assigned = False  # garage, grave, nominal need hourly flow
        self._speed_assigned = False  # NHM, freeflow need hourly speed
        self._truck_composition_assigned = False  # grave need hourly truck percentage

    def __getstate__(self):
        attribute_dict = {}
        attribute_dict["tag"] = self._tag
        attribute_dict["lane_index"] = self._lane_index
        attribute_dict["lane_dir"] = self._lane_dir
        attribute_dict["no_block"] = self._no_block
        attribute_dict["block_size"] = self._block_size
        attribute_dict["hourly_truck_flow"] = self._hourly_truck_flow
        attribute_dict["hourly_car_percentage"] = self._hourly_car_percentage
        attribute_dict["hourly_speed_mean"] = self._hourly_speed_mean
        attribute_dict["hourly_speed_std"] = self._hourly_speed_std
        attribute_dict["hourly_truck_composition"] = self._hourly_truck_composition
        attribute_dict["flow_assigned"] = self._flow_assigned
        attribute_dict["speed_assigned"] = self._speed_assigned
        attribute_dict["truck_composition_assigned"] = self._truck_composition_assigned
        return attribute_dict
    
    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._lane_index = attribute_dict["lane_index"]
        self._lane_dir = attribute_dict["lane_dir"]
        self._no_block = attribute_dict["no_block"]
        self._block_size = attribute_dict["block_size"]
        self._hourly_truck_flow = attribute_dict["hourly_truck_flow"]
        self._hourly_car_percentage = attribute_dict["hourly_car_percentage"]
        self._hourly_speed_mean = attribute_dict["hourly_speed_mean"]
        self._hourly_speed_std = attribute_dict["hourly_speed_std"]
        self._hourly_truck_composition = attribute_dict["hourly_truck_composition"]
        self._flow_assigned = attribute_dict["flow_assigned"]
        self._speed_assigned = attribute_dict["speed_assigned"]
        self._truck_composition_assigned = attribute_dict["truck_composition_assigned"]

    @property
    def tag(self) -> str:
        return self._tag

    @property
    def lane_index(self) -> int:
        return self._lane_index
    
    @property
    def lane_dir(self) -> int:
        return self._lane_dir

    @property
    def flow_assigned(self) -> bool:
        return self._flow_assigned
    
    @property
    def speed_assigned(self) -> bool:
        return self._speed_assigned
    
    @property
    def truck_composition_assigned(self) -> bool:
        return self._truck_composition_assigned

    def assign_lane_data(self, **kwargs) -> None:
        """
        Assign the required data to the lane for traffic generation.

        Keyword Arguments
        -----------------
        hourly_truck_flow : list[float]\n
            The hourly truck flow data in veh/h.

        hourly_car_flow : list[float]\n
            The hourly car flow data in veh/h.

        hourly_car_percentage : list[float]\n
            The hourly car percentage data in percent.

        hourly_speed_mean : list[float]\n
            The hourly mean speed data in dm/s.

        hourly_speed_std : list[float]\n
            The hourly standard deviation of speed data in dm/s.

        hourly_truck_composition : list[list[float]]\n
            The hourly truck composition data in percent. The list should only consist of four truck percentages.

        Returns
        -------
        None.
        """

        if kwargs.get("hourly_truck_flow") is not None and kwargs.get("hourly_car_flow") is not None:
            self._set_hourly_flow1(kwargs.get("hourly_truck_flow"), kwargs.get("hourly_car_flow"))
        elif kwargs.get("hourly_truck_flow") is not None and kwargs.get("hourly_car_percentage") is not None:
            self._set_hourly_flow2(kwargs.get("hourly_truck_flow"), kwargs.get("hourly_car_percentage"))

        if kwargs.get("hourly_speed_mean") is not None and kwargs.get("hourly_speed_std") is not None:
            self._set_hourly_speed(kwargs.get("hourly_speed_mean"), kwargs.get("hourly_speed_std"))

        if kwargs.get("hourly_truck_composition") is not None:
            self._set_hourly_truck_composition(kwargs.get("hourly_truck_composition"))

    def _set_hourly_flow1(self, hourly_truck_flow, hourly_car_flow):

        if len(hourly_truck_flow) != self._no_block or len(hourly_car_flow) != self._no_block:
            raise ValueError("Length of hourly flow data does not match.")
        
        self._hourly_truck_flow = hourly_truck_flow
        for i in range(self._no_block):
            self._hourly_car_percentage[i] = hourly_car_flow[i]/(hourly_car_flow[i]+hourly_truck_flow[i])*100

        self._flow_assigned = True

    def _set_hourly_flow2(self, hourly_truck_flow, hourly_car_percentage):

        if len(hourly_truck_flow) != self._no_block or len(hourly_car_percentage) != self._no_block:
            raise ValueError("Length of hourly flow data does not match.")
        
        self._hourly_truck_flow = hourly_truck_flow
        self._hourly_car_percentage = hourly_car_percentage

        self._flow_assigned = True

    def _set_hourly_speed(self, hourly_speed_mean, hourly_speed_std):
        
        if len(hourly_speed_mean) != self._no_block or len(hourly_speed_std) != self._no_block:
            raise ValueError("Length of hourly speed data does not match.")
        
        self._hourly_speed_mean = hourly_speed_mean
        self._hourly_speed_std = hourly_speed_std

        self._speed_assigned = True

    def _set_hourly_truck_composition(self, hourly_truck_composition:list[list[float]]):

        if not isinstance(hourly_truck_composition[0], list):
            raise TypeError("Hourly Truck composition data should be a list of lists.")

        if len(hourly_truck_composition) != self._no_block:
            raise ValueError("Length of hourly Truck composition data does not match.")
        
        if not all(len(sublist) == 4 for sublist in hourly_truck_composition):
            raise ValueError("The hourly truck composition data should only consist of four truck percentages.")
        
        self._hourly_truck_composition = hourly_truck_composition
        
        self._truck_composition_assigned = True

    def _get_LFC(self):
        """
        Get a CLaneFlowComposition instance (lane flow composition).
        """

        lfc = _LaneFlowComposition(self._lane_index-1, self._lane_dir, self._block_size)  # in C++ BTLS, lane_index is 0-based when create CLFC in CReadLaneData
        
        for i in range(self._no_block):
            data_vector = []

            data_vector.append(float(i))
            data_vector.append(self._hourly_truck_flow[i])
            data_vector.append(self._hourly_speed_mean[i])
            data_vector.append(self._hourly_speed_std[i])
            data_vector.append(self._hourly_car_percentage[i])
            for j in range(len(self._hourly_truck_composition[i])):
                data_vector.append(self._hourly_truck_composition[i][j])
            
            lfc._addBlockData(data_vector)  # C++ function

        lfc._completeData()  # C++ function

        return lfc
        
