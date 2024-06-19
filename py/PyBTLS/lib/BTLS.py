from .libbtls import (
    get_info,
    run,
    _ConfigDataCore,
    _Vehicle,
    _VehClassAxle,
    _VehClassPattern,
    _Bridge,
    _InfluenceLine,
    _InfluenceSurface,
    _TrafficLoader,
    _TrafficGenerator,
    _VehicleBuffer,
    _LaneFlowComposition,
    _MultiModalNormal,
    _Distribution,
    _FlowGenNHM,
    _FlowGenCongested,
    _FlowGenPoisson,
    _FlowGenConstant,
    _FlowModelDataNHM,
    _FlowModelDataCongested,
    _FlowModelDataPoisson,
    _FlowModelDataConstant,
    _VehicleGenNominal,
    _VehicleGenGarage,
    _VehicleGenGrave,
    _VehModelDataNominal,
    _VehModelDataGarage,
    _VehModelDataGrave,
    _VehicleTrafficFile,
)

__all__ = [
    "get_info",
    "run",
    "_ConfigData",
    "_ConfigDataCore",
    "_Vehicle",
    "_VehClassAxle",
    "_VehClassPattern",
    "_Bridge",
    "_InfluenceLine",
    "_InfluenceSurface",
    "_TrafficLoader",
    "_TrafficGenerator",
    "_VehicleBuffer",
    "_LaneFlowComposition",
    "MultiModalNormal",
    "Distribution",
    "_FlowGenNHM",
    "_FlowGenCongested",
    "_FlowGenPoisson",
    "_FlowGenConstant",
    "_FlowModelDataNHM",
    "_FlowModelDataCongested",
    "_FlowModelDataPoisson",
    "_FlowModelDataConstant",
    "_VehicleGenNominal",
    "_VehicleGenGarage",
    "_VehicleGenGrave",
    "_VehicleModelData",
    "_VehicleTrafficFile",
    "_VehModelDataNominal",
    "_VehModelDataGarage",
    "_VehModelDataGrave",
    "MultiModalNormal",
    "Distribution",
]


class MultiModalNormal(_MultiModalNormal):
    pass


class Distribution(_Distribution):
    pass


class _ConfigData(_ConfigDataCore):
    def __init__(self):
        super().__init__()

    def set_veh_gen_grave(
        self,
        vehicle_classifier: int,
        traffic_folder: str,
        truck_track_width: float,
        lane_eccentricity_std: float,
    ) -> None:
        """
        Config for grave vehicle generation method.
        """

        self._Gen.TRAFFIC_FOLDER = traffic_folder
        self._Gen.TRUCK_TRACK_WIDTH = truck_track_width
        self._Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std
        self._Traffic.CLASSIFICATION = (
            vehicle_classifier  # 0 number of axle, 1 axle pattern
        )

    def set_veh_gen_garage(
        self, vehicle_classifier: int, lane_eccentricity_std: float, kernel_type: int
    ) -> None:
        """
        Config for garage vehicle generation method.
        """

        self._Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std
        self._Gen.KERNEL_TYPE = kernel_type
        self._Traffic.CLASSIFICATION = (
            vehicle_classifier  # 0 number of axle, 1 axle pattern
        )

    def set_veh_gen_nominal(
        self, vehicle_classifier: int, lane_eccentricity_std: float, kernel_type: int
    ) -> None:
        """
        Config for nominal vehicle generation method.
        """

        self._Gen.LANE_ECCENTRICITY_STD = lane_eccentricity_std
        self._Gen.KERNEL_TYPE = kernel_type
        self._Traffic.CLASSIFICATION = (
            vehicle_classifier  # 0 number of axle, 1 axle pattern
        )

    def set_headway_gen_NHM(self, traffic_folder: str) -> None:
        """
        Config for NHM headway generation method.
        """

        self._Gen.TRAFFIC_FOLDER = traffic_folder

    def set_headway_gen_constant(
        self, constant_speed: float, constant_gap: float
    ) -> None:
        """
        Config for constant headway generation method.
        """

        self._Traffic.CONSTANT_SPEED = constant_speed / 3.6  # km/h to m/s
        self._Traffic.CONSTANT_GAP = constant_gap

    def set_headway_gen_congested(
        self,
        congested_spacing: float,
        congested_speed: float,
        congested_gap_coef_var: float,
    ) -> None:
        """
        Config for congested headway generation method.
        """

        self._Traffic.CONGESTED_SPACING = congested_spacing
        self._Traffic.CONGESTED_SPEED = congested_speed / 3.6  # km/h to m/s
        self._Traffic.CONGESTED_GAP = (
            self._Traffic.CONGESTED_SPACING / self._Traffic.CONGESTED_SPEED
        )
        self._Traffic.CONGESTED_GAP_COEF_VAR = congested_gap_coef_var

    def set_headway_gen_freeflow(self) -> None:
        """
        Config for freeflow headway generation method.
        """

        pass
