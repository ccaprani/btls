import numpy as np
from PyBTLS.py_module.misc._helper_class import _DfBased
from PyBTLS.py_module.misc._helper_functions import data_enforce_type
from PyBTLS.py_module.default_files._default_files_loader import read_default_raw_file, read_default_file
from PyBTLS.py_module.vehicle.vehicle import Vehicle

class Garage(Vehicle):
    def load_default(self):
        txt = read_default_raw_file('garage.txt')
        super()._init_via_txt(text = txt, file_format = "MON")

class Kernels(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        txt = txt.split('\n')
        data = [t.split(",") for t in txt]
        if (len(data[-1] == 0)) or (len(data[-1]) == 1 and data[-1] == ''):
            data = data[0:-1]
        columns = {
            "bias": float,
            "std_dev": float,
        }
        index = ['gross_vehicle_mass', 'axle_group_mass', 'axle_group_spacing']
        return {
            "data": data_enforce_type(data, columns),
            "index": index
        }

    def load_default(self):
        data = np.array(read_default_file('kernels.csv')) #Ignore last row as its empty
        columns = {
            "bias": float,
            "std_dev": float,
        }
        index = ['gross_vehicle_mass', 'axle_group_mass', 'axle_group_spacing']
        self._init_via_df(data = data_enforce_type(data, columns), index = index)

    def write_as_csv(self, path, fname = "kernels.csv"):
        np.savetxt(path, self.to_numpy(), delimiter=",")

"""
class BtlsKernels():
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
        
    def write_as_csv(self, path, fname = "kernels.csv"):
        if not self.data == None:
            self._create_txt()
        with open(path + "/" + fname, 'w') as f:
            f.write(self.txt)
            
def _btls_default_garage():
    return BtlsGarage(path = "Default Files/garage.txt")

def _btls_default_kernels():
    return BtlsKernels(path = "Default Files/kernels.csv")

BtlsDefaultGarage = _btls_default_garage()
BtlsDefaultKernels = _btls_default_kernels()

"""
