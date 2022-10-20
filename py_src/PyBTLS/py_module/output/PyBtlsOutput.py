import numpy as np
import pandas as pd
import warnings
import os
import re

from abc import ABC, abstractmethod
from PyBTLS.py_module.misc._helper_functions import parse_fixed_width_text, 
from PyBTLS.py_module.misc._vehicle_event_parser import parse_from_vehicle_text_list_to_df_kwargs


"""

"""

class _BtlsWrapper_DefaultOutputs():
    def __init__(self):
        self.interval_statistics = "No Output Set"
        self.vehicles = "No Output Set"
        self.block_maxima_summary = "No Output Set"

class BtlsOutput(ABC):
    """
    Abstract base class for BtlsOutput files
    """
    
    descriptor = None
    _core_data = None
    _core_text = None
    
    def __init__(self, data = None, path = None, descriptor = None):
        self.descriptor = descriptor
        if not path == None:
            if not data == None:
                raise ValueError("ERROR: Ambiguous input at initialisation. Only supply either the 'data' argument, or the 'path' argument to the csv data file")
            else:
                self.import_from_txt(path)
        else:
            self._core_data = data
    
    def __call__(self):
        return self._core_data
    
    @property
    def data(self):
        return self._core_data
    
    @data.setter
    def data(self, data):
        raise AttributeError("ERROR: Setting data manually after object construction is disallowed for `BtlsOutput` class. Instead, construct a new object if required, or use `.import_from_txt()` method to import from an output txt file.")
    
    def to_numpy(self):
        return self._core_data.to_numpy()
    
    def as_numpy(self):
        self.to_numpy()
    
    @abstractmethod
    def _from_txt_to_core_data(self, txt):
        pass
    
    @abstractmethod
    def _from_core_data_to_txt(self):
        pass
    
    def import_from_txt(self, fpath):
        with open(fpath) as f:
            content = f.read()
        self._core_text = content
        self._core_data = self._from_txt_to_core_data(content)
    
    def write_as_csv(self, fpath, fname):
        with open(fpath+fname) as f:
            f.write(self._from_core_data_to_txt())


"""

"""
    
