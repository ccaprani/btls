from .BTLS_collections import (
    _ConfigData,
    _LaneFlowComposition,
    _FlowModelDataNHM,
    _FlowGenNHM,
    _FlowModelDataConstant,
    _FlowGenConstant,
    _FlowModelDataCongested,
    _FlowGenCongested,
    _FlowModelDataPoisson,
    _FlowGenPoisson,
)
from .lfc import LaneFlowComposition
import importlib.resources as pkg_resources

__all__ = [
    "HeadwayGenNHM",
    "HeadwayGenConstant",
    "HeadwayGenCongested",
    "HeadwayGenFreeflow",
]


class HeadwayGenNHM:
    def __init__(self, **kwargs):
        """
        The HeadwayGenNHM instance in Python stores the data for creating a CFlowGenNHM instance in C++.
        Headway generator - NHM. All heavy vehicles' headways will be generated based on the pre-studied distribution of Auxerre heavy vehicles' headways.
        """

        self._tag = "NHM"
        self._config = _ConfigData()

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

    def _set_config(self, **kwargs):
        traffic_folder = str(
            pkg_resources.files("pybtls").joinpath("data/GraveParameters/Auxerre")
        )

        self._config.set_headway_gen_NHM(traffic_folder=traffic_folder)

    def _check_lfc(self, lfc: LaneFlowComposition):
        if not lfc.speed_assigned:
            raise ValueError(
                "Speed data is not included in the LaneFlowComposition instance."
            )

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_FlowGenNHM, _FlowModelDataNHM]:
        """
        Get a CFlowGenNHM instance and a CFlowModelDataNHM instance (generator and its model data).
        """

        model_data = _FlowModelDataNHM(self._config, lfc)

        return _FlowGenNHM(model_data), model_data


class HeadwayGenConstant:
    def __init__(self, constant_speed: float, constant_gap: float, **kwargs):
        """
        The HeadwayGenConstant instance in Python stores the data for creating a CFlowGenConstant instance in C++.
        Headway generator - constant. All heavy vehicles' headways will be generated based on the constant speed and gap.

        Parameters
        ----------
        constant_speed : float\n
            The constant speed of heavy vehicles in km/h.

        constant_gap : float\n
            The constant gap between heavy vehicles in s.
        """

        self._tag = "Constant"
        self._config = _ConfigData()

        self._constant_speed = constant_speed  # in km/h
        self._constant_gap = constant_gap  # in s

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

    def _set_config(self, **kwargs):
        self._config.set_headway_gen_constant(
            constant_speed=self._constant_speed, constant_gap=self._constant_gap
        )

    def _check_lfc(self, lfc: LaneFlowComposition):
        pass

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_FlowGenConstant, _FlowModelDataConstant]:
        """
        Get a CFlowGenConstant instance and a CFlowModelDataConstant instance (generator and its model data).
        """

        model_data = _FlowModelDataConstant(self._config, lfc)

        return _FlowGenConstant(model_data), model_data


class HeadwayGenCongested:
    def __init__(self, congested_spacing: float, congested_speed: float, **kwargs):
        """
        The HeadwayGenCongested instance in Python stores the data for creating a CFlowGenCongested instance in C++.
        Headway generator - congested. All heavy vehicles' headways will be generated based on the congested speed and spacing.

        Parameters
        ----------
        congested_spacing : float\n
            The congested spacing of heavy vehicles in m, front's rear to back's head.

        congested_speed : float\n
            The congested speed of heavy vehicles in km/h.

        Keyword Arguments
        -----------------
        congested_gap_coef_var : float\n
            The coefficient of variation of the congested gap. Default is 0.05.
        """

        self._tag = "Congested"
        self._config = _ConfigData()

        self._congested_spacing = congested_spacing  # in m
        self._congested_speed = congested_speed  # in km/h

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

    def _set_config(self, **kwargs):
        congested_gap_coef_var = kwargs.get("congested_gap_coef_var", 0.05)

        self._config.set_headway_gen_congested(
            congested_spacing=self._congested_spacing,
            congested_speed=self._congested_speed,
            congested_gap_coef_var=congested_gap_coef_var,
        )

    def _check_lfc(self, lfc: LaneFlowComposition):
        pass

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_FlowGenCongested, _FlowModelDataCongested]:
        """
        Get a CFlowGenCongested instance and a CFlowModelDataCongested instance (generator and its model data).
        """

        model_data = _FlowModelDataCongested(self._config, lfc)

        return _FlowGenCongested(model_data), model_data


class HeadwayGenFreeflow:
    def __init__(self, **kwargs):
        """
        The HeadwayGenFreeflow instance in Python stores the data for creating a CFlowGenPoisson instance in C++.
        Headway generator - freeflow. All heavy vehicles' headways will be generated based on the Poisson arrival theory.
        """

        self._tag = "Freeflow"
        self._config = _ConfigData()

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

    def _set_config(self, **kwargs):
        self._config.set_headway_gen_freeflow()

    def _check_lfc(self, lfc: LaneFlowComposition):
        if not lfc.speed_assigned:
            raise ValueError(
                "Speed data is not included in the LaneFlowComposition instance."
            )

    def _get_generator(
        self, lfc: _LaneFlowComposition
    ) -> tuple[_FlowGenPoisson, _FlowModelDataPoisson]:
        """
        Get a CFlowGenPoisson instance and a CFlowModelDataPoisson instance (generator and its model data).
        """

        model_data = _FlowModelDataPoisson(self._config, lfc)

        return _FlowGenPoisson(model_data), model_data
