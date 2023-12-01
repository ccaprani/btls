from .vehicle_generator import VehicleGenNominal, VehicleGenGrave, VehicleGenGarage
from .headway_generator import HeadwayGenNHM, HeadwayGenConstant, HeadwayGenCongested, HeadwayGenFreeflow
from .lfc import LaneFlowComposition
from .BTLS_collections import _ConfigData, _TrafficGenerator
from typing import Union
__all__ = ["TrafficGenerator"]


class Lane():

    def __init__(self, vehicle_gen:Union[VehicleGenNominal, VehicleGenGrave, VehicleGenGarage], headway_gen:Union[HeadwayGenNHM, HeadwayGenConstant, HeadwayGenCongested, HeadwayGenFreeflow], lfc:LaneFlowComposition):
        """
        The Lane instance sotres the information creating a traffic generator for a single lane.
        """
        
        self._index = lfc.lane_index  # 1-based global here
        self._dir = lfc.lane_dir
        self._vehicle_gen = vehicle_gen
        self._headway_gen = headway_gen
        self._lfc = lfc
        self._start_time = 0.0

        self._tag = str(lfc.lane_index)+str(lfc.lane_dir)+vehicle_gen.tag+headway_gen.tag+lfc.tag

    def __getstate__(self):
        
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["index"] = self._index
        attribute_dict["dir"] = self._dir
        attribute_dict["vehicle_gen"] = self._vehicle_gen
        attribute_dict["headway_gen"] = self._headway_gen
        attribute_dict["lfc"] = self._lfc
        attribute_dict["start_time"] = self._start_time

        return attribute_dict
    
    def __setstate__(self, attribute_dict):

        self._tag = attribute_dict["tag"]
        self._index = attribute_dict["index"]
        self._dir = attribute_dict["dir"]
        self._vehicle_gen = attribute_dict["vehicle_gen"]
        self._headway_gen = attribute_dict["headway_gen"]
        self._lfc = attribute_dict["lfc"]
        self._start_time = attribute_dict["start_time"]

    @property
    def tag(self) -> str:
        return self._tag
    
    @property
    def index(self) -> int:
        return self._index
    
    @property
    def dir(self) -> int:
        return self._dir
    
    @property
    def vehicle_gen(self) -> Union[VehicleGenNominal, VehicleGenGrave, VehicleGenGarage]:
        return self._vehicle_gen
    
    @property
    def headway_gen(self) -> Union[HeadwayGenNHM, HeadwayGenConstant, HeadwayGenCongested, HeadwayGenFreeflow]:
        return self._headway_gen
    
    @property
    def lfc(self) -> LaneFlowComposition:
        return self._lfc
    
    @property
    def start_time(self) -> float:
        return self._start_time
    
    def set_start_time(self, start_time:float) -> None:
        self._start_time = start_time


