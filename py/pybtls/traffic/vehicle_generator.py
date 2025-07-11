"""
The methods and classes that are not defined in Python are defined in C++ py_main.cpp.
"""

from ..lib.BTLS import (
    _ConfigData,
    _LaneFlowComposition,
    _VehicleGenNominal,
    _VehModelDataNominal,
    _VehicleGenGrave,
    _VehModelDataGrave,
    _VehicleGenGarage,
    _VehModelDataGarage,
    _VehClassPattern,
    _VehClassAxle,
    Vehicle,
    _Vehicle,
)
from .lfc import LaneFlowComposition
from ..garage.read import read_garage_file
from typing import Literal, Union
from pathlib import Path
import importlib.resources as pkg_resources

__all__ = ["VehicleGenNominal", "VehicleGenGrave", "VehicleGenGarage"]


class VehicleGenNominal:
    def __init__(self, nominal_vehicle: Vehicle, COV_list: list[float], **kwargs):
        """
        The VehicleGenNominal instance in Python stores the data for creating a CVehicleGenNominal instance in C++.\n
        All heavy vehicles will be generated based on the intput nominal vehicle.

        Parameters
        ----------
        nominal_vehicle : Vehicle\n
            Nominal vehicle.

        COV_list : list[float]\n
            List of COV for axle spacing and axle weight. \n
            COV_list = [COV_AS, COV_AW].

        Keyword Arguments
        -----------------
        classifier_type : str\n
            Vehicle classifier type. "axle" or "pattern". Default is "pattern".

        lane_eccentricity_std : float\n
            Standard deviation of lane eccentricity. Default is 0.0.

        kernel_type : int\n
            Kernel type. 0 (Normal) or 1 (Triangle). Default is 1 (Triangle).
        """

        self._tag = "Nominal"
        self._config = _ConfigData()

        if len(COV_list) != 2:
            raise ValueError("Invalid COV list for nominal vehicle generator.")

        self._nominal_vehicle = nominal_vehicle
        self._COV_list = COV_list  # COV_list = [COV_AS, COV_AW]

        self._set_config(**kwargs)

    def __getstate__(self):
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["config"] = self._config
        attribute_dict["nominal_vehicle"] = self._nominal_vehicle
        attribute_dict["COV_list"] = self._COV_list

        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._config = attribute_dict["config"]
        self._nominal_vehicle = attribute_dict["nominal_vehicle"]
        self._COV_list = attribute_dict["COV_list"]

    @property
    def tag(self) -> str:
        return self._tag

    def _get_VC(self):
        return self._config._Traffic.CLASSIFICATION

    def _set_config(self, **kwargs):
        classifier_type = 0 if kwargs.get("classifier_type") == "axle" else 1
        lane_eccentricity_std = kwargs.get("lane_eccentricity_std", 0.0)
        kernel_type = kwargs.get("kernel_type", 1)

        self._config.set_veh_gen_nominal(
            vehicle_classifier=classifier_type,
            lane_eccentricity_std=lane_eccentricity_std,  # in cm
            kernel_type=kernel_type,
        )

    def _check_lfc(self, lfc: LaneFlowComposition):
        if not lfc.flow_assigned:
            raise ValueError(
                "Flow data is not included in the LaneFlowComposition instance."
            )

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_VehicleGenNominal, _VehModelDataNominal]:
        """
        Get a CVehicleGenNominal instance and a CVehModelDataNominal instance (generator and its model data).
        """

        if self._config._Traffic.CLASSIFICATION == 1:
            vehicle_classifier = _VehClassPattern()
        else:
            vehicle_classifier = _VehClassAxle()

        model_data = _VehModelDataNominal(
            self._config, vehicle_classifier, lfc, self._nominal_vehicle, self._COV_list
        )

        return _VehicleGenNominal(model_data), model_data


