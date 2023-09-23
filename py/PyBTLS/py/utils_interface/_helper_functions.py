import numpy as np
import pickle
import pkgutil
from pathlib import Path

def data_enforce_type(data, columns):
    r"""
    Function to prepare data for DataFrame enforcing datatype per column

    Arguments
    ---------
        data: list of list or numpy array
            The data to be prepared
        columns: 
            The data type of each columns in the data.

    Return
    ------
        A dictionary object, with the data type as key and the data as value.
        This can be used directly to construct a pandas DataFrame object.
    
    Usage Example
    -------------
    ```
    data = ...
    columns = ...

    dataframe = pandas.DataFrame(
        {
            "column 1": data_enforce_type(data[:, 1], columns[1]),
            "column 2": data_enforce_type(data[:, 2], columns[2]),
            ...
        }
    ```
    """
    return {
        c: list(map(columns[c], [d[i] for d in data])) for i, c in enumerate(columns)
    }

def from_2darray_to_string(ndarray, delimiter=",", eol_char="\n"):
    r"""
    Convert a 2d numpy array to a string with delimiter and end of line character

    Arguments
    ---------
        ndarray: numpy array
            The 2d numpy array to be converted
        delimiter: str
            The delimiter to be used to separate each column
        eol_char: str
            The end of line character to be used to separate each row

    """
    return eol_char.join(
        item for item in [delimiter.join(item) for item in ndarray.astype(str)]
    )


def parse_fixed_width_text(text, column_width):
    r"""
    Parse text data that has a fixed column width, e.g. the BeDIT and CASTOR files

    Arguments
    ---------
        text: str
            The text to be parsed, can be multiline
        column_width: int
            The width of each column of the entries. Total width must be equal to the width of each text

    Return
    ------
        numpy array of the parsed text
    """
    column_end = np.cumsum(column_width)
    column_start = np.insert(column_end, 0, 0)[0:-1]
    return np.array(
        [
            [t[column_start[i] : column_end[i]] for t in text]
            for i in range(0, len(column_width))
        ]
    ).T





def maxdistance(x, y, e):
    """
    Function implementing a max distance to downsample a 1-d function.
    Useful for reducing the number of IL points required to achieve some error level.

    Arguments
    ---------
        x: numpy array of float
            input x values to be compressed
        y: numpy array of float
            input y values of function to be compressed
        e: float
            maximum realtive distance (error) to achieve

    Return
    ------
        xs: numpy array of float
            compressed x values
        ys: numpy array of float
            compressed y values

    NOTE: `e` is in relative term, i.e. if you want the max error to be within 0.1% of the actual value, you should set this to 0.001
    Originally written in Matlab by C. C. Caprani, converted to python by A. Rizqiansyah.
    """
    n = len(x)
    # assume x and y same length
    p = 0  # index for output vector
    k = 0  # index for input vector
    xs = np.zeros_like(
        y
    )  # Allocate max size for xs and ys at the start. Will be reduced later.
    ys = np.zeros_like(y)
    ys[p] = y[k]  # save first permanent point
    xs[p] = x[k]
    k1 = 0  # index of the starting point
    for k in range(3, n):  # start from 3rd point and check second
        m = (y[k] - ys[p]) / (x[k] - xs[p])  # slope of line to trial point
        emax = 0
        for j in range(k1 + 1, k):  # no. of points in between
            yline = m * (x[j] - x[k1]) + y[k1]
            # Calculation of etrial changed from Matlab version to relative error
            if y[j] == 0:
                # y value is zero, should accept this point regardless
                etrial = e + 1
            else:
                etrial = abs((yline - y[j]) / y[j])
            if etrial > emax:
                emax = etrial
        if emax > e:  # not acceptable so store previous point
            p = p + 1
            xs[p] = x[k - 2]
            ys[p] = y[k - 2]
            k1 = k - 1  # next start point
    # Output only the calculated points, ignore the rest
    # +1 on the end index to slot in the end point, and extra +1 due to python indexing.
    p = p + 1
    xs = xs[0 : p + 1]
    ys = ys[0 : p + 1]
    xs[p] = x[n - 1]
    ys[p] = y[n - 1]
    return xs, ys





def load(path):
    """
    Generic load any object using pickle library.
    NOTE: not all objects can be saved. See the pickl library to check if an object can be saved.

    Arguments
    ---------
        path: str
            path to the file to be loaded

    Return
    ------
        The loaded object
    """
    with open(path, "rb") as f:
        out = pickle.load(f)
    return out


def save(path, obj):
    """
    Generic load any object using pickle library.
    NOTE: not all objects can be saved. See the pickl library to check if an object can be saved.

    Arguments
    ---------
        path: str
            path to the file to be saved
        obj: object
            the object to be saved

    Return
    ------
        None
    """
    with open(path, "wb") as f:
        pickle.dump(obj, f)





def read_default_traffic_file_raw(path):
    """
    Helper function to read traffic file. 
    Allows reading of relative path within the package directory.
    The distinction is made compared to other file (using `read_default_file_raw`) because the traffic file is located outside of the `py` folder.

    Argument
    --------
        path: str
            relative path to the file within the package context

    Return
    ------
        string; the traffic data as raw string.
    """
    parent_path = Path(pkgutil.get_loader(__name__).path)
    fpath = parent_path.parents[4].joinpath("Traffic").joinpath(path)
    with fpath.open() as f:
        out = f.read()
    return out


def read_default_traffic_file(path):
    """
    Helper function to read and parse traffic file. 
    Allows reading of relative path within the package directory.
    Assumes that the data is comma separated, and each row is separated by newline.
    The distinction is made compared to other file (using `read_default_file`) because the traffic file is located outside of the `py` folder.

    Argument
    --------
        path: str
            relative path to the file within the package context

    Return
    ------
        list of list of string; the traffic data.
    """
    data = read_default_traffic_file_raw(path)
    data = data.split("\n")
    return [t.split(",") for t in data]


def read_default_file_raw(path):
    """
    Helper function to read any file. 
    Allows reading of relative path within the package directory.

    Argument
    --------
        path: str
            relative path to the file within the package context

    Return
    ------
        string; the data as raw string.
    """
    parent_path = Path(pkgutil.get_loader(__name__).path)
    fpath = parent_path.parents[4].joinpath("tests").joinpath(path)
    with fpath.open() as f:
        out = f.read()
    return out


def read_default_file(path):
    """
    Helper function to read and parse any file. 
    Allows reading of relative path within the package directory.
    Assumes that the data is comma separated, and each row is separated by newline.

    Argument
    --------
        path: str
            relative path to the file within the package context

    Return
    ------
        list of list of string; the data.
    """
    data = read_default_file_raw(path)
    data = data.split("\n")
    return [t.split(",") for t in data]
