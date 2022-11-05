
r"""

"""
import re
import numpy as np
from PyBTLS.py.utils_interface._helper_class import _DfBased
from PyBTLS.py.utils_interface._helper_functions import data_enforce_type

class _BtlsWrapper_DefaultOutputs():
    def __init__(self):
        self.interval_statistics = "No Output Set"
        self.vehicles = "No Output Set"
        self.block_maxima_summary = "No Output Set"

    def __str__(self):
        return super().__str__()

class CumulativeStatistics(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        if isinstance(txt, str):
            #Attempt to separate by string
            txt = txt.split('\n')
            #Check if last row is empty
            if (len(txt[-1]) == 0) or ((len(txt[-1]) == 1) and (txt[-1][0] == '')):
                txt = txt[0:-1]
        data = [text.split() for text in txt[1:]] #Split by column, ignore header
        #Equalise the length of the data by padding the list at the end
        pad = len(max(data, key=len)) 
        data = np.array([i + [0]*(pad-len(i)) for i in data])

        base_header = [
            'load_effect',
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
            'load_effect': int,
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
        num_defined_axles = data.shape[1] - 11

        #Append the axle info
        axle_header = ['event_' + str(i+1) + '_truck' for i in range(0,num_defined_axles)]
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        return {
            "data": data_enforce_type(data, column_type)
        }
    

class IntervalStatistics(_DfBased):
    descriptor = None
    def __init__(self, descriptor = None, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.descriptor = descriptor

    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        if isinstance(txt, str):
            #Attempt to separate by string
            txt = txt.split('\n')
            #Check if last row is empty
            if (len(txt[-1]) == 0) or ((len(txt[-1]) == 1) and (txt[-1][0] == '')):
                txt = txt[0:-1]
        data = [text.split() for text in txt[1:]] #Split by column, ignore header
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
        axle_column_type = dict(zip(axle_header, np.array([float for i in range(0,num_defined_axles)])))
        column_type.update(axle_column_type)

        return {
            "data": data_enforce_type(data, column_type)
        }

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        numrow = str(len(self.index))
        desc = "" if self.descriptor is None else f"{self.descriptor}: "
        return f"{desc} {numrow} row(s) of {qualname} object at {hex(id(self))}"

    def __repr__(self) -> str:
        return self.__str__()
    
    def _repr_html_(self):
        head = f"{self.__str__()}\n"
        df = super()._repr_html_()
        return f"{head}{df}"

        
class BlockMaximaSummary(_DfBased):
    descriptor = None
    def __init__(self, descriptor = None, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.descriptor = descriptor

    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        if isinstance(txt, str):
            #Attempt to separate by string
            txt = txt.split('\n')
            txt = [re.split("\t\t|\t", t)[0:-1] for t in txt] #Ignore empty column at the end. Ignore empty row at the end
            #Check if last row is empty
            if (len(txt[-1]) == 0) or ((len(txt[-1]) == 1) and (txt[-1][0] == '')):
                txt = txt[0:-1]
        data = txt
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

        return {
            "data": data_enforce_type(data, column_type)
        }

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        numrow = str(len(self.index))
        desc = "" if self.descriptor is None else f"{self.descriptor}: "
        return f"{desc} {numrow} row(s) of {qualname} object at {hex(id(self))}"

    def __repr__(self) -> str:
        return self.__str__()
    
    def _repr_html_(self):
        head = f"{self.__str__()}\n"
        df = super()._repr_html_()
        return f"{head}{df}"