class VehicleGenGrave:
    def __init__(
        self,
        traffic_site: Literal[
            "A196",
            "A296",
            "Angers",
            "Auxerre",
            "B224",
            "Samaris-D",
            "Samaris-D1",
            "Samaris-D2",
            "Samaris-D3",
            "Samaris-S",
            "Samaris-S1",
            "Samaris-S2",
            "Samaris-S3",
        ],
        truck_track_width: float = 190.0,
        **kwargs,
    ):
        """
        The VehicleGenGrave instance in Python stores the data for creating a CVehicleGenGrave instance in C++.\n
        All heavy vehicles will be generated by sampling from the pre-studied distributions of vehicle properties.

        Parameters
        ----------
        traffic_site : Literal['A196','A296','Angers','Auxerre','B224','Samaris-D','Samaris-D1','Samaris-D2','Samaris-D3','Samaris-S','Samaris-S1','Samaris-S2','Samaris-S3']\n
            Traffic site. These sites are all in Europe.

        truck_track_width : float optional\n
            Truck track width. Default is 190.0 (in cm).

        Keyword Arguments
        -----------------
        classifier_type : str\n
            Vehicle classifier type. "axle" or "pattern". Default is "pattern".

        lane_eccentricity_std : float\n
            Standard deviation of lane eccentricity. Default is 0.0.
        """

        self._tag = "Grave"
        self._config = _ConfigData()

        if traffic_site not in [
            "A196",
            "A296",
            "Angers",
            "Auxerre",
            "B224",
            "Samaris-D",
            "Samaris-D1",
            "Samaris-D2",
            "Samaris-D3",
            "Samaris-S",
            "Samaris-S1",
            "Samaris-S2",
            "Samaris-S3",
        ]:
            raise ValueError("Unrecorded traffic site for Grave model.")

        self._traffic_site = traffic_site
        self._truck_track_width = truck_track_width

        self._set_config(**kwargs)

    def __getstate__(self):
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["config"] = self._config

        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._config = attribute_dict["config"]

    @property
    def tag(self) -> str:
        return self._tag

    def _get_VC(self):
        return self._config._Traffic.CLASSIFICATION

    def _set_config(self, **kwargs):
        classifier_type = 0 if kwargs.get("classifier_type") == "axle" else 1
        lane_eccentricity_std = kwargs.get("lane_eccentricity_std", 0.0)

        traffic_folder = str(
            pkg_resources.files("pybtls").joinpath(
                "data/GraveParameters/" + self._traffic_site
            )
        )

        self._config.set_veh_gen_grave(
            vehicle_classifier=classifier_type,
            traffic_folder=traffic_folder,
            truck_track_width=self._truck_track_width,
            lane_eccentricity_std=lane_eccentricity_std,  # in cm
        )

    def _check_lfc(self, lfc: LaneFlowComposition):
        if not lfc.flow_assigned:
            raise ValueError(
                "Flow data is not included in the LaneFlowComposition instance."
            )

        if not lfc.truck_composition_assigned:
            raise ValueError(
                "Truck composition data is not included in the LaneFlowComposition instance."
            )

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_VehicleGenGrave, _VehModelDataGrave]:
        """
        Get a CVehicleGenGrave instance and a CVehModelDataGrave instance (generator and its model data).
        """

        if self._config._Traffic.CLASSIFICATION == 1:
            vehicle_classifier = _VehClassPattern()
        else:
            vehicle_classifier = _VehClassAxle()

        model_data = _VehModelDataGrave(self._config, vehicle_classifier, lfc)

        return _VehicleGenGrave(model_data), model_data


