"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from ..lib.BTLS import (
    _TrafficLoader,
    _VehicleTrafficFile,
    Vehicle,
    _Vehicle,
    _VehClassPattern,
    _VehClassAxle,
)
from typing import Literal, Union
from pathlib import Path

__all__ = ["TrafficLoader"]


class TrafficLoader:
    def __init__(self, no_lane: int):
        """
        The TrafficLoader instance stores the information for creating CTrafficLoader instances for each lane.

        Parameters
        ----------
        no_lane : int\n
            The number of lanes on the bridge.
        """

        self._tag = "HistoryTraffic"

        self._no_lane = no_lane
        self._sim_day = None
        self._no_dir = None
        self._no_lane_dir_1 = None
        self._no_lane_dir_2 = None
        self._vehicle_classifier = 1

        self._lanes_vehicles = [[] for _ in range(no_lane)]

    def __getstate__(self):
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["no_lane"] = self._no_lane
        attribute_dict["sim_day"] = self._sim_day
        attribute_dict["no_dir"] = self._no_dir
        attribute_dict["no_lane_dir_1"] = self._no_lane_dir_1
        attribute_dict["no_lane_dir_2"] = self._no_lane_dir_2
        attribute_dict["vehicle_classifier"] = self._vehicle_classifier
        attribute_dict["lanes_vehicles"] = self._lanes_vehicles

        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._no_lane = attribute_dict["no_lane"]
        self._sim_day = attribute_dict["sim_day"]
        self._no_dir = attribute_dict["no_dir"]
        self._no_lane_dir_1 = attribute_dict["no_lane_dir_1"]
        self._no_lane_dir_2 = attribute_dict["no_lane_dir_2"]
        self._vehicle_classifier = attribute_dict["vehicle_classifier"]
        self._lanes_vehicles = attribute_dict["lanes_vehicles"]

    def add_traffic(
        self,
        traffic: Union[Path, list[_Vehicle]],
        traffic_format: Literal[1, 2, 3, 4] = None,
        use_average_speed: bool = False,
        use_const_speed: bool = False,
        const_speed_value: float = 0.0,
        **kwargs,
    ) -> None:
        """
        Add a recorded traffic from either a .txt file or a vehicle list.

        Parameters
        ----------
        traffic: Union[Path,list[_Vehicle]]\n
            The path to the traffic file,
            or a list of vehicles.

        traffic_format : Literal[1, 2, 3, 4], optional\n
            The format of the .txt traffic file.\n
            1: CASTOR format.\n
            2: BEDIT format.\n
            3: DITIS format.\n
            4: MON format.

        use_average_speed : bool, optional\n
            Whether to use the average speed of the vehicle. \n
            The default is False.

        use_const_speed : bool, optional\n
            Whether to use a constant speed for all vehicles. \n
            The default is False.

        const_speed_value : float, optional\n
            The constant speed value (in km/h). This will be an essential input if use_const_speed is True. \n
            The default is 0.0.

        Keyword Arguments
        -----------------
        classifier_type : str, optional\n
            The vehicle classifier type. The default is "pattern".

        Returns
        -------
        None.
        """

        if use_average_speed and use_const_speed:
            raise ValueError(
                "use_average_speed and use_const_speed cannot both be True."
            )

        if use_const_speed and const_speed_value <= 0.0:
            raise ValueError(
                "const_speed_value should be positive non-zero if use_const_speed is True."
            )

        if kwargs.get("classifier_type") == "axle":
            vehicle_classifier = _VehClassAxle()
            self._vehicle_classifier = 0
        else:
            vehicle_classifier = _VehClassPattern()
            self._vehicle_classifier = 1

        traffic_data = _VehicleTrafficFile(
            vehicle_classifier, use_const_speed, use_average_speed, const_speed_value
        )

        if isinstance(traffic, (Path, str)):
            if traffic_format is None:
                raise ValueError("Argument traffic_format is not specified.")
            traffic = Path(traffic) if not isinstance(traffic, Path) else traffic
            traffic_data.read(
                traffic, traffic_format
            )  # The simulation requires traffic information before _get_traffic_loader is called.
        elif isinstance(traffic, list):
            if all(isinstance(vehicle, (Vehicle, _Vehicle)) for vehicle in traffic):
                traffic_data.assignTraffic(traffic)
            else:
                raise ValueError("Existing non-Vehicle object in the traffic.")
        else:
            raise ValueError("Invalid traffic data for traffic loader.")

        if self._no_lane != traffic_data.getNoLanes():
            raise RuntimeError(
                f"Number of lanes included in traffic file is not equal to {self._no_lane}."
            )
        self._sim_day = traffic_data.getNoDays()
        self._no_dir = traffic_data.getNoDirn()
        self._no_lane_dir_1 = traffic_data.getNoLanesDir1()
        self._no_lane_dir_2 = traffic_data.getNoLanesDir2()
        self._check_traffic()

        for _ in range(traffic_data.getNoVehicles()):
            temp_vehicle = traffic_data.getNextVehicle()
            self._lanes_vehicles[temp_vehicle._getGlobalLane(self._no_lane) - 1].append(
                temp_vehicle
            )

    def _get_traffic_loader(self) -> list[_TrafficLoader]:
        """
        Get a list of CTrafficLoader instances for each lane.
        """

        loader_list = [None] * self._no_lane

        for i in range(self._no_lane):
            traffic_loader = _TrafficLoader()

            if i < self._no_lane_dir_1:
                lane_dir = 1
            else:
                lane_dir = 2

            traffic_loader.setLaneData(lane_dir, i)
            for vehicle in self._lanes_vehicles[i]:
                traffic_loader.addVehicle(vehicle)

            if traffic_loader.getNoVehicles() > 0:
                traffic_loader.setFirstArrivalTime()
            else:
                raise Warning(f"No vehicle in lane {i+1}.")

            loader_list[i] = traffic_loader

        return loader_list

    @property
    def sim_day(self) -> int:
        return self._sim_day

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

    def _check_traffic(self) -> None:
        if self._sim_day == 0:
            raise ValueError("No traffic in vehicle file.")
        if self._no_lane == 0:
            raise ValueError("No lanes in vehicle file.")
        if self._no_dir == 0:
            raise ValueError("No directions in vehicle file.")
        if self._no_dir == 2 and self._no_lane == 1:
            raise ValueError(
                "Two directions traffic found but only one lane specified."
            )
