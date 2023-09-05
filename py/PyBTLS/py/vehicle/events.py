from PyBTLS.py.utils_interface._helper_functions import data_enforce_type
from PyBTLS.py.utils_interface._helper_class import DFBased
from PyBTLS.py.utils_interface._vehicle_event_parser import (
    parse_from_event_text_list_to_event_data,
)
from PyBTLS.py.vehicle.vehicle import Vehicle


class Event(DFBased):
    """
    PyBTLS event class.
    An event is characterized by 
    the number of vehicles (`num_trucks`), 
    the axle configurations of each vehicles (`vehicles`),
    and the distance to the datum (`dist_to_datum`).

    Arguments:
    ----------
    path: str
        Path to the file.
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

        # Parse the text
        data = parse_from_event_text_list_to_event_data(txt)
        # Parse the vehicle txt to Vehicle
        for veh in data:
            veh[-1] = Vehicle(text=veh[-1], file_format=kwargs["file_format"])
        columns = self._get_columns()

        return {"data": data_enforce_type(data, columns)}

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

    def _get_columns(self):
        # Default column names
        return {
            "day": int,
            "load_effect_num": int,  # The number of load effect, need to convert to string
            "load_effect": float,  # The value of the load effect
            "time_of_event": float,  # Seconds, the time that the event happens
            "dist_to_datum": float,  # Distance of first axle to the datum
            "num_trucks": int,  # Number of trucks in the event
            "vehicles": Vehicle,  # The vehicles in the event
        }

    def __repr__(self) -> str:
        return self.__str__()


class BlockMaximaEvent(Event):
    """
    PyBTLS block maxima event class.
    Contains information of the event producing the maximum load effect for each day (or period).
    See the PyBTLS Event class for the definition of an event.

    Arguments:
    ----------
    path: str
        Path to the file.
    file_format: str, optional
        Format of the vehicle text file.
        Either "CASTOR", "BeDIT", "DITIS", or "MON".
        If not supplied, PyBTLS will attempt to auto detect the format.
    """
    def _get_columns(self):
        return {
            "day": int,
            "load_effect_num": int,  # The number of load effect, need to convert to string
            "max_load_effect": float,  # The value of the maximum load effect
            "time_of_max": float,  # Seconds, the time that the event happens
            "dist_to_datum": float,  # Distance of first axle to the datum
            "num_trucks": int,  # Number of trucks in the event
            "vehicles": Vehicle,  # The vehicles in the event
        }