class VehicleGenGarage:
    def __init__(
        self,
        garage: Union[Path, list[_Vehicle]],
        kernel: list[list[float]],
        garage_format: Literal[1, 2, 3, 4] = None,
        **kwargs,
    ):
        """
        The VehicleGenGarage instance in Python stores the data for creating a CVehicleGenGarage instance in C++.\n
        This generation is a bootstrapping process.

        Parameters
        ----------
        garage : Union[Path, list[_Vehicle]]\n
            The path to the garage file,
            or a list of Vehicle objects.

        kernel : list[list[float]]\n
            The kernel is to ensure variation between the generated vehicles and garages. \n
            kernel = [\n
                [Mean_GVW, Std_GVW],\n
                [Mean_AxleWeight, Std_AxleWeight],\n
                [Mean_AxleSpacing, Std_AxleSpacing]\n
                ].

        garage_format : Literal[1,2,3,4], optional\n
            The format of the .txt garage file.\n
            1: CASTOR format.\n
            2: BEDIT format.\n
            3: DITIS format.\n
            4: MON format.

        Keyword Arguments
        -----------------
        classifier_type : str\n
            Vehicle classifier type. "axle" or "pattern". Default is "pattern".

        lane_eccentricity_std : float\n
            Standard deviation of lane eccentricity. Default is 0.0.

        kernel_type : int\n
            Kernel type. 0 (Normal) or 1 (Triangle). Default is 1 (Triangle).
        """

        self._tag = "Garage"
        self._config = _ConfigData()

        if len(kernel) != 3 and not all(len(sublist) == 2 for sublist in kernel):
            raise ValueError("Invalid kernel data for garage vehicle generator.")

        if isinstance(garage, (Path, str)):
            if garage_format is None:
                raise ValueError("Garage format is not specified.")
            self._garage = (
                Path(garage).resolve()
                if not isinstance(garage, Path)
                else garage.resolve()
            )
        elif isinstance(garage, list):
            if all(isinstance(vehicle, (Vehicle, _Vehicle)) for vehicle in garage):
                self._garage = garage
            else:
                raise ValueError("Existing non-Vehicle object in the garage.")
        else:
            raise ValueError("Invalid garage data for garage vehicle generator.")

        self._kernel = kernel  # kernel = [[Mean_GVW, Std_GVW], [Mean_AW, Std_AW], [Mean_AS, Std_AS]]
        self._garage_format = garage_format

        self._set_config(**kwargs)

    def __getstate__(self):
        attribute_dict = {}

        attribute_dict["tag"] = self._tag
        attribute_dict["config"] = self._config
        attribute_dict["garage"] = self._garage
        attribute_dict["garage_format"] = self._garage_format
        attribute_dict["kernel"] = self._kernel

        return attribute_dict

    def __setstate__(self, attribute_dict):
        self._tag = attribute_dict["tag"]
        self._config = attribute_dict["config"]
        self._garage = attribute_dict["garage"]
        self._garage_format = attribute_dict["garage_format"]
        self._kernel = attribute_dict["kernel"]

    @property
    def tag(self) -> str:
        return self._tag

    def _get_VC(self):
        return self._config._Traffic.CLASSIFICATION

    def _set_config(self, **kwargs):
        classifier_type = 0 if kwargs.get("classifier_type") == "axle" else 1
        lane_eccentricity_std = kwargs.get("lane_eccentricity_std", 0.0)
        kernel_type = kwargs.get("kernel_type", 1)

        self._config.set_veh_gen_garage(
            vehicle_classifier=classifier_type,
            lane_eccentricity_std=lane_eccentricity_std,  # in cm
            kernel_type=kernel_type,
        )

    def _check_lfc(self, lfc: LaneFlowComposition):
        if not lfc.flow_assigned:
            raise ValueError(
                "Flow data is not included in the LaneFlowComposition instance."
            )

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_VehicleGenGarage, _VehModelDataGarage]:
        """
        Get a CVehicleGenGarage instance and a CVehModelDataGarage instance (generator and its model data).
        """

        if self._config._Traffic.CLASSIFICATION == 1:
            vehicle_classifier = _VehClassPattern()
        else:
            vehicle_classifier = _VehClassAxle()

        vehicle_list = (
            read_garage_file(self._garage, self._garage_format)
            if isinstance(self._garage, Path)
            else self._garage
        )

        model_data = _VehModelDataGarage(
            self._config, vehicle_classifier, lfc, vehicle_list, self._kernel
        )

        return _VehicleGenGarage(model_data), model_data
