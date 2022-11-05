import numpy as np
from PyBTLS.py.utils_interface._helper_class import _DfBased
from PyBTLS.py.utils_interface._helper_functions import data_enforce_type, read_default_file, read_default_file_raw
from PyBTLS.py.vehicle.vehicle import Vehicle

class Garage(Vehicle):
    def load_default(self):
        txt = read_default_file_raw('garage.txt')
        super()._init_via_txt(text = txt, file_format = "MON")

    def write_as_txt(self, path = "garage.txt", file_format=None):
        return super().write_as_txt(path, file_format)

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

    def write_as_csv(self, path = "kernels.csv"):
        np.savetxt(path, self.to_numpy(), delimiter=",")