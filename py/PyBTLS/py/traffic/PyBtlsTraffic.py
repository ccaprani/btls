"""
 NOTE:  OLD CODES, BUT STILL REQUIRED AS REPLACEMENT IS STILL BEING WORKED ON
 See PyBtls.py.traffic.grave_traffic for replacement
"""

from abc import abstractproperty
from PyBTLS.py.utils_interface._helper_functions import read_default_traffic_file_raw
import numpy as np
import warnings
import os


class BaseTrafficFile:
    """
    Basic class to contain Grave traffic properties, such as axle spacing, axle weight, etc.
    Not to be used directly, but rather as a parent class for the other traffic property classes.
    Change the `_default_fname` property to change the default file name when reading using `.load_default()`
    """
    def __init__(self, data=None, path=None):
        self.data = data
        self.txt = ""
        if not path == None:
            if not data == None:
                raise ValueError(
                    "ERROR: Ambiguous input at initialisation. Only supply either the 'data' argument, or the 'path' argument to the csv data file"
                )
            else:
                self.import_from_csv(path)

    def to_numpy(self):
        return self.data

    def as_numpy(self):
        return self.data

    def _parse_txt_to_data(self):
        1

    def load_default(self, data_source):
        self.txt = read_default_traffic_file_raw(data_source)
        self._parse_txt_to_data()

    def import_from_csv(self, path):
        with open(path, "r") as f:
            txt = f.readlines()
        self.txt = "".join(txt)
        self._parse_txt_to_data()

    def _create_txt():
        1

    def write_as_csv(self, path, fname=None):
        if fname is None:
            fname = self._default_fname
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, "w") as f:
            f.write(self.txt)

    @abstractproperty
    def _default_fname(self):
        pass


class AxleSpacing(BaseTrafficFile):
    """
    Axle spacing distribution.
    """
    @property
    def _default_fname(self):
        return "ASall.csv"


class AxleTrackWidth(BaseTrafficFile):
    """
    Axle track width distribution.
    """
    @property
    def _default_fname(self):
        return "ATW.csv"


class AxleWeight23(BaseTrafficFile):
    """
    Axle weight distribution for Axle 2 and 3 of a 4 or 5 axle truck.
    """
    @property
    def _default_fname(self):
        return "AW2&3.csv"


class AxleWeight45(BaseTrafficFile):
    """
    Axle weight distribution for Axle 4 and 5 of a 4 or 5 axle truck.
    """
    @property
    def _default_fname(self):
        return "AW4&5.csv"


class GrossVehicleWeight(BaseTrafficFile):
    """
    Gross vehicle weight distribution.
    """
    @property
    def _default_fname(self):
        return "GVWpdf.csv"


class Headway(BaseTrafficFile):
    """
    Headway definition class
    """
    @property
    def _default_fname(self):
        return "NHM.csv"


class FlowRate(BaseTrafficFile):
    """
    Traffic flow rate definition
    """
    @property
    def _default_fname(self):
        return "FlowR.csv"


class ClassPercentage(BaseTrafficFile):
    """
    Percentage of truck class to be generated
    """
    @property
    def _default_fname(self):
        return "CLASS%.csv"


class BtlsTraffic:
    """
    Parent class of Grave traffic.
    """
    def __init__(
        self,
        axle_spacing=None,
        axle_track_width=None,
        axle_weight_23=None,
        axle_weight_45=None,
        gross_vehicle_weight=None,
        headway=None,
        flow_rate=None,
        class_percentage=None,
    ):

        self.axle_spacing = axle_spacing
        self.axle_track_width = axle_track_width
        self.axle_weight_23 = axle_weight_23
        self.axle_weight_45 = axle_weight_45
        self.gross_vehicle_weight = gross_vehicle_weight
        self.headway = headway
        self.flow_rate = flow_rate
        self.class_percentage = class_percentage

    def write_as_csv(self, path="Traffic"):
        os.makedirs(path, exist_ok=True)
        if self.axle_spacing == None:
            warnings.warn("WARNING: No AxleSpacing specified")
        else:
            self.axle_spacing.write_as_csv(path)

        if self.axle_track_width == None:
            warnings.warn("WARNING: No AxleTrackWidth specified")
        else:
            self.axle_track_width.write_as_csv(path)

        if self.axle_weight_23 == None:
            warnings.warn("WARNING: No AxleWeight23 specified")
        else:
            self.axle_weight_23.write_as_csv(path)

        if self.axle_weight_45 == None:
            warnings.warn("WARNING: No AxleWeight45 specified")
        else:
            self.axle_weight_45.write_as_csv(path)

        if self.gross_vehicle_weight == None:
            warnings.warn("WARNING: No GrossVehicleWeight specified")
        else:
            self.gross_vehicle_weight.write_as_csv(path)

        if self.headway == None:
            warnings.warn("WARNING: No Headway specified")
        else:
            self.headway.write_as_csv(path)

        if self.flow_rate == None:
            warnings.warn("WARNING: No FlowRate specified")
        else:
            self.flow_rate.write_as_csv(path)

        if self.class_percentage == None:
            warnings.warn("WARNING: No ClassPercentage specified")
        else:
            self.class_percentage.write_as_csv(path)
