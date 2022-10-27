import numpy as np
from PyBTLS.py_module.utils._helper_functions import from_2darray_to_string, data_enforce_type, read_default_file
from PyBTLS.py_module.utils._helper_class import _DfBased, _ListLike

"""

"""
class LaneFlow(_DfBased):
    def __init__(self, lane_num = 1, lane_dir = 1, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.lane_num = lane_num
        self.lane_dir = lane_dir

    def load_default(self, sample = 1):
        match sample:
            case 1:
                fpath = 'generic_lane_flow_data_1.csv'
            case 2:
                fpath = 'generic_lane_flow_data_2.csv'
            case other:
                raise ValueError("ERROR: only 2 samples available. Specifiy 'sample' argument as 1 or 2")
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

        super()._init_via_df(data = data_enforce_type(data, columns))

    # Need to reimplement initialisation. Via txt is not supported as it is too complicated. Only support either
    # via DataFrame (base class has this covered)
    # or via file, which needs to be redefined to not use the _from_txt_to_dataframe_kwargs() method
    # And block the _from_txt_to_dataframe_kwargs() method from working

    def _from_txt_to_dataframe_kwargs(self, txt, *args, **kwargs):
        raise NotImplementedError("ERROR: Lane flow cannot be read from txt. Only from file or dataframes")

    def _init_via_file(self, *args, **kwargs):
        data = np.genfromtxt(kwargs["path"], delimiter=',')
        header = {
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
        return {
            "data": data_enforce_type(data, columns)
        }

    def __str__(self) -> str:
        qualname = type(self).__qualname__
        return f"Lane {self.lane_num} with direction {self.lane_dir}; {qualname} object at {hex(id(self))}"
        
        
"""

"""
class BridgeFlow(_ListLike):
    def __init__(self, lane_flow = None):
        self._lane_flow = []
        if not lane_flow is None:
            self.add(lane_flow)

    @property
    def lane_flow(self):
        return self._lane_flow
    
    @lane_flow.setter
    def lane_flow(self, lane_flow):
        #Reset lane_flow. Clear lane_flow, then reassign
        self._check_args(lane_flow = lane_flow)
        self._lane_flow = []
        self.add(lane_flow)

    def add_lane_flow(self, lane_flow):
        if not isinstance(lane_flow, list):
            lane_flow = [lane_flow]
        self._check_args(lane_flow = lane_flow)
        self._lane_flow.extend(lane_flow)
    
    #Aliasses to add_lane_flow. Some for convenience, some to make it closer to list methods
    def add(self, lane_flow):
        self.add_lane_flow(lane_flow)

    def extend(self, lane_flow):
        self.add_lane_flow(lane_flow)

    def append(self, lane_flow):
        self.add_lane_flow(lane_flow)

    #Insert method similar to python list
    def insert(self, index, lane_flow):
        if not isinstance(lane_flow, self.__setitem_acceptable_type__):
            raise TypeError(self.__setitem_err_msg__)
        self._lane_flow.insert(index, lane_flow)

    def get_existing_lane_num(self):
        #Check that all lane_num are defined
        existing_lane = []
        for i, lf in enumerate(self.lane_flow):
            existing_lane.append(lf.lane_num)
        return existing_lane

    def write_as_csv(self, path):
        self._check_lane_clashes()
        self._check_lane_complete()
        with open(path, 'w') as f:
            f.write(self._as_string())

    def to_numpy(self):
        out = np.empty((len(self._lane_flow) * 25, 9))
        for i, lf in enumerate(self.lane_flow):
            offset = i*25
            out[0+offset, :] = [lf.lane_num, lf.lane_dir, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan]
            out[1+offset : 25+offset, :] = lf.to_numpy()
        return out
            
    def as_numpy(self):
        return self.to_numpy()

    def _check_args(self, *args, **kwargs):
        if "lane_flow" in kwargs:
            lane_flow = kwargs["lane_flow"]
            check = [0 if isinstance(lf, self.__setitem_acceptable_type__) else 1 for lf in lane_flow]
            if np.sum(check)>0:
                raise TypeError(self.__setitem_err_msg__)
    
    def _check_lane_clashes(self):
        existing_lane = []
        for i, lf in enumerate(self.lane_flow):
            if lf.lane_num in existing_lane:
                raise ValueError("ERROR: Duplicate lane_num detected. Check that no flow data has duplicated lane_num")
            else:
                existing_lane.append(lf.lane_num)
        return True
    
    def _check_lane_complete(self):
        existing_lane = self.get_existing_lane_num()
        if not max(existing_lane) == len(existing_lane):
            raise ValueError("ERROR: Some lane_num has no lane_flow data associated with it. Check again!")
        return True

    def _as_string(self):
        #Will return empty string for all nan values
        return from_2darray_to_string(self.to_numpy()).replace("nan", "") + "\n" #extra line required for BTLS to work (?!?!)

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
        name =  name.__str__().replace(',', ',\n').replace("'", "")

        return f"{len(self._lane_flow)} {self.__str_quantifier__} {qualname} object at {hex(id(self))}\n{name}"