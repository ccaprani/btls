import numpy as np


""" 
Functions to convert from vehicle like data frame to text
Useful for exporting to txt.

"""

def vehicle_df_to_txt(df, file_format):
    
    if file_format == "MON":
        num_col = len(df.columns)
        char_length = np.array([
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
        ])
        num_defined_axles = int((num_col-15)/2)
        char_length = np.append(char_length, np.ones(num_defined_axles*2, dtype = int) * 5)
        factor = np.ones(len(char_length))
        factor[6] *= 1000

    elif file_format in ["BeDIT", "CASTOR"]:
        char_length = np.array([
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
        num_defined_axles = 9 if file_format == "CASTOR" else 20
        char_length = np.append(char_length, np.array([[3, 2 if file_format == "CASTOR" else 3] for i in range(0, num_defined_axles)]).flatten()[0:-1])
        factor = np.ones(len(char_length))
    
    elif file_format == "DITIS":
        raise NotImplementedError()
    else:
        raise ValueError("ERROR: file_format must be either 'CASTOR', 'BeDIT', 'DITIS', or 'MON'")

    col_wise_data = [np.round(df.iloc[:, i].to_numpy()*factor[i]).astype(int).astype(str) for i in range(0,len(char_length))]
    col_wise_data = [[col_wise_data[j][i].rjust(char_length[j]) for j in range(0, len(char_length))] for i in range(0, len(col_wise_data[0]))]
    return '\n'.join([''.join(t) for t in col_wise_data])
