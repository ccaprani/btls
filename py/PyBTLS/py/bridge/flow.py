import numpy as np
from PyBTLS.py.utils_interface._helper_functions import (
    from_2darray_to_string,
    data_enforce_type,
    read_default_file,
)
from PyBTLS.py.utils_interface._helper_class import _DfBased, _ListLike

class LaneFlow(_DfBased):
    """
    Object containing the lane flow definition.
    
    """
    def __init__(self, lane_num=1, lane_dir=1, *args, **kwargs):
        """
        This LaneFlow object contains information such as hourly flow rate, percentage of trucks, percentage of 3 axle trucks, etc.
        This object is **not** attached to any bridge object. 
        In fact, a single bridge with multiple lanes can have the same lane flow definition, so that the bridge have the same traffic flow generation on all lanes.
        User can later edit the lane flow properties, e.g., chaning the peak hour flow rate using pandas DataFrame syntax.

        Parameters:
        -----------
        One of the following must be supplied:
            1. "path" (Preferred method)- Path to the lane flow file.
            2. pandas DataFrame constructor args and kwargs - Using pandas DataFrame with lane flow data. See pandas DataFrame documentation for more information.

        **Note**: intialization via txt is **not** supported for this class. Only via file or pandas DataFrame.
        """
        super().__init__(*args, **kwargs)
        self.lane_num = lane_num
        self.lane_dir = lane_dir

    def load_default(self, sample=1):
        """
        Method to load the default lane flow data.
        Useful for quickly getting a working lane flow, from which you can edit easily.
        Two samples are available. set using the `sample` argument.

        Parameters:
        -----------
        sample: int
            Load sample 1 or 2. Default is 1.
        """
        if sample == 1:
            fpath = "generic_lane_flow_data_1.csv"
        elif sample == 2:
            fpath = "generic_lane_flow_data_2.csv"
        else:
            raise ValueError(
                "ERROR: only 2 samples available. Specifiy 'sample' argument as 1 or 2"
            )
        data = read_default_file(fpath)
        columns = {
            "hour": int,
            "mean_truck_flow_rate": float,
            "mean_traffic_velolcity": float,
            "stdev_traffic_velocity": float,
            "pct_cars": float,
            "pct_2_axles": float,
            "pct_3_axles": float,
            "pct_4_axles": float,
            "pct_5_axles": float,
        }

        super()._init_via_df(data=data_enforce_type(data, columns))

    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        # Need to reimplement initialisation. Via txt is not supported as it is too complicated. Only support either
        # via DataFrame (base class has this covered)
        # or via file, which needs to be redefined to not use the _from_txt_to_dataframe_kwargs() method
        # And block the _from_txt_to_dataframe_kwargs() method from working
        raise NotImplementedError(
            "ERROR: Lane flow cannot be read from txt. Only from file or dataframes"
        )

    def _init_via_file(self, *args, **kwargs):
        """
        Priate method to import the file and create data frame automatically.
        Do not use this method directly. Use import_from_file() instead.
        """
        data = np.genfromtxt(kwargs["path"], delimiter=",")
        columns = {
            "hour": int,
            "mean_truck_flow_rate": float,
            "mean_traffic_velolcity": float,
            "stdev_traffic_velocity": float,
            "pct_cars": float,
            "pct_2_axles": float,
            "pct_3_axles": float,
            "pct_4_axles": float,
            "pct_5_axles": float,
        }
        return {"data": data_enforce_type(data, columns)}

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        return f"Lane {self.lane_num} with direction {self.lane_dir}; {qualname} object at {hex(id(self))}"

