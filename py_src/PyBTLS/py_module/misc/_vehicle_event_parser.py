import numpy as np
from PyBTLS.py_module.misc._helper_functions import data_enforce_type, parse_fixed_width_text

r"""
Helper function to parse vehicle data from raw txt to kwargs of pandas dataframe
Inputs:
    txt_list: A list of string objects containing the vehicle information. MUST BE A LIST.
    file_format: the file format of the vehicle text. Either 'CASTOR', 'BeDIT', 'DITIS', or 'MON'
Return:
    A dictionary object:
        {
            "data": np.ndarray(),
            "columns": 
            {
                "column header 1": column type,
                ...
            }
        }
    Returned output is a dictionary kwargs ready to be unpacked and passed to pd.DataFrame constructor

Usage Example:
    text = [vehicle_1_text, vehicle_2_text, ...]
    kwargs = parse_from_vehicle_text_list_to_df_kwargs(text, "MON")
    df = pd.DataFrame(**kwargs)
"""
def parse_from_vehicle_text_list_to_df_kwargs(txt_list, file_format):
    if file_format in ["CASTOR", "BeDIT"]:
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
            1 if file_format == "CASTOR" else 2, #Num axles
            1, #Direction (Zero based !)
            1, #Lane
            3, #Transverse location in lane (dm)
        ])
        column_type = {
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
            }

        #Calculate the number of axles present in the data. Based on the first column
        num_defined_axles = 9 if file_format == "CASTOR" else 20
        #Append the axle info. Sorry for the mess but hopefully its fast and still understandable somewhat
        column_width = np.append(column_width, np.array([[3, 2 if file_format == "CASTOR" else 3] for i in range(0, num_defined_axles)]).flatten()[0:-1])
        axle_header = np.array([["AW"+str(i+1), "AS"+str(i+1)] for i in range(0,num_defined_axles)]).flatten()[0:-1]
        header = np.append(header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([[float, float] for i in range(0,num_defined_axles)]).flatten()[0:-1]))
        column_type.update(axle_column_type)

        data = parse_fixed_width_text(txt_list, column_width)

    elif file_format == "MON":
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
        excess_character = len(txt_list[0]) - np.sum(base_column_width)
        num_defined_axles = int(excess_character/10) # Each axles are 5 + 5 character long
        
        #Append the axle info
        column_width = np.append(base_column_width, np.ones(num_defined_axles*2, dtype = int) * 5)
        axle_header = np.array([["AW"+str(i+1), "AS"+str(i+1)] for i in range(0,num_defined_axles)]).flatten()
        header = np.append(base_header, axle_header)
        axle_column_type = dict(zip(axle_header, np.array([[float, float] for i in range(0,num_defined_axles)]).flatten()))
        column_type.update(axle_column_type)

        #Convert to float (will be converted to the correct dtype later), so we can divide the seconds by 1000 (from ms -> s)
        #Unique to MON file_format
        data = parse_fixed_width_text(txt_list, column_width).astype(float)
        data[:,6] /= 1000

    elif file_format == "DITIS":
        raise NotImplementedError()

    else:
        raise ValueError("ERROR: file_format argument must be 'CASTOR', 'BeDIT', 'DITIS', or 'MON'")

    return {
        "data": data_enforce_type(data, column_type)
    }


r"""
Function to parse block maxima event file/text
WARNING: this function is slow as it depends on list.append(). If we can preallocate the list, this would be a whole lot faster.
Two ways to achieve this:
    1. Know the number of days in advance, so we can preallocate list of exactly that size, or
    2. See if there's any pattern between the day, event info, and vehicle info rows. If there's, we could use list comprehension rather than appending
Or open to any other suggestions
"""
def parse_from_event_text_list_to_event_data(text):
    #Determinations of current line
    # 0: Event info or day line
    # 1++: Vehicle info line
    line_status = 0
    num_vehicle_today = -1
    day_list = []
    current_day = -1
    event_info_list = []
    vehicle_list = []
    vehicle_today_txt = []
    for i, t in enumerate(text):
        match line_status:
            case 0:
                #Determine if this is a day row or event_info row
                if not ' ' in t:
                    #Day line
                    current_day = t
                else:
                    day_list.append(current_day)
                    event_info_list.append(t.split())
                    num_vehicle_today = int(t.split()[-1])
                    line_status = 1
            case other:
                if line_status < num_vehicle_today:
                    vehicle_today_txt.append(t)
                    line_status += 1
                else:
                    #End of today's vehicle
                    vehicle_today_txt.append(t)
                    vehicle_list.append(vehicle_today_txt)
                    vehicle_today_txt = []
                    num_vehicle_today = -1
                    line_status = 0

    #Combine day and event into into one list
    [[event_info_list[i].insert(0, day_list[i])] for i in range(0, len(event_info_list))]; #insert into the object directly. No output and assignment needed
    [[event_info_list[i].append(vehicle_list[i])] for i in range(0, len(event_info_list))]; #insert into the object directly. No output and assignment needed
    
    return event_info_list