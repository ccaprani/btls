from email import header
import numpy as np
import warnings
import os

from PyBTLS.py.utils_interface._helper_class import DFBased


"""
Grave Traffic Definition.
Currently unused. This is meant to replace the current Grave traffic definition in the future.
The current class (see PyBTLS.py.traffic.PyBtlsTraffic) is based on reading the default txt/csv file.
It is not easily editable by the user as the data is stored as string, and user must manipulate it as string.

An advantage of this class is it is based on DataFrame object, allowing it to be manipulated easily.
However, the challenge is to convert the information in each txt and csv files into DataFrames object.
"""

class AxleSpacing(DFBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        txt = txt.split("\n")
        data = [t.split(",") for t in txt]
        if (len(data[-1] == 0)) or (len(data[-1]) == 1 and data[-1] == ""):
            data = data[0:-1]
        data = data.split(",")
        header = [
            [f"rho_{i+1}_{i+2}", f"mu_{i+1}_{i+2}", f"sigma_{i+1}_{i+2}"]
            for i in range(0, 5)
        ]

        raise NotImplementedError()

    def write_as_csv(self, path):
        raise NotImplementedError()


"""
class AxleSpacing():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "ASall.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class AxleTrackWidth():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "ATW.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class AxleWeight23():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "AW2&3.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class AxleWeight45():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "AW4&5.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class GrossVehicleWeight():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "GVWpdf.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class Headway():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "NHM.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class FlowRate():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "FlowR.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
class ClassPercentage():
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
    
    def import_from_csv(self, path):
        with open(path, 'r') as f:
            txt = f.readlines()
        self.txt = ''.join(txt)
        self._parse_txt_to_data()
        
    def _create_txt():
        1
        
    def write_as_csv(self, path, fname = "CLASS%.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
        
        
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
"""
