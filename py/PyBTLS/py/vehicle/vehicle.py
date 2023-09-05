import numpy as np
from PyBTLS.py.utils_interface._helper_class import _DfBased
from PyBTLS.py.utils_interface._vehicle_event_parser import (
    parse_from_vehicle_text_list_to_df_kwargs,
)
from PyBTLS.py.utils_interface._vehicle_event_writer import vehicle_df_to_txt


class Vehicle(_DfBased):
    """
    PyBTLS Vehicle class.
    Contains vehicle characteristic information,
    such as the number of axles, axle weights, axle spacings, etc.

    Arguments:
    ----------
    path: str
        Path to the file.
    file_format: str, optional
        Format of the vehicle text file.
        Either "CASTOR", "BeDIT", "DITIS", or "MON".
        If not supplied, PyBTLS will attempt to auto detect the format.
    """
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        if not "file_format" in kwargs:
            raise ValueError(
                "ERROR: 'path' argument supplied but not 'file_format'. Please supply the 'file_format' of the vehicle text"
            )
        # Check if its already separate by line, or only a pure string that can be separated
        if isinstance(txt, str):
            # Attempt to separate by string
            txt = txt.split("\n")
            # Check if last row is empty
            if (len(txt[-1]) == 0) or ((len(txt[-1]) == 1) and (txt[-1][0] == "")):
                txt = txt[0:-1]
        elif not isinstance(txt, list):
            raise TypeError(
                "ERROR: text supplied or read from file must be in either string, or list of strings"
            )

        return parse_from_vehicle_text_list_to_df_kwargs(txt, kwargs["file_format"])

    def _check_args(self, *args, **kwargs):
        super()._check_args(*args, **kwargs)
        if "path" in kwargs:
            if not (("path" in kwargs) and ("file_format" in kwargs)):
                raise ValueError(
                    "ERROR: Both 'path' and 'file_format' must be supplied together to import from file"
                )
        if "text" in kwargs:
            if not (("text" in kwargs) and ("file_format" in kwargs)):
                raise ValueError(
                    "ERROR: Both 'text' and 'file_format' must be supplied together to import from text"
                )
        if "file_format" in kwargs:
            if not (
                ("text" in kwargs)
                and ("file_format" in kwargs)
                or ("path" in kwargs)
                and ("file_format" in kwargs)
            ):
                raise ValueError(
                    "ERROR: 'file_format' supplied but not 'path' or 'text'. NOTE: Only supply 'text' OR 'path' along with 'file_format'"
                )
        return 1

    def write_as_txt(self, path, file_format=None):
        if file_format is None:
            file_format = self.file_format
        with open(path, "w") as f:
            f.write(vehicle_df_to_txt(self, file_format))

    @property
    def file_format(self):
        return self._detect_self_format()

    def _detect_self_format(self):
        # Auto-detect the format of self
        column = self.columns
        if "second/100" in column:
            if not "AS10" in column:
                return "CASTOR"
            elif not "TAW1" in column:
                return "BeDIT"
            else:
                return "DITIS"
        else:
            return "MON"

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        numrow = str(len(self.index))
        return f"{numrow} vehicle(s) {qualname} object at {hex(id(self))}"
