import numpy as np
import pickle
import pkgutil
from pathlib import Path

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
Function implementing a max distance to downsample a 1-d function.
Useful for reducing the number of IL points required to achieve some error level.
Input:
    x - input x values to be reduced
    y - input y values of function to be reduced
    e - maximum realtive distance (error) to achieve
NOTE: `e` is in relative term, i.e. if you want the max error to be within 0.1% of the actual value, you should set this to 0.001
Originally written in Matlab by C. C. Caprani, converted to python by A. Rizqiansyah.
"""
def maxdistance(x, y, e):
    n = len(x);  # assume x and y same length
    p = 0           # index for output vector
    k = 0           # index for input vector
    xs = np.zeros_like(y) # Allocate max size for xs and ys at the start. Will be reduced later.
    ys = np.zeros_like(y)
    ys[p] = y[k]    # save first permanent point
    xs[p] = x[k]
    k1 = 0         # index of the starting point
    for k in range(3, n):    # start from 3rd point and check second
        m = (y[k]-ys[p])/(x[k]-xs[p])  # slope of line to trial point
        emax = 0
        for j in range(k1+1, k): # no. of points in between
            yline = m*(x[j]-x[k1]) + y[k1]
            # Calculation of etrial changed from Matlab version to relative error
            if y[j] == 0:
                # y value is zero, should accept this point regardless
                etrial = e + 1
            else:
                etrial = abs((yline - y[j])/y[j])  
            if etrial > emax:
                emax = etrial
        if emax > e:  # not acceptable so store previous point
            p = p+1
            xs[p] = x[k-2]
            ys[p] = y[k-2]
            k1 = k-1  # next start point
    # Output only the calculated points, ignore the rest
    # +1 on the end index to slot in the end point, and extra +1 due to python indexing.
    p = p + 1
    xs = xs[0:p+1]
    ys = ys[0:p+1]
    xs[p] = x[n-1]
    ys[p] = y[n-1]
    return xs, ys

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


"""
Helper function to read and parse csv file. Allows reading of relative path within the default_files directory  context
Input:
    path: relative path to the file within the default_files context
Output:
    CSV file as list
"""

def read_default_traffic_file_raw(path):
    parent_path = Path(pkgutil.get_loader(__name__).path)
    fpath = parent_path.parents[4].joinpath("Traffic").joinpath(path)
    with fpath.open() as f:
        out = f.read()
    return out

def read_default_traffic_file(path):
    data = read_default_traffic_file_raw(path)
    data = data.split('\n')
    return [t.split(',') for t in data]

def read_default_file_raw(path):
    parent_path = Path(pkgutil.get_loader(__name__).path)
    fpath = parent_path.parents[1].joinpath("default_files").joinpath(path)
    with fpath.open() as f:
        out = f.read()
    return out

def read_default_file(path):
    data = read_default_file_raw(path)
    data = data.split('\n')
    return [t.split(',') for t in data]