# 实际上lfc中所记录的数据，除truck composition之外，vehicle generator中并未读取。vehicle generator是通过其update()方法来接收headway model data中读取的lfc的相关数据...
# Veh 和 Headway Gens 都不记录lfc中的关于lane的方向数据.
class TrafficGenerator(): 

    def __init__(self, no_lane:int):
        """
        The TrafficGenerator instance stores the information for creating CTrafficGenerator instances for each lane.

        Parameters
        ----------
        no_lane : int\n
            The number of lanes on the bridge.
        """

        self._tag = ""

        self._no_lane = no_lane
        self._lane_count = 0
        self._no_dir = None
        self._no_lane_dir_1 = None
        self._no_lane_dir_2 = None
        self._vehicle_classifier = 1

        self._lanes:list[Lane] = [None]*no_lane

    def __getstate__(self):
        
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["no_lane"] = self._no_lane
        attribute_dict["no_dir"] = self._no_dir
        attribute_dict["no_lane_dir_1"] = self._no_lane_dir_1
        attribute_dict["no_lane_dir_2"] = self._no_lane_dir_2
        attribute_dict["vehicle_classifier"] = self._vehicle_classifier
        attribute_dict["lanes"] = self._lanes

        return attribute_dict
    
    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._no_lane = attribute_dict["no_lane"]
        self._no_dir = attribute_dict["no_dir"]
        self._no_lane_dir_1 = attribute_dict["no_lane_dir_1"]
        self._no_lane_dir_2 = attribute_dict["no_lane_dir_2"]
        self._vehicle_classifier = attribute_dict["vehicle_classifier"]
        self._lanes = attribute_dict["lanes"]

    def add_lane(self, vehicle_gen:Union[VehicleGenNominal, VehicleGenGrave, VehicleGenGarage], headway_gen:Union[HeadwayGenNHM, HeadwayGenConstant, HeadwayGenCongested, HeadwayGenFreeflow], lfc:LaneFlowComposition) -> None:
        """
        Add information for a lane to set its traffic generator.

        Parameters
        ----------
        vehicle_gen : Union[VehicleGenNominal, VehicleGenGrave, VehicleGenGarage]\n
            The vehicle generator for the lane.

        headway_gen : Union[HeadwayGenNHM, HeadwayGenConstant, HeadwayGenCongested, HeadwayGenFreeflow]\n
            The headway generator for the lane.

        lfc : LaneFlowComposition\n
            The lane flow composition for the lane.

        Returns
        -------
        None.
        """

        vehicle_gen._check_lfc(lfc)
        headway_gen._check_lfc(lfc)

        lane = Lane(vehicle_gen, headway_gen, lfc)
        self._tag += lane.tag

        self._lanes[self._lane_count] = lane
        self._lane_count += 1
        
        if self._lane_count == self._no_lane:
            self._set_lane_properties()

    def set_start_time(self, start_time:Union[float,list[float]]) -> None:
        """
        Set the start time for the traffic generators.

        Parameters
        ----------
        start_time : Union[float,list[float]]\n
            The start time for the traffic generators. If a float is provided, all traffic generators will have the same start time. If a list of float is provided, the length of the list should be equal to the number of lanes on the bridge, and each traffic generator will have the corresponding start time.

        Returns
        -------
        None.
        """

        if isinstance(start_time, float):
            for lane in self._lanes:
                lane.set_start_time(start_time)
        elif isinstance(start_time, list) and len(start_time) == self._no_lane:
            for index, time in enumerate(start_time):
                self._lanes[index].set_start_time(time)
        else:
            raise ValueError("start_time should be either a float or a list of float with length equal to no_lane.")

    def _get_traffic_generator(self, bridge_length:float) -> list[_TrafficGenerator]:
        """
        Get a list of CTrafficGenerator instances for each lane.

        Parameters
        ----------
        bridge_length : float\n
            The length of the bridge in m. This bridge length is to prevent vehicle overlap. 
        """

        self._check_vehicle_classifier()

        generator_list = [None]*self._no_lane

        config = _ConfigData()
        config._setRoad(self._no_lane, self._no_dir, self._no_lane_dir_1, self._no_lane_dir_2)

        for i in range(self._no_lane):
            traffic_generator = _TrafficGenerator(config)

            lfc = self._lanes[i].lfc._get_LFC()  # real CLaneFlowComposition instance

            vehicle_gen, _ = self._lanes[i].vehicle_gen._get_generator(lfc)  # real CVehGenXXX instance
            headway_gen, headway_gen_data = self._lanes[i].headway_gen._get_generator(lfc)  # real CFowGenXXX instance

            headway_gen._setMaxBridgeLength(bridge_length)  # This is its right place, absolutely! 

            traffic_generator.setLaneData(lfc, vehicle_gen, headway_gen, self._lanes[i].start_time)  # 0-based global index here
            traffic_generator.initLane(headway_gen_data)

            generator_list[i] = traffic_generator

        return generator_list
    
    @property
    def tag(self) -> str:
        return self._tag

    @property
    def no_lane(self) -> int:
        return self._no_lane
    
    @property
    def no_dir(self) -> int:
        return self._no_dir
    
    @property
    def no_lane_dir_1(self) -> int:
        return self._no_lane_dir_1
    
    @property
    def no_lane_dir_2(self) -> int:
        return self._no_lane_dir_2

    @property
    def vehicle_classifier(self) -> int:
        return self._vehicle_classifier
    
    def _check_vehicle_classifier(self):

        if not (all(lane.vehicle_gen._get_VC() == 0 for lane in self._lanes) or all(lane.vehicle_gen._get_VC() == 1 for lane in self._lanes)):
            raise RuntimeError("Vehicle classifier should be the same for all vehicle generators.")

        self._vehicle_classifier = self._lanes[0].vehicle_gen._get_VC()

    def _set_lane_properties(self):
        lanes_dirs = [lane.dir for lane in self._lanes]
        self._no_dir = max(lanes_dirs)
        self._no_lane_dir_1 = lanes_dirs.count(1)
        self._no_lane_dir_2 = lanes_dirs.count(2)

