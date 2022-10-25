import numpy as np
import pickle

r"""
Function to prepare data for DataFrame enforcing datatype per column
"""
def data_enforce_type(data, columns):
    return {c: list(map(columns[c], [d[i] for d in data])) for i, c in enumerate(columns)}

r"""

"""

def from_2darray_to_string(ndarray, delimiter = ",", eol_char = "\n"):
    return eol_char.join(item for item in [delimiter.join(item) for item in ndarray.astype(str)])


r"""
Parse text data that has a fixed column width, e.g. the BeDIT and CASTOR files
Argument:
    text: The text to be parsed, can be multiline
    column_width: The width of each column of the entries. Total width must be equal to the width of each text
Return
    numpy array of the parsed text
"""
def parse_fixed_width_text(text, column_width):
    column_end = np.cumsum(column_width)
    column_start = np.insert(column_end, 0, 0)[0:-1]
    return np.array([[t[column_start[i]:column_end[i]] for t in text] for i in range(0, len(column_width))]).T

"""
Generic save and load any object using pickle library

"""
def load(path):
    with open(path, 'rb') as f:
        out = pickle.load(f)
    return out

def save(path, obj):
    with open(path, 'wb') as f:
        pickle.dump(obj, f)