class BtlsOutputIntervalStatistics(BtlsOutput):
    def _from_txt_to_core_data(self, txt):
        txt = txt.split('\n') #Split by line
        data = [text.split() for text in txt[1:-1]] #Split by column, ignore header and empty row at the end
        #Equalise the length of the data by padding the list at the end
        pad = len(max(data, key=len)) 
        data = np.array([i + [0]*(pad-len(i)) for i in data])

        base_header = [
            'id',
            'time',
            'num_events',
            'num_event_vehicles',
            'num_event_trucks',
            'min',
            'max',
            'mean',
            'std_dev',
            'variance',
            'skeweness',
            'kurtosis',
        ]

        column_type = {
            'id': int,
            'time': float,
            'num_events': int,
            'num_event_vehicles': int,
            'num_event_trucks': int,
            'min': float,
            'max': float,
            'mean': float,
            'std_dev': float,
            'variance': float,
            'skeweness': float,
            'kurtosis': float,
        }

        #Get size of data, check how many excess truck # event if there's any
        num_defined_axles = data.shape[1] - 12

        #Append the axle info
        axle_header = ['event_' + str(i+1) + '_truck' for i in range(0,num_defined_axles)]
        header = np.append(base_header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        df = pd.DataFrame(data, columns = header)
        df = df.astype(column_type)

        return df
    
    def _from_core_data_to_txt(self):
        pass


"""

"""

class BtlsOutputBlockMaximaSummary(BtlsOutput):
    def _from_txt_to_core_data(self, txt):
        txt = txt.split('\n') #Split by line
        data = [re.split("\t\t|\t", t)[0:-1] for t in txt][0:-1] #Ignore empty column at the end. Ignore empty row at the end.
        #Equalise the length of the data by padding the list at the end
        pad = len(max(data, key=len)) 
        data = np.array([i + [0]*(pad-len(i)) for i in data])

        base_header = [
            "index",
            'event_1_truck',
        ]

        column_type = {
            "index": int,
            'event_1_truck': float,
        }

        #Get size of data, check how many excess truck # event if there's any
        num_defined_axles = data.shape[1] - 2

        #Append the axle info
        axle_header = ['event_' + str(i+2) + '_truck' for i in range(0,num_defined_axles)]
        header = np.append(base_header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        df = pd.DataFrame(data, columns = header)
        df = df.astype(column_type)

        return df

    def _from_core_data_to_txt(self):
        pass



"""

"""    
    
class BtlsOutputVehicleAbstract(ABC):
    """
    Abstract base class for BtlsOutputVehicle files
    """
    _file_format = None
    descriptor = None
    _core_data = None
    _core_text = None
    
    def __init__(self, data = None, path = None, file_format = None, descriptor = None):
        self.descriptor = descriptor
        self._file_format = file_format
        if not path == None:
            if not data == None:
                raise ValueError("ERROR: Ambiguous input at initialisation. Only supply either the 'data' argument, or the 'path' argument to the csv data file")
            else:
                self.import_from_txt(path, file_format)
        else:
            self._core_data = data
    
    def __call__(self):
        return self._core_data
    
    @property
    def file_format(self):
        return self._file_format
    
    @property
    def data(self):
        return self._core_data
    
    @data.setter
    def data(self, data):
        raise AttributeError("ERROR: Setting data manually after object construction is disallowed for `BtlsOutput` class. Instead, construct a new object if required, or use `.import_from_txt()` method to import from an output txt file.")
    
    def to_numpy(self):
        return self._core_data.to_numpy()
    
    def as_numpy(self):
        self.to_numpy()
    
    @abstractmethod
    def _from_txt_to_core_data(self, txt):
        pass
    
    @abstractmethod
    def _from_core_data_to_txt(self):
        pass
    
    def import_from_txt(self, fpath, file_format):
        self._file_format = file_format
        with open(fpath) as f:
            content = f.read()
        self._core_text = content
        self._core_data = self._from_txt_to_core_data(content, file_format)
    
    def write_as_csv(self, fpath, fname):
        with open(fpath+fname) as f:
            f.write(self._from_core_data_to_txt())
            

class BtlsOutputVehicles(BtlsOutputVehicleAbstract):
    def _from_txt_to_core_data(self, txt, file_format):
        txt = txt.split('\n')[0:-1] #Split by line, ignore last empty entry
        if file_format == "BeDIT":
            header = [
                "head",
                "day",
                "month",
                "year",
                "hour",
                "minute",
                "second",
                "second/100",
                "speed",
                "gvw",
                "length",
                "num_axle",
                "direction",
                "lane",
                "transverse_location_in_lane",
                "AW1",
                "AS1",
                "AW2",
                "AS2",
                "AW3",
                "AS3",
                "AW4",
                "AS4",
                "AW5",
                "AS5",
                "AW6",
                "AS6",
                "AW7",
                "AS7",
                "AW8",
                "AS8",
                "AW9",
                "AS9",
                "AW10",
                "AS10",
                "AW11",
                "AS11",
                "AW12",
                "AS12",
                "AW13",
                "AS13",
                "AW14",
                "AS14",
                "AW15",
                "AS15",
                "AW16",
                "AS16",
                "AW17",
                "AS17",
                "AW18",
                "AS18",
                "AW19",
                "AS19",
                "AW20",
            ]
            column_width = np.array([
                4, #Head
                2, #Day
                2, #Month
                2, #Year
                2, #Hour
                2, #Minute
                2, #Second
                2, #Second/100
                3, #Speed (dm/s)
                4, #GVW (kg/100)
                3, #Length (dm)
                2, #Num axles
                1, #Direction (Zero based !)
                1, #Labe
                3, #Transverse location in lane
                3, #AW1,
                3, #AS1,
                3, #AW2,
                3, #AS2,
                3, #AW3,
                3, #AS3,
                3, #AW4,
                3, #AS4,
                3, #AW5,
                3, #AS5,
                3, #AW6,
                3, #AS6,
                3, #AW7,
                3, #AS7,
                3, #AW8,
                3, #AS8,
                3, #AW9,
                3, #AS9,
                3, #AW10,
                3, #AS10,
                3, #AW11,
                3, #AS11,
                3, #AW12,
                3, #AS12,
                3, #AW13,
                3, #AS13,
                3, #AW14,
                3, #AS14,
                3, #AW15,
                3, #AS15,
                3, #AW16,
                3, #AS16,
                3, #AW17,
                3, #AS17,
                3, #AW18,
                3, #AS18,
                3, #AW19,
                3, #AS19,
                3, #AW20,
            ])
            df = pd.DataFrame(parse_fixed_width_text(txt, column_width), columns = header)
            df = df.astype(
                {
                    "head": int,
                    "day": int,
                    "month": int,
                    "year": int,
                    "hour": int,
                    "minute": int,
                    "second": int,
                    "second/100": float,
                    "speed": float,
                    "gvw": float,
                    "length": float,
                    "num_axle": int,
                    "direction": int,
                    "lane": int,
                    "transverse_location_in_lane": float,
                    "AW1": float,
                    "AS1": float,
                    "AW2": float,
                    "AS2": float,
                    "AW3": float,
                    "AS3": float,
                    "AW4": float,
                    "AS4": float,
                    "AW5": float,
                    "AS5": float,
                    "AW6": float,
                    "AS6": float,
                    "AW7": float,
                    "AS7": float,
                    "AW8": float,
                    "AS8": float,
                    "AW9": float,
                    "AS9": float,
                    "AW10": float,
                    "AS10": float,
                    "AW11": float,
                    "AS11": float,
                    "AW12": float,
                    "AS12": float,
                    "AW13": float,
                    "AS13": float,
                    "AW14": float,
                    "AS14": float,
                    "AW15": float,
                    "AS15": float,
                    "AW16": float,
                    "AS16": float,
                    "AW17": float,
                    "AS17": float,
                    "AW18": float,
                    "AS18": float,
                    "AW19": float,
                    "AS19": float,
                    "AW20": float,
                }
            )
            
        if file_format == "MON":
            #Base column width, without the axle spacing, as MON file supports unknown number of axles
            base_column_width = np.array([
                9, #Head
                2, #Day
                2, #Month
                4, #Year
                2, #Hour
                2, #Minute
                5, #Second
                2, #Number of axles
                2, #Number of axle groups
                6, #GVW (kg)
                3, #Speed (km/h)
                5, #Length (mm)
                1, #Lane
                1, #Direction (Zero based !)
                4, #Transverse location in lane
            ], dtype = int)
            
            base_header = [
                "head",
                "day",
                "month",
                "year",
                "hour",
                "minute",
                "second",
                "num_axle",
                "num_axle_groups",
                "gvw",
                "speed",
                "length",
                "lane",
                "direction",
                "transverse_location_in_lane",
            ]
            
            column_type = {
                "head": int,
                "day": int,
                "month": int,
                "year": int,
                "hour": int,
                "minute": int,
                "second": float,
                "num_axle": int,
                "num_axle_groups": int,
                "gvw": float,
                "speed": float,
                "length": float,
                "lane": int,
                "direction": int,
                "transverse_location_in_lane": float
            }
            
            #Calculate the number of axles present in the data. Based on the first column
            excess_character = len(txt[0]) - np.sum(base_column_width)
            num_defined_axles = int(excess_character/10) # Each axles are 5 + 5 charachter long
            
            #Append the axle info
            column_width = np.append(base_column_width, np.ones(num_defined_axles*2, dtype = int) * 5)
            axle_header = np.array([["AW"+str(i+1), "AS"+str(i+1)] for i in range(0,num_defined_axles)]).flatten()
            header = np.append(base_header, axle_header)
            axle_column_type = dict(zip(axle_header, np.array([[float, float] for i in range(0,num_defined_axles)]).flatten()))
            column_type.update(axle_column_type)
            
            df = pd.DataFrame(parse_fixed_width_text(txt, column_width), columns = header)
            df = df.astype(column_type)
            #Divide the seconds by 1000 to convert from milliseconds
            df["second"] = df["second"]/1000
            
        return df
    
    def _from_core_data_to_txt(self):
        pass

