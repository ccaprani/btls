from PyBTLS.py_module.misc._helper_functions import data_enforce_type
from PyBTLS.py_module.misc._helper_class import _DfBased
from PyBTLS.py_module.misc._vehicle_event_parser import parse_from_event_text_list_to_event_data
from PyBTLS.py_module.vehicle.vehicle import Vehicle

class Event(_DfBased):
    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        if isinstance(txt, str):
            #Attempt to separate by string
            txt = txt.split('\n')
            #Check if last row is empty
            if (len(txt[-1]) == 0) or ((len(txt[-1]) == 1) and (txt[-1][0] == '')):
                txt = txt[0:-1]
        elif not isinstance(txt, list):
            raise TypeError("ERROR: text supplied or read from file must be in either string, or list of strings")
        
        #Parse the text
        data = parse_from_event_text_list_to_event_data(txt)
        #Parse the vehicle txt to Vehicle
        for veh in data:
            veh[-1] = Vehicle(text = veh[-1], file_format = "BeDIT")
        columns = self._get_columns()

        return {
            "data": data_enforce_type(data, columns)
        }

    def _get_columns(self):
        #Default column names
        return {
            "day": int,
            "load_effect_num": int, #The number of load effect, need to convert to string
            "load_effect": float, #The value of the load effect
            "time_of_event": float, #Seconds, the time that the event happens
            "dist_to_datum": float, #Distance of first axle to the datum
            "num_trucks": int, #Number of trucks in the event
            "vehicles": object, #The vehicles in the event
        }
        
class BlockMaximaEvent(Event):
    def _get_columns(self):
        return {
            "day": int,
            "load_effect_num": int, #The number of load effect, need to convert to string
            "max_load_effect": float, #The value of the maximum load effect
            "time_of_max": float, #Seconds, the time that the event happens
            "dist_to_datum": float, #Distance of first axle to the datum
            "num_trucks": int, #Number of trucks in the event
            "vehicles": object, #The vehicles in the event
        }