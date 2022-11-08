import numpy as np
import pandas as pd
from PyBTLS.py.utils_interface._helper_class import _DfBased
from PyBTLS.py.utils_interface._helper_functions import (
    data_enforce_type,
    read_default_file,
)


"""

"""


class NominalVehicle(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        txt = txt.split("\n")
        data = [t.split(",") for t in txt]
        if (len(data[-1] == 0)) or (len(data[-1]) == 1 and data[-1] == ""):
            data = data[0:-1]

        columns = {
            "axle_spacing": float,
            "axle_weight": float,
        }
        index = ["cov" if i == 0 else i for i in range(0, len(data))]
        return {"data": data_enforce_type(data, columns), "index": index}

    def load_default(self):
        data = np.array(
            read_default_file("nominal_vehicle.csv")[0:-1]
        )  # Ignore last row as its empty
        columns = {
            "axle_spacing": float,
            "axle_weight": float,
        }
        index = ["cov" if i == 0 else i for i in range(0, len(data))]
        self._init_via_df(data=data_enforce_type(data, columns), index=index)

    def write_as_csv(self, path="nominal_vehicle.csv"):
        np.savetxt(path, self.to_numpy(), delimiter=",")

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        return f"{str(len(self.index) - 1)} axles {qualname} object at {hex(id(self))}"