class BridgeFlow(_ListLike):
    """
    List like object containing LaneFlow objects for each lane on a bridge.
    """
    def __init__(self, lane_flow=None):
        """
        This object holds all the lane flow definition for each lane on a bridge.
        Users can easily access the lane flow a particular lane using list notation.
        Example:
        ```
        # Create the bridge flow objects
        # assuming lane_flow_light and lane_flow_heavy are LaneFlow objects
        bridge_flow = BridgeFlow([lane_flow_light, lane_flow_heavy])

        # Access the lane flow for lane 1
        print(bridge_flow[0])

        # Increase the hourly traffic by 10% for lane 2
        bridge_flow[1].mean_truck_flow_rate *= 1.1
        ```
        
        Parameters:
        -----------
        lane_flow: list of LaneFlow objects, optional
            List of LaneFlow objects. Can be added later using the add_lane_flow() method.

        """
        self._lane_flow = []
        if not lane_flow is None:
            self.add(lane_flow)

    @property
    def lane_flow(self):
        return self._lane_flow

    @lane_flow.setter
    def lane_flow(self, lane_flow):
        """
        Method to set all the lane flow objects at once.
        This will overwrite all existing lane flow.

        Parameters:
        -----------
        lane_flow: list of LaneFlow objects
            List of LaneFlow objects to be set.
        """
        # Reset lane_flow. Clear lane_flow, then reassign
        self._check_args(lane_flow=lane_flow)
        self._lane_flow = []
        self.add(lane_flow)

    def add_lane_flow(self, lane_flow):
        """
        Method to add lane flow objects to the existing lane flow.
        This will append the lane flow objects to the end.

        Parameters:
        -----------
        lane_flow: list of LaneFlow objects
            List of LaneFlow objects to be added.
        """
        if not isinstance(lane_flow, list):
            lane_flow = [lane_flow]
        self._check_args(lane_flow=lane_flow)
        self._lane_flow.extend(lane_flow)

    # Aliasses to add_lane_flow. Some for convenience, some to make it closer to list methods
    def add(self, lane_flow):
        """
        Alias for add_lane_flow() method.
        """
        self.add_lane_flow(lane_flow)

    def extend(self, lane_flow):
        """
        Alias for add_lane_flow() method.
        Provided to add similarity to Python List object.
        """
        self.add_lane_flow(lane_flow)

    def append(self, lane_flow):
        """
        Alias for add_lane_flow() method.
        Provided to add similarity to Python List object.
        """
        self.add_lane_flow(lane_flow)

    
    def insert(self, index, lane_flow):
        """
        Insert method similar to python list.

        Parameters:
        -----------
        index: int
            Index to insert the lane flow object
        lane_flow: LaneFlow object
            LaneFlow object to be inserted
        """
        if not isinstance(lane_flow, self.__setitem_acceptable_type__):
            raise TypeError(self.__setitem_err_msg__)
        self._lane_flow.insert(index, lane_flow)

    def get_existing_lane_num(self):
        """
        Method to check that all lane_num are defined
        """
        existing_lane = []
        for i, lf in enumerate(self.lane_flow):
            existing_lane.append(lf.lane_num)
        return existing_lane

    def write_as_csv(self, path="lane_flow.csv"):
        """
        Method to write to csv file, as required by BTLS to run.

        Parameters:
        -----------
        path: str, optional
            Path to the csv file. Default is "lane_flow.csv"
        """
        self._check_lane_clashes()
        self._check_lane_complete()
        with open(path, "w") as f:
            f.write(self._as_string())

    def to_numpy(self):
        """
        Method to convert this object to numpy array.
        """
        out = np.empty((len(self._lane_flow) * 25, 9))
        for i, lf in enumerate(self.lane_flow):
            offset = i * 25
            out[0 + offset, :] = [
                lf.lane_num,
                lf.lane_dir,
                np.nan,
                np.nan,
                np.nan,
                np.nan,
                np.nan,
                np.nan,
                np.nan,
            ]
            out[1 + offset : 25 + offset, :] = lf.to_numpy()
        return out

    def as_numpy(self):
        """
        Alias for to_numpy() method.
        """
        return self.to_numpy()

    def _check_args(self, *args, **kwargs):
        """
        Check that all objects to be added using the add_lane_flow() method are LaneFlow objects.
        """
        if "lane_flow" in kwargs:
            lane_flow = kwargs["lane_flow"]
            check = [
                0 if isinstance(lf, self.__setitem_acceptable_type__) else 1
                for lf in lane_flow
            ]
            if np.sum(check) > 0:
                raise TypeError(self.__setitem_err_msg__)

    def _check_lane_clashes(self):
        """
        Method that ensures no lane flow objects have the same lane number
        """
        existing_lane = []
        for i, lf in enumerate(self.lane_flow):
            if lf.lane_num in existing_lane:
                raise ValueError(
                    "ERROR: Duplicate lane_num detected. Check that no flow data has duplicated lane_num"
                )
            else:
                existing_lane.append(lf.lane_num)
        return True

    def _check_lane_complete(self):
        """
        Method to check that all lanes have lane flow data associated with it.
        """
        existing_lane = self.get_existing_lane_num()
        if not max(existing_lane) == len(existing_lane):
            raise ValueError(
                "ERROR: Some lane_num has no lane_flow data associated with it. Check again!"
            )
        return True

    def _as_string(self):
        """
        Method to convert this object to string, as required by BTLS to run.
        """
        # Will return empty string for all nan values
        return (
            from_2darray_to_string(self.to_numpy()).replace("nan", "") + "\n"
        )  # extra line required for BTLS to work (?!?!)

    @property
    def __primarykey__(self):
        return "_lane_flow"

    @property
    def __setitem_acceptable_type__(self):
        return LaneFlow

    @property
    def __str_quantifier__(self):
        return "lane(s)"

    def __str__(self):
        qualname = type(self).__qualname__
        return f"{len(self._lane_flow)} {self.__str_quantifier__} {qualname} object at {hex(id(self))}"

    def __repr__(self):
        qualname = type(self).__qualname__
        name = [lf.__str__() for lf in self.lane_flow]
        name = name.__str__().replace(",", ",\n").replace("'", "")

        return f"{len(self._lane_flow)} {self.__str_quantifier__} {qualname} object at {hex(id(self))}\n{name}"
