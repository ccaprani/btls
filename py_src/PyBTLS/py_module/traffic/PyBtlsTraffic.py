"""
 NOTE:  OLD CODES, BUT STILL REQUIRED AS REPLACEMENT IS STILL BEING WORKED ON
 See PyBtls.py_module.traffic.grave_traffic for replacement
"""

from abc import abstractproperty
from PyBTLS.py_module.default_files._default_files_loader import read_default_raw_file
import numpy as np
import warnings
import os


class BaseTrafficFile():
    def __init__(self, data = None, path = None):
        self.data = data
        self.txt = ""
        if not path == None:
            if not data == None:
                raise ValueError("ERROR: Ambiguous input at initialisation. Only supply either the 'data' argument, or the 'path' argument to the csv data file")
            else:
                self.import_from_csv(path)

    def to_numpy(self):
        return self.data
    
    def as_numpy(self):
        return self.data
    
    def _parse_txt_to_data(self):
        1

    def load_default(self, data_source):
        self.txt = read_default_raw_file('default_traffic/' + data_source)
        self._parse_txt_to_data()
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = None):
        if fname is None:
            fname = self._default_fname
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)

    @abstractproperty
    def _default_fname(self):
        pass

class AxleSpacing(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "ASall.csv"
        
class AxleTrackWidth(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "ATW.csv"

class AxleWeight23(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "AW2&3.csv"
        
class AxleWeight45(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "AW4&5.csv"
        
class GrossVehicleWeight(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "GVWpdf.csv"

class Headway(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "NHM.csv"
        
class FlowRate(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "FlowR.csv"
        
class ClassPercentage(BaseTrafficFile):
    @property
    def _default_fname(self):
        return "CLASS%.csv"
        
        
class BtlsTraffic():
    def __init__(self, 
                 axle_spacing = None, 
                 axle_track_width = None,
                 axle_weight_23 = None,
                 axle_weight_45 = None,
                 gross_vehicle_weight = None,
                 headway = None,
                 flow_rate = None,
                 class_percentage = None):
        
        self.axle_spacing = axle_spacing
        self.axle_track_width = axle_track_width
        self.axle_weight_23 = axle_weight_23
        self.axle_weight_45 = axle_weight_45
        self.gross_vehicle_weight = gross_vehicle_weight
        self.headway = headway
        self.flow_rate = flow_rate
        self.class_percentage = class_percentage
        
    def write_as_csv(self, path):
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
