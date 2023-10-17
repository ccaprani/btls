from abc import ABC, abstractmethod, abstractproperty
from PyBTLS.lib.BTLS_collections import _Vehicle
__all__ = ['CriterionBase','AUCriterion']


# Classification base
class CriterionBase(ABC):
    """
    CriterionBase is an abstract base class for regional classification criterion.

    Attributes
    ----------
    group_spacing : float
        The critical spacing to determine an axle group.
    light_vehicle_weight_limit : float
        The weight limit to determine a light vehicle.
    _vehicle_weight : float
        The vehicle weight.
    _vehicle_length : float
        The vehicle length.
    _vehicle_no_axles : int
        The number of axles of a vehicle.
    _vehicle_axle_spacings_list : list[float]
        The list of axle spacings of a vehicle.
    _vehicle_no_axle_groups : int
        The number of axle groups of a vehicle.
    """
    
    def __init__(self):
        """
        Constructor of CriterionBase.
        """

        super().__init__()
        self.group_spacing = None
        self.light_vehicle_weight_limit = None
        self._vehicle_weight = None
        self._vehicle_length = None
        self._vehicle_no_axles = None
        self._vehicle_axle_spacings_list = None
        self._vehicle_no_axle_groups = None

    @abstractmethod
    def set_vehicle_property(self, vehicle:_Vehicle) -> None:
        """
        To set the attributes from the input vehicle object.

        Parameters
        ----------
        vehicle : Vehicle
            The input vehicle object.

        Returns
        -------
        None.
        """

        pass

    @abstractmethod
    def get_vehicle_class(self) -> str:
        """
        To get the vehicle classified.

        Returns
        -------
        vehicle_class : str
            The vehicle's class.
        """

        pass
    
    def _get_vehicle_axle_spacing(self, vehicle):
        vehicle_axle_spacings_list = []
        for i in range(0, self._vehicle_no_axles-1):
            vehicle_axle_spacings_list.append(vehicle.get_axle_spacing(i))
        return vehicle_axle_spacings_list

    def _get_vehicle_no_axle_groups(self):
        vehicle_no_axle_groups = 1
        for _, axle_spacing in enumerate(self._vehicle_axle_spacings_list):
            if axle_spacing >= self.group_spacing:
                vehicle_no_axle_groups += 1
        return vehicle_no_axle_groups


# Austroads' classification - Australia
class AUCriterion(CriterionBase):
    """
    AUCriterion is a class for Austroads' classification.
    """

    def __init__(self, group_spacing:float=2.1, light_vehicle_weight_limit:float=30.0):
        super().__init__()
        self.group_spacing = group_spacing
        self.light_vehicle_weight_limit = light_vehicle_weight_limit

    def set_vehicle_property(self, vehicle:_Vehicle) -> None:
        self._vehicle_weight = vehicle.get_gvw()
        self._vehicle_length = vehicle.get_length()
        self._vehicle_no_axles = vehicle.get_no_axles()
        self._vehicle_axle_spacings_list = self._get_vehicle_axle_spacing(vehicle)
        self._vehicle_no_axle_groups = self._get_vehicle_no_axle_groups()

    def get_vehicle_class(self) -> str:
        if self._vehicle_no_axle_groups == 1:
            return "error_trailer"
        elif self._vehicle_weight < self.light_vehicle_weight_limit:
            return "light_vehicle"
        if self._vehicle_no_axles == 2 and self._vehicle_no_axle_groups == 2:
            return "class_3"
        elif self._vehicle_no_axles == 3 and self._vehicle_no_axle_groups == 2:
            return "class_4"
        elif self._vehicle_no_axles == 4 and self._vehicle_no_axle_groups == 2:
            return "class_5"
        elif self._vehicle_no_axles == 3 and self._vehicle_no_axle_groups == 3:
            return "class_6"
        elif self._vehicle_no_axles == 4 and self._vehicle_no_axle_groups == 3:
            return "class_7"
        elif self._vehicle_no_axles == 5 and self._vehicle_no_axle_groups == 3:
            return "class_8"
        elif self._vehicle_no_axles >= 6 and self._vehicle_no_axle_groups == 3:
            return "class_9"
        elif self._vehicle_no_axles >= 7 and self._vehicle_no_axle_groups == 4:
            return "class_10"
        elif self._vehicle_no_axles >= 7 and self._vehicle_no_axle_groups == (5 or 6):
            return "class_11"
        elif self._vehicle_no_axles >= 7 and self._vehicle_no_axle_groups >= 7:
            return "class_12"
        else:
            return "unknown"
    

if __name__ == "__main__":
    vehicle = _Vehicle()
    criterion = AUCriterion(13, 2.1, 30.0)
    criterion.set_vehicle_property(vehicle)
    print(criterion.get_vehicle_class())

